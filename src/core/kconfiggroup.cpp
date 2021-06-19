/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfiggroup.h"
#include "kconfiggroup_p.h"

#include "kconfig.h"
#include "kconfig_core_log_settings.h"
#include "kconfig_p.h"
#include "kconfigdata_p.h"
#include "ksharedconfig.h"

#include <QDate>
#include <QDir>
#include <QFile>
#include <QPoint>
#include <QRect>
#include <QSharedData>
#include <QString>
#include <QTextStream>
#include <QUrl>

#include <math.h>
#include <stdlib.h>

class KConfigGroupPrivate : public QSharedData
{
public:
    KConfigGroupPrivate(KConfig *owner, bool isImmutable, bool isConst, const QByteArray &name)
        : mOwner(owner)
        , mName(name)
        , bImmutable(isImmutable)
        , bConst(isConst)
    {
    }

    KConfigGroupPrivate(const KSharedConfigPtr &owner, const QByteArray &name)
        : sOwner(owner)
        , mOwner(sOwner.data())
        , mName(name)
        , bImmutable(name.isEmpty() ? owner->isImmutable() : owner->isGroupImmutable(name))
        , bConst(false)
    {
    }

    KConfigGroupPrivate(KConfigGroup *parent, bool isImmutable, bool isConst, const QByteArray &name)
        : sOwner(parent->d->sOwner)
        , mOwner(parent->d->mOwner)
        , mName(name)
        , bImmutable(isImmutable)
        , bConst(isConst)
    {
        if (!parent->d->mName.isEmpty()) {
            mParent = parent->d;
        }
    }

    KConfigGroupPrivate(const KConfigGroupPrivate *other, bool isImmutable, const QByteArray &name)
        : sOwner(other->sOwner)
        , mOwner(other->mOwner)
        , mName(name)
        , bImmutable(isImmutable)
        , bConst(other->bConst)
    {
        if (!other->mName.isEmpty()) {
            mParent = const_cast<KConfigGroupPrivate *>(other);
        }
    }

    KSharedConfig::Ptr sOwner;
    KConfig *mOwner;
    QExplicitlySharedDataPointer<KConfigGroupPrivate> mParent;
    QByteArray mName;

    /* bitfield */
    const bool bImmutable : 1; // is this group immutable?
    const bool bConst : 1; // is this group read-only?

    QByteArray fullName() const
    {
        if (!mParent) {
            return name();
        }
        return mParent->fullName(mName);
    }

    QByteArray name() const
    {
        if (mName.isEmpty()) {
            return QByteArrayLiteral("<default>");
        }
        return mName;
    }

    QByteArray fullName(const QByteArray &aGroup) const
    {
        if (mName.isEmpty()) {
            return aGroup;
        }
        return fullName() + '\x1d' + aGroup;
    }

    static QExplicitlySharedDataPointer<KConfigGroupPrivate> create(KConfigBase *master, const QByteArray &name, bool isImmutable, bool isConst)
    {
        QExplicitlySharedDataPointer<KConfigGroupPrivate> data;
        if (dynamic_cast<KConfigGroup *>(master)) {
            data = new KConfigGroupPrivate(static_cast<KConfigGroup *>(master), isImmutable, isConst, name);
        } else {
            data = new KConfigGroupPrivate(dynamic_cast<KConfig *>(master), isImmutable, isConst, name);
        }
        return data;
    }

    static QByteArray serializeList(const QList<QByteArray> &list);
    static QStringList deserializeList(const QString &data);
};

QByteArray KConfigGroupPrivate::serializeList(const QList<QByteArray> &list)
{
    QByteArray value;

    if (!list.isEmpty()) {
        auto it = list.cbegin();
        const auto end = list.cend();

        value = QByteArray(*it).replace('\\', QByteArrayLiteral("\\\\")).replace(',', QByteArrayLiteral("\\,"));

        while (++it != end) {
            // In the loop, so it is not done when there is only one element.
            // Doing it repeatedly is a pretty cheap operation.
            value.reserve(4096);

            value += ',';
            value += QByteArray(*it).replace('\\', QByteArrayLiteral("\\\\")).replace(',', QByteArrayLiteral("\\,"));
        }

        // To be able to distinguish an empty list from a list with one empty element.
        if (value.isEmpty()) {
            value = QByteArrayLiteral("\\0");
        }
    }

    return value;
}

QStringList KConfigGroupPrivate::deserializeList(const QString &data)
{
    if (data.isEmpty()) {
        return QStringList();
    }
    if (data == QLatin1String("\\0")) {
        return QStringList(QString());
    }
    QStringList value;
    QString val;
    val.reserve(data.size());
    bool quoted = false;
    for (int p = 0; p < data.length(); p++) {
        if (quoted) {
            val += data[p];
            quoted = false;
        } else if (data[p].unicode() == '\\') {
            quoted = true;
        } else if (data[p].unicode() == ',') {
            val.squeeze(); // release any unused memory
            value.append(val);
            val.clear();
            val.reserve(data.size() - p);
        } else {
            val += data[p];
        }
    }
    value.append(val);
    return value;
}

static QVector<int> asIntList(const QByteArray &string)
{
    const auto &splitString = string.split(',');

    QVector<int> list;
    list.reserve(splitString.count());
    for (const QByteArray &s : splitString) {
        list << s.toInt();
    }
    return list;
}

static QVector<qreal> asRealList(const QByteArray &string)
{
    const auto &splitString = string.split(',');

    QVector<qreal> list;
    list.reserve(splitString.count());
    for (const QByteArray &s : splitString) {
        list << s.toDouble();
    }
    return list;
}

static QString errString(const char *pKey, const QByteArray &value, const QVariant &aDefault)
{
    return QStringLiteral("\"%1\" - conversion of \"%3\" to %2 failed")
        .arg(QString::fromLatin1(pKey), QString::fromLatin1(QVariant::typeToName(aDefault.type())), QString::fromLatin1(value));
}

static QString formatError(int expected, int got)
{
    return QStringLiteral(" (wrong format: expected %1 items, got %2)").arg(expected).arg(got);
}

QVariant KConfigGroup::convertToQVariant(const char *pKey, const QByteArray &value, const QVariant &aDefault)
{
    // if a type handler is added here you must add a QVConversions definition
    // to conversioncheck.h, or ConversionCheck::to_QVariant will not allow
    // readEntry<T> to convert to QVariant.
    switch (static_cast<QMetaType::Type>(aDefault.type())) {
    case QMetaType::UnknownType:
        return QVariant();
    case QMetaType::QString:
        // this should return the raw string not the dollar expanded string.
        // imho if processed string is wanted should call
        // readEntry(key, QString) not readEntry(key, QVariant)
        return QString::fromUtf8(value);
    case QMetaType::QVariantList:
    case QMetaType::QStringList:
        return KConfigGroupPrivate::deserializeList(QString::fromUtf8(value));
    case QMetaType::QByteArray:
        return value;
    case QMetaType::Bool: {
        const QByteArray lower(value.toLower());
        if (lower == "false" || lower == "no" || lower == "off" || lower == "0") {
            return false;
        }
        return true;
    }
    case QMetaType::Double:
    case QMetaType::Float:
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong: {
        QVariant tmp = value;
        if (!tmp.convert(aDefault.type())) {
            tmp = aDefault;
        }
        return tmp;
    }
    case QMetaType::QPoint: {
        const auto list = asIntList(value);

        if (list.count() != 2) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault) << formatError(2, list.count());
            return aDefault;
        }
        return QPoint(list.at(0), list.at(1));
    }
    case QMetaType::QPointF: {
        const auto list = asRealList(value);

        if (list.count() != 2) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault) << formatError(2, list.count());
            return aDefault;
        }
        return QPointF(list.at(0), list.at(1));
    }
    case QMetaType::QRect: {
        const auto list = asIntList(value);

        if (list.count() != 4) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault) << formatError(4, list.count());
            return aDefault;
        }
        const QRect rect(list.at(0), list.at(1), list.at(2), list.at(3));
        if (!rect.isValid()) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault);
            return aDefault;
        }
        return rect;
    }
    case QMetaType::QRectF: {
        const auto list = asRealList(value);

        if (list.count() != 4) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault) << formatError(4, list.count());
            return aDefault;
        }
        const QRectF rect(list.at(0), list.at(1), list.at(2), list.at(3));
        if (!rect.isValid()) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault);
            return aDefault;
        }
        return rect;
    }
    case QMetaType::QSize: {
        const auto list = asIntList(value);

        if (list.count() != 2) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault) << formatError(2, list.count());
            return aDefault;
        }
        const QSize size(list.at(0), list.at(1));
        if (!size.isValid()) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault);
            return aDefault;
        }
        return size;
    }
    case QMetaType::QSizeF: {
        const auto list = asRealList(value);

        if (list.count() != 2) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault) << formatError(2, list.count());
            return aDefault;
        }
        const QSizeF size(list.at(0), list.at(1));
        if (!size.isValid()) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault);
            return aDefault;
        }
        return size;
    }
    case QMetaType::QDateTime: {
        const auto list = asRealList(value);
        if (list.count() < 6) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault) << formatError(6, list.count());
            return aDefault;
        }
        const QDate date(list.at(0), list.at(1), list.at(2));
        const qreal totalSeconds = list.at(5);
        qreal seconds;
        const qreal fractional = modf(totalSeconds, &seconds);
        const qreal milliseconds = round(fractional * 1000.0);
        const QTime time(list.at(3), list.at(4), seconds, milliseconds);
        const QDateTime dt(date, time);
        if (!dt.isValid()) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault);
            return aDefault;
        }
        return dt;
    }
    case QMetaType::QDate: {
        auto list = asIntList(value);
        if (list.count() == 6) {
            list = list.mid(0, 3); // don't break config files that stored QDate as QDateTime
        }
        if (list.count() != 3) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault) << formatError(3, list.count());
            return aDefault;
        }
        const QDate date(list.at(0), list.at(1), list.at(2));
        if (!date.isValid()) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault);
            return aDefault;
        }
        return date;
    }
    case QMetaType::QColor:
    case QMetaType::QFont:
        qCWarning(KCONFIG_CORE_LOG) << "KConfigGroup::readEntry was passed GUI type '" << aDefault.typeName()
                                    << "' but KConfigGui isn't linked! If it is linked to your program, "
                                       "this is a platform bug. Please inform the KDE developers";
        break;
    case QMetaType::QUrl:
        return QUrl(QString::fromUtf8(value));

    default:
        break;
    }

    qCWarning(KCONFIG_CORE_LOG) << "unhandled type " << aDefault.typeName();
    return QVariant();
}

#ifdef Q_OS_WIN
#include <QDir>
#endif

static bool cleanHomeDirPath(QString &path, const QString &homeDir)
{
#ifdef Q_OS_WIN // safer
    if (!QDir::toNativeSeparators(path).startsWith(QDir::toNativeSeparators(homeDir))) {
        return false;
    }
#else
    if (!path.startsWith(homeDir)) {
        return false;
    }
#endif

    int len = homeDir.length();
    // replace by "$HOME" if possible
    if (len && (path.length() == len || path[len] == QLatin1Char('/'))) {
        path.replace(0, len, QStringLiteral("$HOME"));
        return true;
    }

    return false;
}

static QString translatePath(QString path) // krazy:exclude=passbyvalue
{
    if (path.isEmpty()) {
        return path;
    }

    // only "our" $HOME should be interpreted
    path.replace(QLatin1Char('$'), QLatin1String("$$"));

    const bool startsWithFile = path.startsWith(QLatin1String("file:"), Qt::CaseInsensitive);
    path = startsWithFile ? QUrl(path).toLocalFile() : path;

    if (QDir::isRelativePath(path)) {
        return path;
    }

    // we can not use KGlobal::dirs()->relativeLocation("home", path) here,
    // since it would not recognize paths without a trailing '/'.
    // All of the 3 following functions to return the user's home directory
    // can return different paths. We have to test all them.
    const QString homeDir0 = QFile::decodeName(qgetenv("HOME"));
    const QString homeDir1 = QDir::homePath();
    const QString homeDir2 = QDir(homeDir1).canonicalPath();
    if (cleanHomeDirPath(path, homeDir0) || cleanHomeDirPath(path, homeDir1) || cleanHomeDirPath(path, homeDir2)) {
        // qDebug() << "Path was replaced\n";
    }

    if (startsWithFile) {
        path = QUrl::fromLocalFile(path).toString();
    }

    return path;
}

KConfigGroup::KConfigGroup()
    : d()
{
}

bool KConfigGroup::isValid() const
{
    return bool(d);
}

KConfigGroupGui _kde_internal_KConfigGroupGui;
static inline bool readEntryGui(const QByteArray &data, const char *key, const QVariant &input, QVariant &output)
{
    if (_kde_internal_KConfigGroupGui.readEntryGui) {
        return _kde_internal_KConfigGroupGui.readEntryGui(data, key, input, output);
    }
    return false;
}

static inline bool writeEntryGui(KConfigGroup *cg, const char *key, const QVariant &input, KConfigGroup::WriteConfigFlags flags)
{
    if (_kde_internal_KConfigGroupGui.writeEntryGui) {
        return _kde_internal_KConfigGroupGui.writeEntryGui(cg, key, input, flags);
    }
    return false;
}

KConfigGroup::KConfigGroup(KConfigBase *master, const QString &_group)
    : d(KConfigGroupPrivate::create(master, _group.toUtf8(), master->isGroupImmutable(_group), false))
{
}

KConfigGroup::KConfigGroup(KConfigBase *master, const char *_group)
    : d(KConfigGroupPrivate::create(master, _group, master->isGroupImmutable(_group), false))
{
}

KConfigGroup::KConfigGroup(const KConfigBase *master, const QString &_group)
    : d(KConfigGroupPrivate::create(const_cast<KConfigBase *>(master), _group.toUtf8(), master->isGroupImmutable(_group), true))
{
}

KConfigGroup::KConfigGroup(const KConfigBase *master, const char *_group)
    : d(KConfigGroupPrivate::create(const_cast<KConfigBase *>(master), _group, master->isGroupImmutable(_group), true))
{
}

KConfigGroup::KConfigGroup(const KSharedConfigPtr &master, const QString &_group)
    : d(new KConfigGroupPrivate(master, _group.toUtf8()))
{
}

KConfigGroup::KConfigGroup(const KSharedConfigPtr &master, const char *_group)
    : d(new KConfigGroupPrivate(master, _group))
{
}

KConfigGroup &KConfigGroup::operator=(const KConfigGroup &rhs)
{
    d = rhs.d;
    return *this;
}

KConfigGroup::KConfigGroup(const KConfigGroup &rhs)
    : d(rhs.d)
{
}

KConfigGroup::~KConfigGroup()
{
    d.reset();
}

KConfigGroup KConfigGroup::groupImpl(const QByteArray &aGroup)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::groupImpl", "accessing an invalid group");
    Q_ASSERT_X(!aGroup.isEmpty(), "KConfigGroup::groupImpl", "can not have an unnamed child group");

    KConfigGroup newGroup;

    newGroup.d = new KConfigGroupPrivate(this, isGroupImmutableImpl(aGroup), d->bConst, aGroup);

    return newGroup;
}

const KConfigGroup KConfigGroup::groupImpl(const QByteArray &aGroup) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::groupImpl", "accessing an invalid group");
    Q_ASSERT_X(!aGroup.isEmpty(), "KConfigGroup::groupImpl", "can not have an unnamed child group");

    KConfigGroup newGroup;

    newGroup.d = new KConfigGroupPrivate(const_cast<KConfigGroup *>(this), isGroupImmutableImpl(aGroup), true, aGroup);

    return newGroup;
}

KConfigGroup KConfigGroup::parent() const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::parent", "accessing an invalid group");

    KConfigGroup parentGroup;

    if (d->mParent) {
        parentGroup.d = d->mParent;
    } else {
        parentGroup.d = new KConfigGroupPrivate(d->mOwner, d->mOwner->isImmutable(), d->bConst, "");
        // make sure we keep the refcount up on the KConfig object
        parentGroup.d->sOwner = d->sOwner;
    }

    return parentGroup;
}

void KConfigGroup::deleteGroup(WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::deleteGroup", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::deleteGroup", "deleting a read-only group");

    config()->deleteGroup(d->fullName(), flags);
}

#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 0)
void KConfigGroup::changeGroup(const QString &group)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::changeGroup", "accessing an invalid group");
    d.detach();
    d->mName = group.toUtf8();
}
#endif

#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 0)
void KConfigGroup::changeGroup(const char *group)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::changeGroup", "accessing an invalid group");
    d.detach();
    d->mName = group;
}
#endif

QString KConfigGroup::name() const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::name", "accessing an invalid group");

    return QString::fromUtf8(d->name());
}

bool KConfigGroup::exists() const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::exists", "accessing an invalid group");

    return config()->hasGroup(d->fullName());
}

bool KConfigGroup::sync()
{
    Q_ASSERT_X(isValid(), "KConfigGroup::sync", "accessing an invalid group");

    if (!d->bConst) {
        return config()->sync();
    }

    return false;
}

QMap<QString, QString> KConfigGroup::entryMap() const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::entryMap", "accessing an invalid group");

    return config()->entryMap(QString::fromUtf8(d->fullName()));
}

KConfig *KConfigGroup::config()
{
    Q_ASSERT_X(isValid(), "KConfigGroup::config", "accessing an invalid group");

    return d->mOwner;
}

const KConfig *KConfigGroup::config() const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::config", "accessing an invalid group");

    return d->mOwner;
}

bool KConfigGroup::isEntryImmutable(const char *key) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::isEntryImmutable", "accessing an invalid group");

    return (isImmutable() || !config()->d_func()->canWriteEntry(d->fullName(), key, config()->readDefaults()));
}

bool KConfigGroup::isEntryImmutable(const QString &key) const
{
    return isEntryImmutable(key.toUtf8().constData());
}

QString KConfigGroup::readEntryUntranslated(const QString &pKey, const QString &aDefault) const
{
    return readEntryUntranslated(pKey.toUtf8().constData(), aDefault);
}

QString KConfigGroup::readEntryUntranslated(const char *key, const QString &aDefault) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::readEntryUntranslated", "accessing an invalid group");

    QString result = config()->d_func()->lookupData(d->fullName(), key, KEntryMap::SearchFlags(), nullptr);
    if (result.isNull()) {
        return aDefault;
    }
    return result;
}

QString KConfigGroup::readEntry(const char *key, const char *aDefault) const
{
    return readEntry(key, QString::fromUtf8(aDefault));
}

QString KConfigGroup::readEntry(const QString &key, const char *aDefault) const
{
    return readEntry(key.toUtf8().constData(), aDefault);
}

QString KConfigGroup::readEntry(const char *key, const QString &aDefault) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::readEntry", "accessing an invalid group");

    bool expand = false;

    // read value from the entry map
    QString aValue = config()->d_func()->lookupData(d->fullName(), key, KEntryMap::SearchLocalized, &expand);
    if (aValue.isNull()) {
        aValue = aDefault;
    }

    if (expand) {
        return KConfigPrivate::expandString(aValue);
    }

    return aValue;
}

QString KConfigGroup::readEntry(const QString &key, const QString &aDefault) const
{
    return readEntry(key.toUtf8().constData(), aDefault);
}

QStringList KConfigGroup::readEntry(const char *key, const QStringList &aDefault) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::readEntry", "accessing an invalid group");

    const QString data = readEntry(key, QString());
    if (data.isNull()) {
        return aDefault;
    }

    return KConfigGroupPrivate::deserializeList(data);
}

QStringList KConfigGroup::readEntry(const QString &key, const QStringList &aDefault) const
{
    return readEntry(key.toUtf8().constData(), aDefault);
}

QVariant KConfigGroup::readEntry(const char *key, const QVariant &aDefault) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::readEntry", "accessing an invalid group");

    const QByteArray data = config()->d_func()->lookupData(d->fullName(), key, KEntryMap::SearchLocalized);
    if (data.isNull()) {
        return aDefault;
    }

    QVariant value;
    if (!readEntryGui(data, key, aDefault, value)) {
        return convertToQVariant(key, data, aDefault);
    }

    return value;
}

QVariant KConfigGroup::readEntry(const QString &key, const QVariant &aDefault) const
{
    return readEntry(key.toUtf8().constData(), aDefault);
}

QVariantList KConfigGroup::readEntry(const char *key, const QVariantList &aDefault) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::readEntry", "accessing an invalid group");

    const QString data = readEntry(key, QString());
    if (data.isNull()) {
        return aDefault;
    }

    const auto &list = KConfigGroupPrivate::deserializeList(data);

    QVariantList value;
    value.reserve(list.count());
    for (const QString &v : list) {
        value << v;
    }

    return value;
}

QVariantList KConfigGroup::readEntry(const QString &key, const QVariantList &aDefault) const
{
    return readEntry(key.toUtf8().constData(), aDefault);
}

QStringList KConfigGroup::readXdgListEntry(const QString &key, const QStringList &aDefault) const
{
    return readXdgListEntry(key.toUtf8().constData(), aDefault);
}

QStringList KConfigGroup::readXdgListEntry(const char *key, const QStringList &aDefault) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::readXdgListEntry", "accessing an invalid group");

    const QString data = readEntry(key, QString());
    if (data.isNull()) {
        return aDefault;
    }

    QStringList value;
    QString val;
    val.reserve(data.size());
    // XXX List serialization being a separate layer from low-level parsing is
    // probably a bug. No affected entries are defined, though.
    bool quoted = false;
    for (int p = 0; p < data.length(); p++) {
        if (quoted) {
            val += data[p];
            quoted = false;
        } else if (data[p] == QLatin1Char('\\')) {
            quoted = true;
        } else if (data[p] == QLatin1Char(';')) {
            value.append(val);
            val.clear();
            val.reserve(data.size() - p);
        } else {
            val += data[p];
        }
    }
    if (!val.isEmpty()) {
        value.append(val);
    }
    return value;
}

QString KConfigGroup::readPathEntry(const QString &pKey, const QString &aDefault) const
{
    return readPathEntry(pKey.toUtf8().constData(), aDefault);
}

QString KConfigGroup::readPathEntry(const char *key, const QString &aDefault) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::readPathEntry", "accessing an invalid group");

    bool expand = false;

    QString aValue = config()->d_func()->lookupData(d->fullName(), key, KEntryMap::SearchLocalized, &expand);
    if (aValue.isNull()) {
        aValue = aDefault;
    }

    return KConfigPrivate::expandString(aValue);
}

QStringList KConfigGroup::readPathEntry(const QString &pKey, const QStringList &aDefault) const
{
    return readPathEntry(pKey.toUtf8().constData(), aDefault);
}

QStringList KConfigGroup::readPathEntry(const char *key, const QStringList &aDefault) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::readPathEntry", "accessing an invalid group");

    const QString data = readPathEntry(key, QString());
    if (data.isNull()) {
        return aDefault;
    }

    return KConfigGroupPrivate::deserializeList(data);
}

void KConfigGroup::writeEntry(const char *key, const QString &value, WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::writeEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::writeEntry", "writing to a read-only group");

    writeEntry(key, value.toUtf8(), flags);
}

void KConfigGroup::writeEntry(const QString &key, const QString &value, WriteConfigFlags flags)
{
    writeEntry(key.toUtf8().constData(), value, flags);
}

void KConfigGroup::writeEntry(const QString &key, const char *value, WriteConfigFlags pFlags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::writeEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::writeEntry", "writing to a read-only group");

    writeEntry(key.toUtf8().constData(), QVariant(QString::fromLatin1(value)), pFlags);
}

void KConfigGroup::writeEntry(const char *key, const char *value, WriteConfigFlags pFlags)
{
    writeEntry(key, QVariant(QString::fromLatin1(value)), pFlags);
}

void KConfigGroup::writeEntry(const char *key, const QByteArray &value, WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::writeEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::writeEntry", "writing to a read-only group");

    config()->d_func()->putData(d->fullName(), key, value.isNull() ? QByteArray("") : value, flags);
}

void KConfigGroup::writeEntry(const QString &key, const QByteArray &value, WriteConfigFlags pFlags)
{
    writeEntry(key.toUtf8().constData(), value, pFlags);
}

void KConfigGroup::writeEntry(const char *key, const QStringList &list, WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::writeEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::writeEntry", "writing to a read-only group");

    QList<QByteArray> balist;
    balist.reserve(list.count());

    for (const QString &entry : list) {
        balist.append(entry.toUtf8());
    }

    writeEntry(key, KConfigGroupPrivate::serializeList(balist), flags);
}

void KConfigGroup::writeEntry(const QString &key, const QStringList &list, WriteConfigFlags flags)
{
    writeEntry(key.toUtf8().constData(), list, flags);
}

void KConfigGroup::writeEntry(const char *key, const QVariantList &list, WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::writeEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::writeEntry", "writing to a read-only group");

    QList<QByteArray> data;
    data.reserve(list.count());

    for (const QVariant &v : list) {
        if (v.type() == QVariant::ByteArray) {
            data << v.toByteArray();
        } else {
            data << v.toString().toUtf8();
        }
    }

    writeEntry(key, KConfigGroupPrivate::serializeList(data), flags);
}

void KConfigGroup::writeEntry(const char *key, const QVariant &value, WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::writeEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::writeEntry", "writing to a read-only group");

    if (writeEntryGui(this, key, value, flags)) {
        return; // GUI type that was handled
    }

    QByteArray data;
    // if a type handler is added here you must add a QVConversions definition
    // to conversioncheck.h, or ConversionCheck::to_QVariant will not allow
    // writeEntry<T> to convert to QVariant.
    switch (static_cast<QMetaType::Type>(value.type())) {
    case QMetaType::UnknownType:
        data = "";
        break;
    case QMetaType::QByteArray:
        data = value.toByteArray();
        break;
    case QMetaType::QString:
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::Double:
    case QMetaType::Float:
    case QMetaType::Bool:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
        data = value.toString().toUtf8();
        break;
    case QMetaType::QVariantList:
        if (!value.canConvert(QMetaType::QStringList))
            qCWarning(KCONFIG_CORE_LOG) << "not all types in \"" << key
                                        << "\" can convert to QString,"
                                           " information will be lost";
        Q_FALLTHROUGH();
    case QMetaType::QStringList:
        writeEntry(key, value.toList(), flags);
        return;
    case QMetaType::QPoint: {
        const QPoint rPoint = value.toPoint();

        const QVariantList list{rPoint.x(), rPoint.y()};

        writeEntry(key, list, flags);
        return;
    }
    case QMetaType::QPointF: {
        const QPointF point = value.toPointF();

        const QVariantList list{point.x(), point.y()};

        writeEntry(key, list, flags);
        return;
    }
    case QMetaType::QRect: {
        const QRect rRect = value.toRect();

        const QVariantList list{rRect.left(), rRect.top(), rRect.width(), rRect.height()};

        writeEntry(key, list, flags);
        return;
    }
    case QMetaType::QRectF: {
        const QRectF rRectF = value.toRectF();

        const QVariantList list{rRectF.left(), rRectF.top(), rRectF.width(), rRectF.height()};

        writeEntry(key, list, flags);
        return;
    }
    case QMetaType::QSize: {
        const QSize rSize = value.toSize();

        const QVariantList list{rSize.width(), rSize.height()};

        writeEntry(key, list, flags);
        return;
    }
    case QMetaType::QSizeF: {
        const QSizeF rSizeF = value.toSizeF();

        const QVariantList list{rSizeF.width(), rSizeF.height()};

        writeEntry(key, list, flags);
        return;
    }
    case QMetaType::QDate: {
        const QDate date = value.toDate();

        const QVariantList list{date.year(), date.month(), date.day()};

        writeEntry(key, list, flags);
        return;
    }
    case QMetaType::QDateTime: {
        const QDateTime rDateTime = value.toDateTime();

        const QTime time = rDateTime.time();
        const QDate date = rDateTime.date();

        const QVariantList list{
            date.year(),
            date.month(),
            date.day(),

            time.hour(),
            time.minute(),
            time.second() + time.msec() / 1000.0,
        };

        writeEntry(key, list, flags);
        return;
    }

    case QMetaType::QColor:
    case QMetaType::QFont:
        qCWarning(KCONFIG_CORE_LOG) << "KConfigGroup::writeEntry was passed GUI type '" << value.typeName()
                                    << "' but KConfigGui isn't linked! If it is linked to your program, this is a platform bug. "
                                       "Please inform the KDE developers";
        break;
    case QMetaType::QUrl:
        data = QUrl(value.toUrl()).toString().toUtf8();
        break;
    default:
        qCWarning(KCONFIG_CORE_LOG) << "KConfigGroup::writeEntry - unhandled type" << value.typeName() << "in group" << name();
    }

    writeEntry(key, data, flags);
}

void KConfigGroup::writeEntry(const QString &key, const QVariant &value, WriteConfigFlags flags)
{
    writeEntry(key.toUtf8().constData(), value, flags);
}

void KConfigGroup::writeEntry(const QString &key, const QVariantList &list, WriteConfigFlags flags)
{
    writeEntry(key.toUtf8().constData(), list, flags);
}

void KConfigGroup::writeXdgListEntry(const QString &key, const QStringList &value, WriteConfigFlags pFlags)
{
    writeXdgListEntry(key.toUtf8().constData(), value, pFlags);
}

void KConfigGroup::writeXdgListEntry(const char *key, const QStringList &list, WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::writeXdgListEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::writeXdgListEntry", "writing to a read-only group");

    QString value;
    value.reserve(4096);

    // XXX List serialization being a separate layer from low-level escaping is
    // probably a bug. No affected entries are defined, though.
    for (QString val : list) { // clazy:exclude=range-loop
        val.replace(QLatin1Char('\\'), QLatin1String("\\\\")).replace(QLatin1Char(';'), QLatin1String("\\;"));
        value += val + QLatin1Char(';');
    }

    writeEntry(key, value, flags);
}

void KConfigGroup::writePathEntry(const QString &pKey, const QString &path, WriteConfigFlags pFlags)
{
    writePathEntry(pKey.toUtf8().constData(), path, pFlags);
}

void KConfigGroup::writePathEntry(const char *pKey, const QString &path, WriteConfigFlags pFlags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::writePathEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::writePathEntry", "writing to a read-only group");

    config()->d_func()->putData(d->fullName(), pKey, translatePath(path).toUtf8(), pFlags, true);
}

void KConfigGroup::writePathEntry(const QString &pKey, const QStringList &value, WriteConfigFlags pFlags)
{
    writePathEntry(pKey.toUtf8().constData(), value, pFlags);
}

void KConfigGroup::writePathEntry(const char *pKey, const QStringList &value, WriteConfigFlags pFlags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::writePathEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::writePathEntry", "writing to a read-only group");

    QList<QByteArray> list;
    list.reserve(value.length());
    for (const QString &path : value) {
        list << translatePath(path).toUtf8();
    }

    config()->d_func()->putData(d->fullName(), pKey, KConfigGroupPrivate::serializeList(list), pFlags, true);
}

void KConfigGroup::deleteEntry(const char *key, WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::deleteEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::deleteEntry", "deleting from a read-only group");

    config()->d_func()->putData(d->fullName(), key, QByteArray(), flags);
}

void KConfigGroup::deleteEntry(const QString &key, WriteConfigFlags flags)
{
    deleteEntry(key.toUtf8().constData(), flags);
}

void KConfigGroup::revertToDefault(const char *key)
{
    revertToDefault(key, WriteConfigFlags());
}

void KConfigGroup::revertToDefault(const char *key, WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::revertToDefault", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::revertToDefault", "writing to a read-only group");

    config()->d_func()->revertEntry(d->fullName(), key, flags);
}

void KConfigGroup::revertToDefault(const QString &key)
{
    revertToDefault(key, WriteConfigFlags());
}

void KConfigGroup::revertToDefault(const QString &key, WriteConfigFlags flags)
{
    revertToDefault(key.toUtf8().constData(), flags);
}

bool KConfigGroup::hasDefault(const char *key) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::hasDefault", "accessing an invalid group");

    KEntryMap::SearchFlags flags = KEntryMap::SearchDefaults | KEntryMap::SearchLocalized;

    return !config()->d_func()->lookupData(d->fullName(), key, flags).isNull();
}

bool KConfigGroup::hasDefault(const QString &key) const
{
    return hasDefault(key.toUtf8().constData());
}

bool KConfigGroup::hasKey(const char *key) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::hasKey", "accessing an invalid group");

    KEntryMap::SearchFlags flags = KEntryMap::SearchLocalized;
    if (config()->readDefaults()) {
        flags |= KEntryMap::SearchDefaults;
    }

    return !config()->d_func()->lookupData(d->fullName(), key, flags).isNull();
}

bool KConfigGroup::hasKey(const QString &key) const
{
    return hasKey(key.toUtf8().constData());
}

bool KConfigGroup::isImmutable() const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::isImmutable", "accessing an invalid group");

    return d->bImmutable;
}

QStringList KConfigGroup::groupList() const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::groupList", "accessing an invalid group");

    return config()->d_func()->groupList(d->fullName());
}

QStringList KConfigGroup::keyList() const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::keyList", "accessing an invalid group");

    return config()->keyList(QString::fromUtf8(d->fullName()));
}

void KConfigGroup::markAsClean()
{
    Q_ASSERT_X(isValid(), "KConfigGroup::markAsClean", "accessing an invalid group");

    config()->markAsClean();
}

KConfigGroup::AccessMode KConfigGroup::accessMode() const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::accessMode", "accessing an invalid group");

    return config()->accessMode();
}

bool KConfigGroup::hasGroupImpl(const QByteArray &b) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::hasGroupImpl", "accessing an invalid group");

    return config()->hasGroup(d->fullName(b));
}

void KConfigGroup::deleteGroupImpl(const QByteArray &b, WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::deleteGroupImpl", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::deleteGroupImpl", "deleting from a read-only group");

    config()->deleteGroup(d->fullName(b), flags);
}

bool KConfigGroup::isGroupImmutableImpl(const QByteArray &b) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::isGroupImmutableImpl", "accessing an invalid group");

    if (!hasGroupImpl(b)) { // group doesn't exist yet
        return d->bImmutable; // child groups are immutable if the parent is immutable.
    }

    return config()->isGroupImmutable(d->fullName(b));
}

void KConfigGroup::copyTo(KConfigBase *other, WriteConfigFlags pFlags) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::copyTo", "accessing an invalid group");
    Q_ASSERT(other != nullptr);

    if (KConfigGroup *otherGroup = dynamic_cast<KConfigGroup *>(other)) {
        config()->d_func()->copyGroup(d->fullName(), otherGroup->d->fullName(), otherGroup, pFlags);
    } else if (KConfig *otherConfig = dynamic_cast<KConfig *>(other)) {
        KConfigGroup newGroup = otherConfig->group(d->fullName());
        otherConfig->d_func()->copyGroup(d->fullName(), d->fullName(), &newGroup, pFlags);
    } else {
        Q_ASSERT_X(false, "KConfigGroup::copyTo", "unknown type of KConfigBase");
    }
}

void KConfigGroup::reparent(KConfigBase *parent, WriteConfigFlags pFlags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::reparent", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::reparent", "reparenting a read-only group");
    Q_ASSERT_X(!d->bImmutable, "KConfigGroup::reparent", "reparenting an immutable group");
    Q_ASSERT(parent != nullptr);

    KConfigGroup oldGroup(*this);

    d = KConfigGroupPrivate::create(parent, d->mName, false, false);
    oldGroup.copyTo(this, pFlags);
    oldGroup.deleteGroup(); // so that the entries with the old group name are deleted on sync
}
