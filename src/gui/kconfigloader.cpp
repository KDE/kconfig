/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfigloader.h"
#include "kconfigloader_p.h"
#include "kconfigloaderhandler_p.h"

#include <QColor>
#include <QFont>
#include <QHash>
#include <QUrl>

#include <QDebug>

void ConfigLoaderPrivate::parse(KConfigLoader *loader, QIODevice *xml)
{
    clearData();
    loader->clearItems();

    if (xml) {
        ConfigLoaderHandler handler(loader, this);
        handler.parse(xml);
    }
}

ConfigLoaderHandler::ConfigLoaderHandler(KConfigLoader *config, ConfigLoaderPrivate *d)
    : m_config(config),
      d(d)
{
    resetState();
}

bool ConfigLoaderHandler::parse(QIODevice *input)
{
    if (!input->open(QIODevice::ReadOnly)) {
        qWarning() << "Impossible to open device";
        return false;
    }
    QXmlStreamReader reader(input);

    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.hasError()) {
            return false;
        }

        switch (reader.tokenType()) {
        case QXmlStreamReader::StartElement:
            if (!startElement(reader.name(), reader.attributes())) {
                return false;
            }
            break;
        case QXmlStreamReader::EndElement:
            if (!endElement(reader.name())) {
                return false;
            }
            break;
        case QXmlStreamReader::Characters:
            if (!reader.isWhitespace() && !reader.text().trimmed().isEmpty()) {
                if (!characters(reader.text())) {
                    return false;
                }
            }
            break;
        default:
            break;
        }
    }

    if (reader.isEndDocument()) {
        return false;
    }

    return true;
}

bool ConfigLoaderHandler::startElement(const QStringRef &localName, const QXmlStreamAttributes &attrs)
{
    // qDebug() << "ConfigLoaderHandler::startElement(" << localName << qName;
    const QString tag = localName.toString().toLower();
    if (tag == QLatin1String("group")) {
        QString group;
        for (const auto &attr : attrs) {
            const QStringRef name = attr.name();
            if (name.compare(QLatin1String("name"), Qt::CaseInsensitive) == 0) {
                //qDebug() << "set group to" << attrs.value(i);
                group = attr.value().toString();
            }
        }
        if (group.isEmpty()) {
            group = d->baseGroup;
        } else {
            d->groups.append(group);
            if (!d->baseGroup.isEmpty()) {
                group = d->baseGroup + QLatin1Char('\x1d') + group;
            }
        }

        if (m_config) {
            m_config->setCurrentGroup(group);
        }
    } else if (tag == QLatin1String("entry")) {
        for (const auto &attr : attrs) {
            const QStringRef name = attr.name();
            if (name.compare(QLatin1String("name"), Qt::CaseInsensitive) == 0) {
                m_name = attr.value().trimmed().toString();
            } else if (name.compare(QLatin1String("type"), Qt::CaseInsensitive) == 0) {
                m_type = attr.value().toString().toLower();
            } else if (name.compare(QLatin1String("key"), Qt::CaseInsensitive) == 0) {
                m_key = attr.value().trimmed().toString();
            }
        }
    } else if (tag == QLatin1String("choice")) {
        m_choice.name.clear();
        m_choice.label.clear();
        m_choice.whatsThis.clear();
        for (const auto &attr : attrs) {
            const QStringRef name = attr.name();
            if (name.compare(QLatin1String("name"), Qt::CaseInsensitive) == 0) {
                m_choice.name = attr.value().toString();
            }
        }
        m_inChoice = true;
    }

    return true;
}

bool ConfigLoaderHandler::characters(const QStringRef &ch)
{
    m_cdata.append(ch.toString());
    return true;
}

bool ConfigLoaderHandler::endElement(const QStringRef &localName)
{
//     qDebug() << "ConfigLoaderHandler::endElement(" << localName << qName;
    const QStringRef tag = localName;
    if (tag.compare(QLatin1String("entry"), Qt::CaseInsensitive) == 0) {
        addItem();
        resetState();
    } else if (tag.compare(QLatin1String("label"), Qt::CaseInsensitive) == 0) {
        if (m_inChoice) {
            m_choice.label = m_cdata.trimmed();
        } else {
            m_label = m_cdata.trimmed();
        }
    } else if (tag.compare(QLatin1String("whatsthis"), Qt::CaseInsensitive) == 0) {
        if (m_inChoice) {
            m_choice.whatsThis = m_cdata.trimmed();
        } else {
            m_whatsThis = m_cdata.trimmed();
        }
    } else if (tag.compare(QLatin1String("default"), Qt::CaseInsensitive) == 0) {
        m_default = m_cdata.trimmed();
    } else if (tag.compare(QLatin1String("min"), Qt::CaseInsensitive) == 0) {
        m_min = m_cdata.toInt(&m_haveMin);
    } else if (tag.compare(QLatin1String("max"), Qt::CaseInsensitive) == 0) {
        m_max = m_cdata.toInt(&m_haveMax);
    } else if (tag.compare(QLatin1String("choice"), Qt::CaseInsensitive) == 0) {
        m_enumChoices.append(m_choice);
        m_inChoice = false;
    }

    m_cdata.clear();
    return true;
}

void ConfigLoaderHandler::addItem()
{
    if (m_name.isEmpty()) {
        if (m_key.isEmpty()) {
            return;
        }

        m_name = m_key;
    }

    m_name.remove(QLatin1Char(' '));

    KConfigSkeletonItem *item = nullptr;

    if (m_type == QLatin1String("bool")) {
        bool defaultValue = m_default.toLower() == QLatin1String("true");
        item = m_config->addItemBool(m_name, *d->newBool(), defaultValue, m_key);
    } else if (m_type == QLatin1String("color")) {
        item = m_config->addItemColor(m_name, *d->newColor(), QColor(m_default), m_key);
    } else if (m_type == QLatin1String("datetime")) {
        item = m_config->addItemDateTime(m_name, *d->newDateTime(),
                                         QDateTime::fromString(m_default), m_key);
    } else if (m_type == QLatin1String("enum")) {
        m_key = (m_key.isEmpty()) ? m_name : m_key;
        KConfigSkeleton::ItemEnum *enumItem =
            new KConfigSkeleton::ItemEnum(m_config->currentGroup(),
                                          m_key, *d->newInt(),
                                          m_enumChoices,
                                          m_default.toUInt());
        m_config->addItem(enumItem, m_name);
        item = enumItem;
    } else if (m_type == QLatin1String("font")) {
        item = m_config->addItemFont(m_name, *d->newFont(), QFont(m_default), m_key);
    } else if (m_type == QLatin1String("int")) {
        KConfigSkeleton::ItemInt *intItem = m_config->addItemInt(m_name, *d->newInt(),
                                            m_default.toInt(), m_key);

        if (m_haveMin) {
            intItem->setMinValue(m_min);
        }

        if (m_haveMax) {
            intItem->setMaxValue(m_max);
        }

        item = intItem;
    } else if (m_type == QLatin1String("password")) {
        item = m_config->addItemPassword(m_name, *d->newString(), m_default, m_key);
    } else if (m_type == QLatin1String("path")) {
        item = m_config->addItemPath(m_name, *d->newString(), m_default, m_key);
    } else if (m_type == QLatin1String("string")) {
        item = m_config->addItemString(m_name, *d->newString(), m_default, m_key);
    } else if (m_type == QLatin1String("stringlist")) {
        //FIXME: the split() is naive and will break on lists with ,'s in them
        //empty parts are not wanted in this case
        item = m_config->addItemStringList(m_name, *d->newStringList(),
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
                                           m_default.split(QLatin1Char(','), QString::SkipEmptyParts), m_key);
#else
                                           m_default.split(QLatin1Char(','), Qt::SkipEmptyParts), m_key);
#endif
    } else if (m_type == QLatin1String("uint")) {
        KConfigSkeleton::ItemUInt *uintItem =
            m_config->addItemUInt(m_name, *d->newUint(), m_default.toUInt(), m_key);
        if (m_haveMin) {
            uintItem->setMinValue(m_min);
        }
        if (m_haveMax) {
            uintItem->setMaxValue(m_max);
        }
        item = uintItem;
    } else if (m_type == QLatin1String("url")) {
        m_key = (m_key.isEmpty()) ? m_name : m_key;
        KConfigSkeleton::ItemUrl *urlItem =
            new KConfigSkeleton::ItemUrl(m_config->currentGroup(),
                                         m_key, *d->newUrl(),
                                         QUrl::fromUserInput(m_default));
        m_config->addItem(urlItem, m_name);
        item = urlItem;
    } else if (m_type == QLatin1String("double")) {
        KConfigSkeleton::ItemDouble *doubleItem = m_config->addItemDouble(m_name,
                *d->newDouble(), m_default.toDouble(), m_key);
        if (m_haveMin) {
            doubleItem->setMinValue(m_min);
        }
        if (m_haveMax) {
            doubleItem->setMaxValue(m_max);
        }
        item = doubleItem;
    } else if (m_type == QLatin1String("intlist")) {
        const QStringList tmpList = m_default.split(QLatin1Char(','));
        QList<int> defaultList;
        for (const QString &tmp : tmpList) {
            defaultList.append(tmp.toInt());
        }
        item = m_config->addItemIntList(m_name, *d->newIntList(), defaultList, m_key);
    } else if (m_type == QLatin1String("longlong")) {
        KConfigSkeleton::ItemLongLong *longlongItem = m_config->addItemLongLong(m_name,
                *d->newLongLong(), m_default.toLongLong(), m_key);
        if (m_haveMin) {
            longlongItem->setMinValue(m_min);
        }
        if (m_haveMax) {
            longlongItem->setMaxValue(m_max);
        }
        item = longlongItem;
        /* No addItemPathList in KConfigSkeleton ?
        } else if (m_type == "PathList") {
            //FIXME: the split() is naive and will break on lists with ,'s in them
            item = m_config->addItemPathList(m_name, *d->newStringList(), m_default.split(","), m_key);
        */
    } else if (m_type == QLatin1String("point")) {
        QPoint defaultPoint;
        QStringList tmpList = m_default.split(QLatin1Char(','));
        if (tmpList.size() >= 2) {
            defaultPoint.setX(tmpList[0].toInt());
            defaultPoint.setY(tmpList[1].toInt());
        }
        item = m_config->addItemPoint(m_name, *d->newPoint(), defaultPoint, m_key);
    } else if (m_type == QLatin1String("rect")) {
        QRect defaultRect;
        QStringList tmpList = m_default.split(QLatin1Char(','));
        if (tmpList.size() >= 4) {
            defaultRect.setCoords(tmpList[0].toInt(), tmpList[1].toInt(),
                                  tmpList[2].toInt(), tmpList[3].toInt());
        }
        item = m_config->addItemRect(m_name, *d->newRect(), defaultRect, m_key);
    } else if (m_type == QLatin1String("size")) {
        QSize defaultSize;
        QStringList tmpList = m_default.split(QLatin1Char(','));
        if (tmpList.size() >= 2) {
            defaultSize.setWidth(tmpList[0].toInt());
            defaultSize.setHeight(tmpList[1].toInt());
        }
        item = m_config->addItemSize(m_name, *d->newSize(), defaultSize, m_key);
    } else if (m_type == QLatin1String("ulonglong")) {
        KConfigSkeleton::ItemULongLong *ulonglongItem =
            m_config->addItemULongLong(m_name, *d->newULongLong(), m_default.toULongLong(), m_key);
        if (m_haveMin) {
            ulonglongItem->setMinValue(m_min);
        }
        if (m_haveMax) {
            ulonglongItem->setMaxValue(m_max);
        }
        item = ulonglongItem;
        /* No addItemUrlList in KConfigSkeleton ?
        } else if (m_type == "urllist") {
            //FIXME: the split() is naive and will break on lists with ,'s in them
            QStringList tmpList = m_default.split(",");
            QList<QUrl> defaultList;
            foreach (const QString& tmp, tmpList) {
                defaultList.append(QUrl(tmp));
            }
            item = m_config->addItemUrlList(m_name, *d->newUrlList(), defaultList, m_key);*/
    }

    if (item) {
        item->setLabel(m_label);
        item->setWhatsThis(m_whatsThis);
        d->keysToNames.insert(item->group() + item->key(), item->name());
    }
}

void ConfigLoaderHandler::resetState()
{
    m_haveMin = false;
    m_min = 0;
    m_haveMax = false;
    m_max = 0;
    m_name.clear();
    m_type.clear();
    m_label.clear();
    m_default.clear();
    m_key.clear();
    m_whatsThis.clear();
    m_enumChoices.clear();
    m_inChoice = false;
}

KConfigLoader::KConfigLoader(const QString &configFile, QIODevice *xml, QObject *parent)
    : KConfigSkeleton(configFile, parent),
      d(new ConfigLoaderPrivate)
{
    d->parse(this, xml);
}

KConfigLoader::KConfigLoader(KSharedConfigPtr config, QIODevice *xml, QObject *parent)
    : KConfigSkeleton(std::move(config), parent),
      d(new ConfigLoaderPrivate)
{
    d->parse(this, xml);
}

//FIXME: obviously this is broken and should be using the group as the root,
//       but KConfigSkeleton does not currently support this. it will eventually though,
//       at which point this can be addressed properly
KConfigLoader::KConfigLoader(const KConfigGroup &config, QIODevice *xml, QObject *parent)
    : KConfigSkeleton(KSharedConfig::openConfig(config.config()->name(), config.config()->openFlags(), config.config()->locationType()), parent),
      d(new ConfigLoaderPrivate)
{
    KConfigGroup group = config.parent();
    d->baseGroup = config.name();
    while (group.isValid() && group.name() != QLatin1String("<default>")) {
        d->baseGroup = group.name() + QLatin1Char('\x1d') + d->baseGroup;
        group = group.parent();
    }
    d->parse(this, xml);
}

KConfigLoader::~KConfigLoader()
{
    delete d;
}

KConfigSkeletonItem *KConfigLoader::findItem(const QString &group, const QString &key) const
{
    return KConfigSkeleton::findItem(d->keysToNames[group + key]);
}

KConfigSkeletonItem *KConfigLoader::findItemByName(const QString &name) const
{
    return KConfigSkeleton::findItem(name);
}

QVariant KConfigLoader::property(const QString &name) const
{
    KConfigSkeletonItem *item = KConfigSkeleton::findItem(name);

    if (item) {
        return item->property();
    }

    return QVariant();
}

bool KConfigLoader::hasGroup(const QString &group) const
{
    return d->groups.contains(group);
}

QStringList KConfigLoader::groupList() const
{
    return d->groups;
}

#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 0)
bool KConfigLoader::usrWriteConfig()
#else
bool KConfigLoader::usrSave()
#endif
{
    if (d->saveDefaults) {
        const auto listItems = items();
        for (const auto& item : listItems) {
            config()->group(item->group()).writeEntry(item->key(), "");
        }
    }
    return true;
}
