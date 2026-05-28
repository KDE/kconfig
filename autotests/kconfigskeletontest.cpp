/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfigskeletontest.h"

#include <kconfig.h>

#include <QBuffer>
#include <QFont>
#include <QtTestGui>

using namespace Qt::Literals;

QTEST_MAIN(KConfigSkeletonTest)

// clazy:excludeall=non-pod-global-static

static const bool s_default_setting1{false};
static const QColor s_default_setting2{1, 2, 3};
static const QString s_default_setting4{QStringLiteral("Hello World")};

static const bool s_write_setting1{true};
static const QColor s_write_setting2{3, 2, 1};
static const QString s_write_setting4{QStringLiteral("KDE")};

static QFont defaultSetting3()
{
    return QFont{QStringLiteral("helvetica"), 12};
}

static QFont writeSettings3()
{
    return QFont{QStringLiteral("helvetica"), 14};
}

void KConfigSkeletonTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void KConfigSkeletonTest::init()
{
    QFile::remove(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String{"/kconfigskeletontestrc"});
    s = new KConfigSkeleton(QStringLiteral("kconfigskeletontestrc"));
    s->setCurrentGroup(QStringLiteral("MyGroup"));
    itemBool = s->addItemBool(QStringLiteral("MySetting1"), mMyBool, s_default_setting1);
    s->addItemColor(QStringLiteral("MySetting2"), mMyColor, s_default_setting2);

    s->setCurrentGroup(QStringLiteral("MyOtherGroup"));
    s->addItemFont(QStringLiteral("MySetting3"), mMyFont, defaultSetting3());
    s->addItemString(QStringLiteral("MySetting4"), mMyString, s_default_setting4);

    QCOMPARE(mMyBool, s_default_setting1);
    QCOMPARE(mMyColor, s_default_setting2);
    QCOMPARE(mMyFont, defaultSetting3());
    QCOMPARE(mMyString, s_default_setting4);

    QVERIFY(s->isDefaults());
    QVERIFY(!s->isSaveNeeded());
}

void KConfigSkeletonTest::cleanup()
{
    delete s;
}

void KConfigSkeletonTest::testSimple()
{
    mMyBool = s_write_setting1;
    mMyColor = s_write_setting2;
    mMyFont = writeSettings3();
    mMyString = s_write_setting4;

    QVERIFY(s->isSaveNeeded());
    QVERIFY(!s->isDefaults());

    s->save();

    QVERIFY(!s->isSaveNeeded());
    QVERIFY(!s->isDefaults());

    mMyBool = false;
    mMyColor = QColor();
    mMyString.clear();
    mMyFont = QFont();

    QVERIFY(s->isSaveNeeded());
    QVERIFY(!s->isDefaults());

    s->read();

    QVERIFY(!s->isSaveNeeded());
    QVERIFY(!s->isDefaults());

    QCOMPARE(mMyBool, s_write_setting1);
    QCOMPARE(mMyColor, s_write_setting2);
    QCOMPARE(mMyFont, writeSettings3());
    QCOMPARE(mMyString, s_write_setting4);
}

void KConfigSkeletonTest::testRemoveItem()
{
    QVERIFY(s->findItem(QStringLiteral("MySetting1")));
    s->removeItem(QStringLiteral("MySetting1"));
    QVERIFY(!s->findItem(QStringLiteral("MySetting1")));
}

void KConfigSkeletonTest::testClear()
{
    QVERIFY(s->findItem(QStringLiteral("MySetting2")));
    QVERIFY(s->findItem(QStringLiteral("MySetting3")));
    QVERIFY(s->findItem(QStringLiteral("MySetting4")));

    s->clearItems();

    QVERIFY(!s->findItem(QStringLiteral("MySetting2")));
    QVERIFY(!s->findItem(QStringLiteral("MySetting3")));
    QVERIFY(!s->findItem(QStringLiteral("MySetting4")));
}

void KConfigSkeletonTest::testDefaults()
{
    mMyBool = s_write_setting1;
    mMyColor = s_write_setting2;
    mMyFont = writeSettings3();
    mMyString = s_write_setting4;

    QVERIFY(s->isSaveNeeded());
    QVERIFY(!s->isDefaults());

    s->save();

    QVERIFY(!s->isSaveNeeded());
    QVERIFY(!s->isDefaults());

    s->setDefaults();

    QVERIFY(s->isSaveNeeded());
    QVERIFY(s->isDefaults());

    QCOMPARE(mMyBool, s_default_setting1);
    QCOMPARE(mMyColor, s_default_setting2);
    QCOMPARE(mMyFont, defaultSetting3());
    QCOMPARE(mMyString, s_default_setting4);

    s->save();

    QVERIFY(!s->isSaveNeeded());
    QVERIFY(s->isDefaults());
}

void KConfigSkeletonTest::testKConfigDirty()
{
    itemBool->setValue(true);
    itemBool->writeConfig(s->config());
    QVERIFY(s->config()->isDirty());
    s->save();
    QVERIFY(!s->config()->isDirty());

    itemBool->setValue(false);
    itemBool->writeConfig(s->config());
    QVERIFY(s->config()->isDirty());
    s->save();
    QVERIFY(!s->config()->isDirty());
}

void KConfigSkeletonTest::testSaveRead()
{
    itemBool->setValue(true);
    s->save();

    itemBool->setValue(false);
    s->save();

    mMyBool = true;

    s->read();
    QCOMPARE(mMyBool, false);
}

void KConfigSkeletonTest::testKconfigQIODevice()
{
    auto buffer = std::make_shared<QBuffer>();
    QVERIFY(buffer->open(QIODevice::ReadWrite | QIODevice::Text));
    auto config = std::make_unique<KConfig>(buffer, KConfig::OpenFlag::SimpleConfig);

    auto sIODevice = std::make_unique<KConfigSkeleton>(std::move(config), KCoreConfigSkeleton::DisambiguateConstructor::IsStdUniqPtr);
    sIODevice->setCurrentGroup(QStringLiteral("MyGroup"));
    itemBool = sIODevice->addItemBool(QStringLiteral("MySetting1"), mMyBool, s_default_setting1);
    sIODevice->addItemColor(QStringLiteral("MySetting2"), mMyColor, s_default_setting2);

    sIODevice->setCurrentGroup(QStringLiteral("MyOtherGroup"));
    sIODevice->addItemFont(QStringLiteral("MySetting3"), mMyFont, defaultSetting3());
    sIODevice->addItemString(QStringLiteral("MySetting4"), mMyString, s_default_setting4);

    QCOMPARE(mMyBool, s_default_setting1);
    QCOMPARE(mMyColor, s_default_setting2);
    QCOMPARE(mMyFont, defaultSetting3());
    QCOMPARE(mMyString, s_default_setting4);

    QVERIFY(sIODevice->isDefaults());
    QVERIFY(!sIODevice->isSaveNeeded());

    buffer->seek(0);
    QCOMPARE(buffer->size(), 0);

    sIODevice->save();

    // all values are default, nothing is written
    buffer->seek(0);
    QCOMPARE(buffer->size(), 0);

    // sets a value will make the kconfig dirty
    itemBool->setValue(!itemBool->value());

    sIODevice->save();

    buffer->seek(0);

    auto iniData = QString::fromUtf8(buffer->readAll());
    iniData.remove(QLatin1Char('\r'));
    QCOMPARE(iniData.toUtf8(), "[MyGroup]\nMySetting1=true\n");
}

void KConfigSkeletonTest::testReadDefaults()
{
    KConfigSkeleton skeleton(QStringLiteral("kconfigskeletondefaultstestrc"));

    // prepare the defaults file
    const QString defaultsFile = QLatin1String("kconfigskeletondefaultstestrc.defaults");
    const QString defaultsFilePath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + defaultsFile;
    QFile::remove(defaultsFile);
    KConfig defaults(defaultsFile, KConfig::SimpleConfig);

    KConfigGroup group = defaults.group(QStringLiteral("MyOtherGroup"));
    group.writeEntry("MySetting4", "Bla");
    QVERIFY(group.sync());

    group = defaults.group(QStringLiteral("MyGroup"));
    group.writeEntry("MySetting2", QColor(255, 0, 0));
    QVERIFY(group.sync());

    skeleton.config()->addConfigSources(QStringList{defaultsFilePath});

    // build the skeleton
    bool theBool = false;
    QColor theColor;
    QString theString;

    skeleton.setCurrentGroup(QStringLiteral("MyGroup"));
    auto itemBool = skeleton.addItemBool(QStringLiteral("MySetting1"), theBool, s_default_setting1);
    auto itemColor = skeleton.addItemColor(QStringLiteral("MySetting2"), theColor, s_default_setting2);

    skeleton.setCurrentGroup(QStringLiteral("MyOtherGroup"));
    skeleton.addItemString(QStringLiteral("MySetting4"), theString, s_default_setting4);

    // verify initial values
    QCOMPARE(theBool, false);
    QCOMPARE(theColor, QColor(255, 0, 0));
    QCOMPARE(theString, u"Bla"_s);

    // set some user values
    itemBool->setValue(true);
    itemColor->setValue(QColor(0, 244, 0));
    QCOMPARE(theBool, true);
    QCOMPARE(theColor, QColor(0, 244, 0));

    // verify that default values are read
    skeleton.useDefaults(true);
    QCOMPARE(theBool, false);
    QCOMPARE(theString, u"Bla"_s);
    QCOMPARE(theColor, QColor(255, 0, 0));
    skeleton.useDefaults(false);

    // verify that user values are used again
    QCOMPARE(theBool, true);
    QCOMPARE(theString, u"Bla"_s);
    QCOMPARE(theColor, QColor(0, 244, 0));
}

void KConfigSkeletonTest::testAddItem()
{
    KConfigSkeleton skel;

    QList<QUrl> urls;
    QStringList paths;

    skel.addItemUrlList(u"foo"_s, urls, {QUrl(u"https://kde.org"_s)});
    skel.addItemPathList(u"paths"_s, paths, {u"/foo"_s, u"/bar"_s});

    QCOMPARE(urls, {QUrl(u"https://kde.org"_s)});
    QStringList expectedPaths = {u"/foo"_s, u"/bar"_s};
    QCOMPARE(paths, expectedPaths);
}

void KConfigSkeletonTest::testDeleteEntry()
{
    // prepare the defaults file
    const QString defaultsFile = QLatin1String("kconfigskeletondeletetestrc.defaults");
    const QString defaultsFilePath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + defaultsFile;
    QFile::remove(defaultsFile);
    KConfig defaults(defaultsFile, KConfig::SimpleConfig);

    KConfigGroup group = defaults.group(u"Main"_s);
    group.writePathEntry("MyPath", u"/foo/bar"_s);
    group.writeEntry("MyString", u"Yo"_s);
    group.writeEntry("MyInt", 41);
    group.writeEntry("MyPaths", QStringList{u"/foo"_s, u"/bar"_s});
    group.writeEntry("MyUrls", QStringList(u"https://linux.kde.org"_s));
    group.writeEntry("MyStrings", QStringList(u"a"_s));
    group.writeEntry("MyInts", {1, 2, 3});
    group.writeEntry("MyEnum", u"hello"_s);
    group.writeEntry("MyColor", QColor(234, 234, 234));
    group.writeEntry("MyFont", QFont(u"Comic Sans"_s));
    QVERIFY(group.sync());

    // prepare user file with deleted entry
    const QString userFile = QLatin1String("kconfigskeletondeletetestrc");
    const QString userFilePath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + userFile;
    QFile::remove(userFilePath);
    KConfig c(userFile);
    c.addConfigSources(QStringList{defaultsFilePath});
    auto g = c.group(u"Main"_s);
    g.deleteEntry("MyPath");
    g.deleteEntry("MyString");
    g.deleteEntry("MyInt");
    g.deleteEntry("MyPaths");
    g.deleteEntry("MyUrls");
    g.deleteEntry("MyStrings");
    g.deleteEntry("MyInts");
    g.deleteEntry("MyEnum");
    g.deleteEntry("MyColor");
    g.deleteEntry("MyFont");
    c.sync();

    // build skeleton
    KConfigSkeleton skeleton(QStringLiteral("kconfigskeletondeletetestrc"));
    skeleton.config()->addConfigSources(QStringList{defaultsFilePath});

    QString myPath;
    QString myString;
    int myInt;
    QStringList myPaths;
    QList<QUrl> myUrls;
    QStringList myStrings;
    QList<int> myInts;
    int myEnum;
    QColor myColor;
    QFont myFont;

    skeleton.setCurrentGroup(u"Main"_s);

    skeleton.addItemPath(u"MyPath"_s, myPath);
    skeleton.addItemString(u"MyString"_s, myString, u"Hello"_s);
    skeleton.addItemInt(u"MyInt"_s, myInt, 42);
    skeleton.addItemPathList(u"MyPaths"_s, myPaths, {u"/foo/yo"_s});
    skeleton.addItemUrlList(u"MyUrls"_s, myUrls, {QUrl(u"https://kde.org"_s)});
    skeleton.addItemStringList(u"MyStrings"_s, myStrings, QStringList{u"a"_s, u"b"_s, u"c"_s});
    skeleton.addItemIntList(u"MyInts"_s, myInts, {13, 42});
    skeleton.addItemColor(u"MyColor"_s, myColor, QColor(3, 2, 1));
    skeleton.addItemFont(u"MyFont"_s, myFont, QFont(u"Helvetica"_s));

    QList<KCoreConfigSkeleton::ItemEnum::Choice> choices = {
        {
            .name = u"Hello"_s,
            .label = QString(),
            .toolTip = QString(),
            .whatsThis = QString(),
            .value = u"hello"_s,
        },
        {
            .name = u"Servus"_s,
            .label = QString(),
            .toolTip = QString(),
            .whatsThis = QString(),
            .value = u"servus"_s,
        },
    };
    auto itemEnum = new KCoreConfigSkeleton::ItemEnum(u"Main"_s, u"MyEnum"_s, myEnum, choices, 1);
    skeleton.addItem(itemEnum);

    // verify that the value is actually deleted
    QCOMPARE(myPath, QString());
    QCOMPARE(myString, u"Hello"_s);
    QCOMPARE(myInt, 42);
    QStringList expectedPaths = {u"/foo/yo"_s};
    QCOMPARE(myPaths, expectedPaths);
    QList<QUrl> expectedUrls = {QUrl(u"https://kde.org"_s)};
    QCOMPARE(myUrls, expectedUrls);
    QStringList expectedStrings = QStringList{u"a"_s, u"b"_s, u"c"_s};
    QCOMPARE(myStrings, expectedStrings);
    QList<int> expectedInts = {13, 42};
    QCOMPARE(myInts, expectedInts);
    QCOMPARE(myEnum, 1);
    QCOMPARE(myColor, QColor(3, 2, 1));
    QCOMPARE(myFont, QFont(u"Helvetica"_s));
}

#include "moc_kconfigskeletontest.cpp"
