/*
 * SPDX-FileCopyrightText: 2026 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kconfigpropertymaptest.h"

#include <KConfig>
#include <KConfigGroup>
#include <KConfigLoader>
#include <KConfigPropertyMap>

#include <QFile>
#include <QJSValue>
#include <QSignalSpy>
#include <QStandardPaths>

Q_DECLARE_METATYPE(QList<int>)

static const QString g_testName = QStringLiteral("kconfigpropertymaptest"); // clazy:exclude=non-pod-global-static
static const QString g_groupName = QStringLiteral("kconfigloadertest");

static QString configFilePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + g_testName;
}

// KConfigPropertyMap exposes both the current value and the default of every item.
#define CHECK_PROPERTY_VALUES(configName, expectedValue)                                                                                                       \
    QCOMPARE(m_propertyMap->value(QStringLiteral(configName)), QVariant::fromValue(expectedValue));                                                            \
    QCOMPARE(m_propertyMap->value(QStringLiteral(configName "Default")), QVariant::fromValue(expectedValue));

void ConfigPropertyMapTest::resetPropertyMap()
{
    m_propertyMap.reset();
    m_loader.reset();
}

void ConfigPropertyMapTest::initPropertyMap()
{
    resetPropertyMap();

    QFile configFile(QFINDTESTDATA(QStringLiteral("kconfigloadertest.xml")));
    m_loader = std::make_unique<KConfigLoader>(m_config, &configFile);
    m_propertyMap = std::make_unique<KConfigPropertyMap>(m_loader.get());
}

void ConfigPropertyMapTest::init()
{
    QStandardPaths::setTestModeEnabled(true);

    // Drop leftovers from a previous run (in particular the immutable entry written by isImmutable()).
    m_config.reset();
    QFile::remove(configFilePath());

    m_config = KSharedConfig::openConfig(g_testName, KConfig::SimpleConfig);
    initPropertyMap();
}

void ConfigPropertyMapTest::cleanup()
{
    resetPropertyMap();
    m_config.reset();
    QFile::remove(configFilePath());
}

void ConfigPropertyMapTest::boolDefaultValue()
{
    CHECK_PROPERTY_VALUES("DefaultBoolItem", true);
}

void ConfigPropertyMapTest::colorDefaultValue()
{
    CHECK_PROPERTY_VALUES("DefaultColorItem", QColor(0x00FF00));
}

void ConfigPropertyMapTest::dateTimeDefaultValue()
{
    CHECK_PROPERTY_VALUES("DefaultDateTimeItem", QDateTime::fromString(QStringLiteral("Thu Sep 09 2010")));
}

void ConfigPropertyMapTest::enumDefaultValue()
{
    CHECK_PROPERTY_VALUES("DefaultEnumItem", 3);
}

void ConfigPropertyMapTest::enumDefaultValueString()
{
    CHECK_PROPERTY_VALUES("DefaultEnumItemString", 2);
}

void ConfigPropertyMapTest::fontDefaultValue()
{
    CHECK_PROPERTY_VALUES("DefaultFontItem", QFont(QStringLiteral("DejaVu Sans")));
}

void ConfigPropertyMapTest::intDefaultValue()
{
    CHECK_PROPERTY_VALUES("DefaultIntItem", 27);
}

void ConfigPropertyMapTest::passwordDefaultValue()
{
    CHECK_PROPERTY_VALUES("DefaultPasswordItem", QStringLiteral("h4x."));
}

void ConfigPropertyMapTest::pathDefaultValue()
{
    CHECK_PROPERTY_VALUES("DefaultPathItem", QStringLiteral("/dev/null"));
}

void ConfigPropertyMapTest::stringDefaultValue()
{
    CHECK_PROPERTY_VALUES("DefaultStringItem", QStringLiteral("TestString"));
}

void ConfigPropertyMapTest::stringListDefaultValue()
{
    const QStringList expected{QStringLiteral("One"), QStringLiteral("Two"), QStringLiteral("Three"), QStringLiteral("Four"), QStringLiteral("Five")};
    CHECK_PROPERTY_VALUES("DefaultStringListItem", expected);
}

void ConfigPropertyMapTest::uintDefaultValue()
{
    CHECK_PROPERTY_VALUES("DefaultUIntItem", 7U);
}

void ConfigPropertyMapTest::urlDefaultValue()
{
    CHECK_PROPERTY_VALUES("DefaultUrlItem", QUrl(QStringLiteral("http://kde.org")));
}

void ConfigPropertyMapTest::doubleDefaultValue()
{
    CHECK_PROPERTY_VALUES("DefaultDoubleItem", 13.37);
}

void ConfigPropertyMapTest::intListEmptyDefaultValue()
{
    CHECK_PROPERTY_VALUES("EmptyDefaultIntListItem", QVariant::fromValue(QList<int>()));
}

void ConfigPropertyMapTest::intListDefaultValue()
{
    const QList<int> expected{1, 1, 2, 3, 5, 8};
    CHECK_PROPERTY_VALUES("DefaultIntListItem", expected);
}

void ConfigPropertyMapTest::longLongDefaultValue()
{
    CHECK_PROPERTY_VALUES("DefaultLongLongItem", Q_INT64_C(-9211372036854775808));
}

void ConfigPropertyMapTest::pointDefaultValue()
{
    CHECK_PROPERTY_VALUES("DefaultPointItem", QPoint(185, 857));
}

void ConfigPropertyMapTest::pointFDefaultValue()
{
    CHECK_PROPERTY_VALUES("DefaultPointFItem", QPointF(185.5, 857.5));
}

void ConfigPropertyMapTest::rectDefaultValue()
{
    QRect expected;
    expected.setCoords(3, 7, 951, 358);
    CHECK_PROPERTY_VALUES("DefaultRectItem", expected);
}

void ConfigPropertyMapTest::rectFDefaultValue()
{
    QRectF expected;
    expected.setCoords(3.5, 7.5, 951.5, 358.5);
    CHECK_PROPERTY_VALUES("DefaultRectFItem", expected);
}

void ConfigPropertyMapTest::sizeDefaultValue()
{
    CHECK_PROPERTY_VALUES("DefaultSizeItem", QSize(640, 480));
}

void ConfigPropertyMapTest::sizeFDefaultValue()
{
    CHECK_PROPERTY_VALUES("DefaultSizeFItem", QSizeF(640.5, 480.5));
}

void ConfigPropertyMapTest::ulongLongDefaultValue()
{
    CHECK_PROPERTY_VALUES("DefaultULongLongItem", Q_UINT64_C(9223372036854775806));
}

void ConfigPropertyMapTest::timeDefaultValue()
{
    CHECK_PROPERTY_VALUES("DefaultTimeItem", QTime(21, 42, 32));
}

void ConfigPropertyMapTest::valueChangedUpdatesConfig()
{
    QSignalSpy spy{m_propertyMap.get(), &QQmlPropertyMap::valueChanged};

    // Writes through a property (as they happen from QML) are propagated to the config item.
    QVERIFY(m_propertyMap->setProperty("DefaultIntItem", 42));
    QVERIFY(m_propertyMap->setProperty("DefaultStringItem", QStringLiteral("Changed")));

    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("DefaultIntItem"));
    QCOMPARE(spy.at(0).at(1).toInt(), 42);
    QCOMPARE(spy.at(1).at(0).toString(), QStringLiteral("DefaultStringItem"));
    QCOMPARE(spy.at(1).at(1).toString(), QStringLiteral("Changed"));

    QCOMPARE(m_propertyMap->value(QStringLiteral("DefaultIntItem")), QVariant(42));
    QCOMPARE(m_propertyMap->value(QStringLiteral("DefaultIntItemDefault")), QVariant(27));
}

void ConfigPropertyMapTest::writeConfig()
{
    QVERIFY(m_propertyMap->setProperty("DefaultIntItem", 42));
    QVERIFY(m_propertyMap->setProperty("DefaultStringItem", QStringLiteral("Written")));

    m_propertyMap->writeConfig();

    const KConfigGroup group = m_config->group(g_groupName);
    QCOMPARE(group.readEntry(QStringLiteral("DefaultIntItem"), 0), 42);
    QCOMPARE(group.readEntry(QStringLiteral("DefaultStringItem"), QString()), QStringLiteral("Written"));
}

void ConfigPropertyMapTest::configChangedReloadsValues()
{
    QVERIFY(m_propertyMap->setProperty("DefaultIntItem", 42));

    QSignalSpy spy{m_propertyMap.get(), &QQmlPropertyMap::valueChanged};

    // Saving emits configChanged(), which makes the map reload and notify.
    m_propertyMap->writeConfig();

    QCOMPARE(m_propertyMap->value(QStringLiteral("DefaultIntItem")), QVariant(42));

    bool found = false;
    for (const QList<QVariant> &signal : spy) {
        if (signal.at(0).toString() == QStringLiteral("DefaultIntItem") && signal.at(1).toInt() == 42) {
            found = true;
            break;
        }
    }
    QVERIFY(found);
}

void ConfigPropertyMapTest::isImmutable()
{
    QVERIFY(!m_propertyMap->isImmutable(QStringLiteral("DefaultIntItem")));
    QVERIFY(!m_propertyMap->isImmutable(QStringLiteral("DoesNotExist")));

    QFile file(configFilePath());
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write(QStringLiteral("[%1]\nDefaultIntItem[$i]=13\n").arg(g_groupName).toUtf8());
    file.close();

    m_config->reparseConfiguration();

    // Re-initialize the map so it picks up the immutable item from the config file.
    initPropertyMap();

    QVERIFY(m_propertyMap->isImmutable(QStringLiteral("DefaultIntItem")));
    QVERIFY(!m_propertyMap->isImmutable(QStringLiteral("DefaultStringItem")));
}

void ConfigPropertyMapTest::qmlValueConversion()
{
    // Values coming from QML are QJSValue and need to be converted by updateValue().
    const QJSValue jsValue(QStringLiteral("From QML"));
    QVERIFY(m_propertyMap->setProperty("DefaultStringItem", QVariant::fromValue(jsValue)));
    QCOMPARE(m_propertyMap->value(QStringLiteral("DefaultStringItem")), QVariant(QStringLiteral("From QML")));
}

QTEST_MAIN(ConfigPropertyMapTest)

#include "moc_kconfigpropertymaptest.cpp"
