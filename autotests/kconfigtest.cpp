/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Qt5 TODO: re-enable. No point in doing it before, it breaks on QString::fromUtf8(QByteArray), which exists in qt5.
#undef QT_NO_CAST_FROM_BYTEARRAY

#include "kconfigtest.h"
#include "helper.h"

#include <QtTest/QtTest>
#include <qtemporarydir.h>
#include <QStandardPaths>
#include <kdesktopfile.h>

#include <ksharedconfig.h>
#include <kconfiggroup.h>

#ifdef Q_OS_UNIX
#include <utime.h>
#endif
#ifndef Q_OS_WIN
#include <unistd.h> // gethostname
#endif

KCONFIGGROUP_DECLARE_ENUM_QOBJECT(KConfigTest, Testing)
KCONFIGGROUP_DECLARE_FLAGS_QOBJECT(KConfigTest, Flags)

QTEST_MAIN(KConfigTest)

#define BOOLENTRY1 true
#define BOOLENTRY2 false
#define STRINGENTRY1 "hello"
#define STRINGENTRY2 " hello"
#define STRINGENTRY3 "hello "
#define STRINGENTRY4 " hello "
#define STRINGENTRY5 " "
#define STRINGENTRY6 ""
#define UTF8BITENTRY "Hello äöü"
#define TRANSLATEDSTRINGENTRY1 "bonjour"
#define BYTEARRAYENTRY QByteArray( "\x00\xff\x7f\x3c abc\x00\x00", 10 )
#define ESCAPEKEY " []\0017[]==]"
#define ESCAPEENTRY "[]\170[]]=3=]\\] "
#define DOUBLEENTRY 123456.78912345
#define FLOATENTRY 123.567f
#define POINTENTRY QPoint( 4351, 1235 )
#define SIZEENTRY QSize( 10, 20 )
#define RECTENTRY QRect( 10, 23, 5321, 13 )
#define DATETIMEENTRY QDateTime( QDate( 2002, 06, 23 ), QTime( 12, 55, 40 ) )
#define STRINGLISTENTRY (QStringList( "Hello," ) << " World")
#define STRINGLISTEMPTYENTRY QStringList()
#define STRINGLISTJUSTEMPTYELEMENT QStringList(QString())
#define STRINGLISTEMPTYTRAINLINGELEMENT (QStringList( "Hi" ) << QString())
#define STRINGLISTESCAPEODDENTRY (QStringList( "Hello\\\\\\" ) << "World")
#define STRINGLISTESCAPEEVENENTRY (QStringList( "Hello\\\\\\\\" ) << "World")
#define STRINGLISTESCAPECOMMAENTRY (QStringList( "Hel\\\\\\,\\\\,\\,\\\\\\\\,lo" ) << "World")
#define INTLISTENTRY1 QList<int>() << 1 << 2 << 3 << 4
#define BYTEARRAYLISTENTRY1 QList<QByteArray>() << "" << "1,2" << "end"
#define VARIANTLISTENTRY (QVariantList() << true << false << QString("joe") << 10023)
#define VARIANTLISTENTRY2 (QVariantList() << POINTENTRY << SIZEENTRY)
#define HOMEPATH QString(QDir::homePath()+"/foo")
#define HOMEPATHESCAPE QString(QDir::homePath()+"/foo/$HOME")
#define DOLLARGROUP "$i"

#define TEST_SUBDIR "kconfigtest_subdir/"

static inline QString testConfigDir() {
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/" TEST_SUBDIR;
}

static inline QString kdeGlobalsPath() {
    return testConfigDir() + "../kdeglobals";
}

void KConfigTest::initTestCase()
{
    // ensure we don't use files in the real config directory
    QStandardPaths::setTestModeEnabled(true);

    // to make sure all files from a previous failed run are deleted
    cleanupTestCase();

    KSharedConfigPtr mainConfig = KSharedConfig::openConfig();
    mainConfig->group("Main").writeEntry("Key", "Value");
    mainConfig->sync();

    KConfig sc(TEST_SUBDIR "kconfigtest");

    KConfigGroup cg(&sc, "AAA");
    cg.writeEntry("stringEntry1", STRINGENTRY1,
                  KConfig::Persistent | KConfig::Global);
    cg.deleteEntry("stringEntry2", KConfig::Global);

    cg = KConfigGroup(&sc, "Hello");
    cg.writeEntry("boolEntry1", BOOLENTRY1);
    cg.writeEntry("boolEntry2", BOOLENTRY2);

    QByteArray data(UTF8BITENTRY);
    QCOMPARE(data.size(), 12);   // the source file is in utf8
    QCOMPARE(QString::fromUtf8(data).length(), 9);
    cg.writeEntry("Test", data);
    cg.writeEntry("bytearrayEntry", BYTEARRAYENTRY);
    cg.writeEntry(ESCAPEKEY, ESCAPEENTRY);
    cg.writeEntry("emptyEntry", "");
    cg.writeEntry("stringEntry1", STRINGENTRY1);
    cg.writeEntry("stringEntry2", STRINGENTRY2);
    cg.writeEntry("stringEntry3", STRINGENTRY3);
    cg.writeEntry("stringEntry4", STRINGENTRY4);
    cg.writeEntry("stringEntry5", STRINGENTRY5);
    cg.writeEntry("urlEntry1", QUrl("http://qt-project.org"));
    cg.writeEntry("keywith=equalsign", STRINGENTRY1);
    cg.deleteEntry("stringEntry5");
    cg.deleteEntry("stringEntry6");   // deleting a nonexistent entry
    cg.writeEntry("byteArrayEntry1", QByteArray(STRINGENTRY1),
                  KConfig::Global | KConfig::Persistent);
    cg.writeEntry("doubleEntry1", DOUBLEENTRY);
    cg.writeEntry("floatEntry1", FLOATENTRY);

    sc.deleteGroup("deleteMe"); // deleting a nonexistent group

    cg = KConfigGroup(&sc, "Complex Types");
    cg.writeEntry("rectEntry", RECTENTRY);
    cg.writeEntry("pointEntry", POINTENTRY);
    cg.writeEntry("sizeEntry", SIZEENTRY);
    cg.writeEntry("dateTimeEntry", DATETIMEENTRY);
    cg.writeEntry("dateEntry", DATETIMEENTRY.date());

    KConfigGroup ct = cg;
    cg = KConfigGroup(&ct, "Nested Group 1");
    cg.writeEntry("stringentry1", STRINGENTRY1);

    cg = KConfigGroup(&ct, "Nested Group 2");
    cg.writeEntry("stringEntry2", STRINGENTRY2);

    cg = KConfigGroup(&cg, "Nested Group 2.1");
    cg.writeEntry("stringEntry3", STRINGENTRY3);

    cg = KConfigGroup(&ct, "Nested Group 3");
    cg.writeEntry("stringEntry3", STRINGENTRY3);

    cg = KConfigGroup(&sc, "List Types");
    cg.writeEntry("listOfIntsEntry1", INTLISTENTRY1);
    cg.writeEntry("listOfByteArraysEntry1", BYTEARRAYLISTENTRY1);
    cg.writeEntry("stringListEntry", STRINGLISTENTRY);
    cg.writeEntry("stringListEmptyEntry", STRINGLISTEMPTYENTRY);
    cg.writeEntry("stringListJustEmptyElement", STRINGLISTJUSTEMPTYELEMENT);
    cg.writeEntry("stringListEmptyTrailingElement", STRINGLISTEMPTYTRAINLINGELEMENT);
    cg.writeEntry("stringListEscapeOddEntry", STRINGLISTESCAPEODDENTRY);
    cg.writeEntry("stringListEscapeEvenEntry", STRINGLISTESCAPEEVENENTRY);
    cg.writeEntry("stringListEscapeCommaEntry", STRINGLISTESCAPECOMMAENTRY);
    cg.writeEntry("variantListEntry", VARIANTLISTENTRY);

    cg = KConfigGroup(&sc, "Path Type");
    cg.writePathEntry("homepath", HOMEPATH);
    cg.writePathEntry("homepathescape", HOMEPATHESCAPE);

    cg = KConfigGroup(&sc, "Enum Types");
#if defined(_MSC_VER) && _MSC_VER == 1600
    cg.writeEntry("dummy", 42);
#else
    //Visual C++ 2010 throws an Internal Compiler Error here
    cg.writeEntry("enum-10", Tens);
    cg.writeEntry("enum-100", Hundreds);
    cg.writeEntry("flags-bit0", Flags(bit0));
    cg.writeEntry("flags-bit0-bit1", Flags(bit0 | bit1));
#endif

    cg = KConfigGroup(&sc, "ParentGroup");
    KConfigGroup cg1(&cg, "SubGroup1");
    cg1.writeEntry("somestring", "somevalue");
    cg.writeEntry("parentgrpstring", "somevalue");
    KConfigGroup cg2(&cg, "SubGroup2");
    cg2.writeEntry("substring", "somevalue");
    KConfigGroup cg3(&cg, "SubGroup/3");
    cg3.writeEntry("sub3string", "somevalue");

    QVERIFY(sc.isDirty());
    QVERIFY(sc.sync());
    QVERIFY(!sc.isDirty());

    QVERIFY2(QFile::exists(testConfigDir() + QStringLiteral("/kconfigtest")),
             qPrintable(testConfigDir() + QStringLiteral("/kconfigtest must exist")));
    QVERIFY2(QFile::exists(kdeGlobalsPath()),
             qPrintable(kdeGlobalsPath() + QStringLiteral(" must exist")));

    KConfig sc1(TEST_SUBDIR "kdebugrc", KConfig::SimpleConfig);
    KConfigGroup sg0(&sc1, "0");
    sg0.writeEntry("AbortFatal", false);
    sg0.writeEntry("WarnOutput", 0);
    sg0.writeEntry("FatalOutput", 0);
    QVERIFY(sc1.sync());

    //Setup stuff to test KConfig::addConfigSources()
    KConfig devcfg(TEST_SUBDIR "specificrc");
    KConfigGroup devonlygrp(&devcfg, "Specific Only Group");
    devonlygrp.writeEntry("ExistingEntry", "DevValue");
    KConfigGroup devandbasegrp(&devcfg, "Shared Group");
    devandbasegrp.writeEntry("SomeSharedEntry", "DevValue");
    devandbasegrp.writeEntry("SomeSpecificOnlyEntry", "DevValue");
    QVERIFY(devcfg.sync());
    KConfig basecfg(TEST_SUBDIR "baserc");
    KConfigGroup basegrp(&basecfg, "Base Only Group");
    basegrp.writeEntry("ExistingEntry", "BaseValue");
    KConfigGroup baseanddevgrp(&basecfg, "Shared Group");
    baseanddevgrp.writeEntry("SomeSharedEntry", "BaseValue");
    baseanddevgrp.writeEntry("SomeBaseOnlyEntry", "BaseValue");
    QVERIFY(basecfg.sync());

    KConfig gecfg(TEST_SUBDIR "groupescapetest", KConfig::SimpleConfig);
    cg = KConfigGroup(&gecfg, DOLLARGROUP);
    cg.writeEntry("entry", "doesntmatter");

}

void KConfigTest::cleanupTestCase()
{
    //ensure we don't delete the real directory
    QDir localConfig(testConfigDir());
    //qDebug() << "Erasing" << localConfig;
    if (localConfig.exists()) {
        QVERIFY(localConfig.removeRecursively());
    }
    QVERIFY(!localConfig.exists());
    if (QFile::exists(kdeGlobalsPath())) {
        QVERIFY(QFile::remove(kdeGlobalsPath()));
    }

}

static QList<QByteArray> readLinesFrom(const QString &path)
{
    QFile file(path);
    const bool opened = file.open(QIODevice::ReadOnly | QIODevice::Text);
    QList<QByteArray> lines;
    if (!opened) {
        QWARN(qPrintable(QLatin1String("Failed to open ") + path));
        return lines;
    }
    QByteArray line;
    do {
        line = file.readLine();
        if (!line.isEmpty()) {
            lines.append(line);
        }
    } while (!line.isEmpty());
    return lines;
}

static QList<QByteArray> readLines(const char *fileName = TEST_SUBDIR "kconfigtest")
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    Q_ASSERT(!path.isEmpty());
    return readLinesFrom(path + '/' + fileName);
}

// see also testDefaults, which tests reverting with a defaults (global) file available
void KConfigTest::testDirtyAfterRevert()
{
    KConfig sc(TEST_SUBDIR "kconfigtest_revert");

    KConfigGroup cg(&sc, "Hello");
    cg.revertToDefault("does_not_exist");
    QVERIFY(!sc.isDirty());
    cg.writeEntry("Test", "Correct");
    QVERIFY(sc.isDirty());
    sc.sync();
    QVERIFY(!sc.isDirty());

    cg.revertToDefault("Test");
    QVERIFY(sc.isDirty());
    QVERIFY(sc.sync());
    QVERIFY(!sc.isDirty());

    cg.revertToDefault("Test");
    QVERIFY(!sc.isDirty());
}

void KConfigTest::testRevertAllEntries()
{
    // this tests the case were we revert (delete) all entries in a file,
    // leaving a blank file
    {
        KConfig sc(TEST_SUBDIR "konfigtest2", KConfig::SimpleConfig);
        KConfigGroup cg(&sc, "Hello");
        cg.writeEntry("Test", "Correct");
    }

    {
        KConfig sc(TEST_SUBDIR "konfigtest2", KConfig::SimpleConfig);
        KConfigGroup cg(&sc, "Hello");
        QCOMPARE(cg.readEntry("Test", "Default"), QString("Correct"));
        cg.revertToDefault("Test");
    }

    KConfig sc(TEST_SUBDIR "konfigtest2", KConfig::SimpleConfig);
    KConfigGroup cg(&sc, "Hello");
    QCOMPARE(cg.readEntry("Test", "Default"), QString("Default"));
}

void KConfigTest::testSimple()
{
    // kdeglobals (which was created in initTestCase) must be found this way:
    const QStringList kdeglobals = QStandardPaths::locateAll(QStandardPaths::GenericConfigLocation, QLatin1String("kdeglobals"));
    QVERIFY(!kdeglobals.isEmpty());

    KConfig sc2(TEST_SUBDIR "kconfigtest");
    QCOMPARE(sc2.name(), QString(TEST_SUBDIR "kconfigtest"));

    // make sure groupList() isn't returning something it shouldn't
    Q_FOREACH (const QString &group, sc2.groupList()) {
        QVERIFY(!group.isEmpty() && group != "<default>");
        QVERIFY(!group.contains(QChar(0x1d)));
    }

    KConfigGroup sc3(&sc2, "AAA");

    QVERIFY(sc3.hasKey("stringEntry1"));     // from kdeglobals
    QVERIFY(!sc3.isEntryImmutable("stringEntry1"));
    QCOMPARE(sc3.readEntry("stringEntry1"), QString(STRINGENTRY1));

    QVERIFY(!sc3.hasKey("stringEntry2"));
    QCOMPARE(sc3.readEntry("stringEntry2", QString("bla")), QString("bla"));

    QVERIFY(!sc3.hasDefault("stringEntry1"));

    sc3 = KConfigGroup(&sc2, "Hello");
    QCOMPARE(sc3.readEntry("Test", QByteArray()), QByteArray(UTF8BITENTRY));
    QCOMPARE(sc3.readEntry("bytearrayEntry", QByteArray()), BYTEARRAYENTRY);
    QCOMPARE(sc3.readEntry(ESCAPEKEY), QString(ESCAPEENTRY));
    QCOMPARE(sc3.readEntry("Test", QString()), QString::fromUtf8(UTF8BITENTRY));
    QCOMPARE(sc3.readEntry("emptyEntry"/*, QString("Fietsbel")*/), QString(""));
    QCOMPARE(sc3.readEntry("emptyEntry", QString("Fietsbel")).isEmpty(), true);
    QCOMPARE(sc3.readEntry("stringEntry1"), QString(STRINGENTRY1));
    QCOMPARE(sc3.readEntry("stringEntry2"), QString(STRINGENTRY2));
    QCOMPARE(sc3.readEntry("stringEntry3"), QString(STRINGENTRY3));
    QCOMPARE(sc3.readEntry("stringEntry4"), QString(STRINGENTRY4));
    QVERIFY(!sc3.hasKey("stringEntry5"));
    QCOMPARE(sc3.readEntry("stringEntry5", QString("test")), QString("test"));
    QVERIFY(!sc3.hasKey("stringEntry6"));
    QCOMPARE(sc3.readEntry("stringEntry6", QString("foo")), QString("foo"));
    QCOMPARE(sc3.readEntry("urlEntry1", QUrl()), QUrl("http://qt-project.org"));
    QCOMPARE(sc3.readEntry("boolEntry1", BOOLENTRY1), BOOLENTRY1);
    QCOMPARE(sc3.readEntry("boolEntry2", false), BOOLENTRY2);
    QCOMPARE(sc3.readEntry("keywith=equalsign", QString("wrong")), QString(STRINGENTRY1));
    QCOMPARE(sc3.readEntry("byteArrayEntry1", QByteArray()),
             QByteArray(STRINGENTRY1));
    QCOMPARE(sc3.readEntry("doubleEntry1", 0.0), DOUBLEENTRY);
    QCOMPARE(sc3.readEntry("floatEntry1", 0.0f), FLOATENTRY);
}

void KConfigTest::testDefaults()
{
    KConfig config(TEST_SUBDIR "defaulttest", KConfig::NoGlobals);
    const QString defaultsFile = TEST_SUBDIR "defaulttest.defaults";
    KConfig defaults(defaultsFile, KConfig::SimpleConfig);

    const QString Default("Default");
    const QString NotDefault("Not Default");
    const QString Value1(STRINGENTRY1);
    const QString Value2(STRINGENTRY2);

    KConfigGroup group = defaults.group("any group");
    group.writeEntry("entry1", Default);
    QVERIFY(group.sync());

    group = config.group("any group");
    group.writeEntry("entry1", Value1);
    group.writeEntry("entry2", Value2);
    QVERIFY(group.sync());

    config.addConfigSources(QStringList() << QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + '/' + defaultsFile);

    config.setReadDefaults(true);
    QCOMPARE(group.readEntry("entry1", QString()), Default);
    QCOMPARE(group.readEntry("entry2", NotDefault), NotDefault); // no default for entry2

    config.setReadDefaults(false);
    QCOMPARE(group.readEntry("entry1", Default), Value1);
    QCOMPARE(group.readEntry("entry2", NotDefault), Value2);

    group.revertToDefault("entry1");
    QCOMPARE(group.readEntry("entry1", QString()), Default);
    group.revertToDefault("entry2");
    QCOMPARE(group.readEntry("entry2", QString()), QString());

    // TODO test reverting localized entries

    Q_ASSERT(config.isDirty());
    group.sync();

    // Check that everything is OK on disk, too
    KConfig reader(TEST_SUBDIR "defaulttest", KConfig::NoGlobals);
    reader.addConfigSources(QStringList() << QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + '/' + defaultsFile);
    KConfigGroup readerGroup = reader.group("any group");
    QCOMPARE(readerGroup.readEntry("entry1", QString()), Default);
    QCOMPARE(readerGroup.readEntry("entry2", QString()), QString());
}

void KConfigTest::testLocale()
{
    KConfig config(TEST_SUBDIR "kconfigtest.locales", KConfig::SimpleConfig);
    const QString Translated(TRANSLATEDSTRINGENTRY1);
    const QString Untranslated(STRINGENTRY1);

    KConfigGroup group = config.group("Hello");
    group.writeEntry("stringEntry1", Untranslated);
    config.setLocale("fr");
    group.writeEntry("stringEntry1", Translated, KConfig::Localized | KConfig::Persistent);
    QVERIFY(config.sync());

    QCOMPARE(group.readEntry("stringEntry1", QString()), Translated);
    QCOMPARE(group.readEntryUntranslated("stringEntry1"), Untranslated);

    config.setLocale("C"); // strings written in the "C" locale are written as nonlocalized
    group.writeEntry("stringEntry1", Untranslated, KConfig::Localized | KConfig::Persistent);
    QVERIFY(config.sync());

    QCOMPARE(group.readEntry("stringEntry1", QString()), Untranslated);
}

void KConfigTest::testEncoding()
{
    QString groupstr = QString::fromUtf8("UTF-8:\xc3\xb6l");

    KConfig c(TEST_SUBDIR "kconfigtestencodings");
    KConfigGroup cg(&c, groupstr);
    cg.writeEntry("key", "value");
    QVERIFY(c.sync());

    QList<QByteArray> lines = readLines(TEST_SUBDIR "kconfigtestencodings");
    QCOMPARE(lines.count(), 2);
    QCOMPARE(lines.first(), QByteArray("[UTF-8:\xc3\xb6l]\n"));

    KConfig c2(TEST_SUBDIR "kconfigtestencodings");
    KConfigGroup cg2(&c2, groupstr);
    QVERIFY(cg2.readEntry("key") == QByteArray("value"));

    QVERIFY(c2.groupList().contains(groupstr));
}

void KConfigTest::testLists()
{
    KConfig sc2(TEST_SUBDIR "kconfigtest");
    KConfigGroup sc3(&sc2, "List Types");

    QCOMPARE(sc3.readEntry(QString("stringListEntry"), QStringList()),
             STRINGLISTENTRY);

    QCOMPARE(sc3.readEntry(QString("stringListEmptyEntry"), QStringList("wrong")),
             STRINGLISTEMPTYENTRY);

    QCOMPARE(sc3.readEntry(QString("stringListJustEmptyElement"), QStringList()),
             STRINGLISTJUSTEMPTYELEMENT);

    QCOMPARE(sc3.readEntry(QString("stringListEmptyTrailingElement"), QStringList()),
             STRINGLISTEMPTYTRAINLINGELEMENT);

    QCOMPARE(sc3.readEntry(QString("stringListEscapeOddEntry"), QStringList()),
             STRINGLISTESCAPEODDENTRY);

    QCOMPARE(sc3.readEntry(QString("stringListEscapeEvenEntry"), QStringList()),
             STRINGLISTESCAPEEVENENTRY);

    QCOMPARE(sc3.readEntry(QString("stringListEscapeCommaEntry"), QStringList()),
             STRINGLISTESCAPECOMMAENTRY);

    QCOMPARE(sc3.readEntry("listOfIntsEntry1"), QString::fromLatin1("1,2,3,4"));
    QList<int> expectedIntList = INTLISTENTRY1;
    QVERIFY(sc3.readEntry("listOfIntsEntry1", QList<int>()) == expectedIntList);

    QCOMPARE(QVariant(sc3.readEntry("variantListEntry", VARIANTLISTENTRY)).toStringList(),
             QVariant(VARIANTLISTENTRY).toStringList());

    QCOMPARE(sc3.readEntry("listOfByteArraysEntry1", QList<QByteArray>()), BYTEARRAYLISTENTRY1);
}

void KConfigTest::testPath()
{
    KConfig sc2(TEST_SUBDIR "kconfigtest");
    KConfigGroup sc3(&sc2, "Path Type");
    QCOMPARE(sc3.readPathEntry("homepath", QString()), HOMEPATH);
    QCOMPARE(sc3.readPathEntry("homepathescape", QString()), HOMEPATHESCAPE);
    QCOMPARE(sc3.entryMap()["homepath"], HOMEPATH);

    {
        QFile file(testConfigDir() + "/pathtest");
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << "[Test Group]" << endl
            << "homePath=$HOME/foo" << endl
            << "homePath2=file://$HOME/foo" << endl
            << "withBraces[$e]=file://${HOME}/foo" << endl
            << "URL[$e]=file://${HOME}/foo" << endl
            << "hostname[$e]=$(hostname)" << endl
            << "noeol=foo"; // no EOL
    }
    KConfig cf2(TEST_SUBDIR "pathtest");
    KConfigGroup group = cf2.group("Test Group");
    QVERIFY(group.hasKey("homePath"));
    QCOMPARE(group.readPathEntry("homePath", QString()), HOMEPATH);
    QVERIFY(group.hasKey("homePath2"));
    QCOMPARE(group.readPathEntry("homePath2", QString()), QString("file://" + HOMEPATH));
    QVERIFY(group.hasKey("withBraces"));
    QCOMPARE(group.readPathEntry("withBraces", QString()), QString("file://" + HOMEPATH));
    QVERIFY(group.hasKey("URL"));
    QCOMPARE(group.readEntry("URL", QString()), QString("file://" + HOMEPATH));
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    // I don't know if this will work on windows
    // This test hangs on OS X
    QVERIFY(group.hasKey("hostname"));
    char hostname[256];
    QVERIFY(::gethostname(hostname, sizeof(hostname)) == 0);
    QCOMPARE(group.readEntry("hostname", QString()), QString::fromLatin1(hostname));
#endif
    QVERIFY(group.hasKey("noeol"));
    QCOMPARE(group.readEntry("noeol", QString()), QString("foo"));
}

void KConfigTest::testPersistenceOfExpandFlagForPath()
{
    // This test checks that a path entry starting with $HOME is still flagged
    // with the expand flag after the config was altered without rewriting the
    // path entry.

    // 1st step: Open the config, add a new dummy entry and then sync the config
    // back to the storage.
    {
        KConfig sc2(TEST_SUBDIR "kconfigtest");
        KConfigGroup sc3(&sc2, "Path Type");
        sc3.writeEntry("dummy", "dummy");
        QVERIFY(sc2.sync());
    }

    // 2nd step: Call testPath() again. Rewriting the config must not break
    // the testPath() test.
    testPath();
}

void KConfigTest::testPathQtHome()
{
    {
        QFile file(testConfigDir() + "/pathtest");
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << "[Test Group]" << endl
            << "dataDir[$e]=$QT_DATA_HOME/kconfigtest" << endl
            << "cacheDir[$e]=$QT_CACHE_HOME/kconfigtest" << endl
            << "configDir[$e]=$QT_CONFIG_HOME/kconfigtest" << endl;
    }
    KConfig cf2(TEST_SUBDIR "pathtest");
    KConfigGroup group = cf2.group("Test Group");
    qunsetenv("QT_DATA_HOME");
    qunsetenv("QT_CACHE_HOME");
    qunsetenv("QT_CONFIG_HOME");
    QVERIFY(group.hasKey("dataDir"));
    QCOMPARE(group.readEntry("dataDir", QString()), QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation).append(QStringLiteral("/kconfigtest")));
    QVERIFY(group.hasKey("cacheDir"));
    QCOMPARE(group.readEntry("cacheDir", QString()), QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation).append(QStringLiteral("/kconfigtest")));
    QVERIFY(group.hasKey("configDir"));
    QCOMPARE(group.readEntry("configDir", QString()), QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).append(QStringLiteral("/kconfigtest")));
    qputenv("QT_DATA_HOME","/1");
    qputenv("QT_CACHE_HOME","/2");
    qputenv("QT_CONFIG_HOME","/3");
    QVERIFY(group.hasKey("dataDir"));
    QCOMPARE(group.readEntry("dataDir", QString()), QStringLiteral("/1/kconfigtest"));
    QVERIFY(group.hasKey("cacheDir"));
    QCOMPARE(group.readEntry("cacheDir", QString()), QStringLiteral("/2/kconfigtest"));
    QVERIFY(group.hasKey("configDir"));
    QCOMPARE(group.readEntry("configDir", QString()), QStringLiteral("/3/kconfigtest"));
}

void KConfigTest::testComplex()
{
    KConfig sc2(TEST_SUBDIR "kconfigtest");
    KConfigGroup sc3(&sc2, "Complex Types");

    QCOMPARE(sc3.readEntry("pointEntry", QPoint()), POINTENTRY);
    QCOMPARE(sc3.readEntry("sizeEntry", SIZEENTRY), SIZEENTRY);
    QCOMPARE(sc3.readEntry("rectEntry", QRect(1, 2, 3, 4)), RECTENTRY);
    QCOMPARE(sc3.readEntry("dateTimeEntry", QDateTime()).toString(Qt::ISODate),
             DATETIMEENTRY.toString(Qt::ISODate));
    QCOMPARE(sc3.readEntry("dateEntry", QDate()).toString(Qt::ISODate),
             DATETIMEENTRY.date().toString(Qt::ISODate));
    QCOMPARE(sc3.readEntry("dateTimeEntry", QDate()), DATETIMEENTRY.date());
}

void KConfigTest::testEnums()
{
    //Visual C++ 2010 (compiler version 16.0) throws an Internal Compiler Error
    //when compiling the code in initTestCase that creates these KConfig entries,
    //so we can't run this test
#if defined(_MSC_VER) && _MSC_VER == 1600
    QSKIP("Visual C++ 2010 can't compile this test");
#endif
    KConfig sc(TEST_SUBDIR "kconfigtest");
    KConfigGroup sc3(&sc, "Enum Types");

    QCOMPARE(sc3.readEntry("enum-10"), QString("Tens"));
    QVERIFY(sc3.readEntry("enum-100", Ones) != Ones);
    QVERIFY(sc3.readEntry("enum-100", Ones) != Tens);

    QCOMPARE(sc3.readEntry("flags-bit0"), QString("bit0"));
    QVERIFY(sc3.readEntry("flags-bit0", Flags()) == bit0);

    int eid = staticMetaObject.indexOfEnumerator("Flags");
    QVERIFY(eid != -1);
    QMetaEnum me = staticMetaObject.enumerator(eid);
    Flags bitfield = bit0 | bit1;

    QCOMPARE(sc3.readEntry("flags-bit0-bit1"), QString(me.valueToKeys(bitfield)));
    QVERIFY(sc3.readEntry("flags-bit0-bit1", Flags()) == bitfield);
}

void KConfigTest::testEntryMap()
{
    KConfig sc(TEST_SUBDIR "kconfigtest");
    KConfigGroup cg(&sc, "Hello");
    QMap<QString, QString> entryMap = cg.entryMap();
    qDebug() << entryMap.keys();
    QCOMPARE(entryMap.value("stringEntry1"), QString(STRINGENTRY1));
    QCOMPARE(entryMap.value("stringEntry2"), QString(STRINGENTRY2));
    QCOMPARE(entryMap.value("stringEntry3"), QString(STRINGENTRY3));
    QCOMPARE(entryMap.value("stringEntry4"), QString(STRINGENTRY4));
    QVERIFY(!entryMap.contains("stringEntry5"));
    QVERIFY(!entryMap.contains("stringEntry6"));
    QCOMPARE(entryMap.value("Test"), QString::fromUtf8(UTF8BITENTRY));
    QCOMPARE(entryMap.value("bytearrayEntry"), QString::fromUtf8(BYTEARRAYENTRY.constData()));
    QCOMPARE(entryMap.value("emptyEntry"), QString());
    QVERIFY(entryMap.contains("emptyEntry"));
    QCOMPARE(entryMap.value("boolEntry1"), QString(BOOLENTRY1 ? "true" : "false"));
    QCOMPARE(entryMap.value("boolEntry2"), QString(BOOLENTRY2 ? "true" : "false"));
    QCOMPARE(entryMap.value("keywith=equalsign"), QString(STRINGENTRY1));
    QCOMPARE(entryMap.value("byteArrayEntry1"), QString(STRINGENTRY1));
    QCOMPARE(entryMap.value("doubleEntry1").toDouble(), DOUBLEENTRY);
    QCOMPARE(entryMap.value("floatEntry1").toFloat(), FLOATENTRY);
}

void KConfigTest::testInvalid()
{
    KConfig sc(TEST_SUBDIR "kconfigtest");

    // all of these should print a message to the kdebug.dbg file
    KConfigGroup sc3(&sc, "Invalid Types");
    sc3.writeEntry("badList", VARIANTLISTENTRY2);

    QList<int> list;

    // 1 element list
    list << 1;
    sc3.writeEntry(QString("badList"), list);

    QVERIFY(sc3.readEntry("badList", QPoint()) == QPoint());
    QVERIFY(sc3.readEntry("badList", QRect()) == QRect());
    QVERIFY(sc3.readEntry("badList", QSize()) == QSize());
    QVERIFY(sc3.readEntry("badList", QDate()) == QDate());
    QVERIFY(sc3.readEntry("badList", QDateTime()) == QDateTime());

    // 2 element list
    list << 2;
    sc3.writeEntry("badList", list);

    QVERIFY(sc3.readEntry("badList", QRect()) == QRect());
    QVERIFY(sc3.readEntry("badList", QDate()) == QDate());
    QVERIFY(sc3.readEntry("badList", QDateTime()) == QDateTime());

    // 3 element list
    list << 303;
    sc3.writeEntry("badList", list);

    QVERIFY(sc3.readEntry("badList", QPoint()) == QPoint());
    QVERIFY(sc3.readEntry("badList", QRect()) == QRect());
    QVERIFY(sc3.readEntry("badList", QSize()) == QSize());
    QVERIFY(sc3.readEntry("badList", QDate()) == QDate());     // out of bounds
    QVERIFY(sc3.readEntry("badList", QDateTime()) == QDateTime());

    // 4 element list
    list << 4;
    sc3.writeEntry("badList", list);

    QVERIFY(sc3.readEntry("badList", QPoint()) == QPoint());
    QVERIFY(sc3.readEntry("badList", QSize()) == QSize());
    QVERIFY(sc3.readEntry("badList", QDate()) == QDate());
    QVERIFY(sc3.readEntry("badList", QDateTime()) == QDateTime());

    // 5 element list
    list[2] = 3;
    list << 5;
    sc3.writeEntry("badList", list);

    QVERIFY(sc3.readEntry("badList", QPoint()) == QPoint());
    QVERIFY(sc3.readEntry("badList", QRect()) == QRect());
    QVERIFY(sc3.readEntry("badList", QSize()) == QSize());
    QVERIFY(sc3.readEntry("badList", QDate()) == QDate());
    QVERIFY(sc3.readEntry("badList", QDateTime()) == QDateTime());

    // 6 element list
    list << 6;
    sc3.writeEntry("badList", list);

    QVERIFY(sc3.readEntry("badList", QPoint()) == QPoint());
    QVERIFY(sc3.readEntry("badList", QRect()) == QRect());
    QVERIFY(sc3.readEntry("badList", QSize()) == QSize());
}

void KConfigTest::testChangeGroup()
{
    KConfig sc(TEST_SUBDIR "kconfigtest");
    KConfigGroup sc3(&sc, "Hello");
    QCOMPARE(sc3.name(), QString("Hello"));
    KConfigGroup newGroup(sc3);
#ifndef KDE_NO_DEPRECATED
    newGroup.changeGroup("FooBar"); // deprecated!
    QCOMPARE(newGroup.name(), QString("FooBar"));
    QCOMPARE(sc3.name(), QString("Hello")); // unchanged

    // Write into the "changed group" and check that it works
    newGroup.writeEntry("InFooBar", "FB");
    QCOMPARE(KConfigGroup(&sc, "FooBar").entryMap().value("InFooBar"), QString("FB"));
    QCOMPARE(KConfigGroup(&sc, "Hello").entryMap().value("InFooBar"), QString());
#endif

    KConfigGroup rootGroup(sc.group(""));
    QCOMPARE(rootGroup.name(), QString("<default>"));
    KConfigGroup sc32(rootGroup.group("Hello"));
    QCOMPARE(sc32.name(), QString("Hello"));
    KConfigGroup newGroup2(sc32);
#ifndef KDE_NO_DEPRECATED
    newGroup2.changeGroup("FooBar"); // deprecated!
    QCOMPARE(newGroup2.name(), QString("FooBar"));
    QCOMPARE(sc32.name(), QString("Hello")); // unchanged
#endif
}

// Simple test for deleteEntry
void KConfigTest::testDeleteEntry()
{
    const char *configFile = TEST_SUBDIR "kconfigdeletetest";
    {
        KConfig conf(configFile);
        conf.group("Hello").writeEntry("DelKey", "ToBeDeleted");
    }
    const QList<QByteArray> lines = readLines(configFile);
    Q_ASSERT(lines.contains("[Hello]\n"));
    Q_ASSERT(lines.contains("DelKey=ToBeDeleted\n"));

    KConfig sc(configFile);
    KConfigGroup group(&sc, "Hello");

    group.deleteEntry("DelKey");
    QCOMPARE(group.readEntry("DelKey", QString("Fietsbel")), QString("Fietsbel"));

    QVERIFY(group.sync());
    Q_ASSERT(!readLines(configFile).contains("DelKey=ToBeDeleted\n"));
    QCOMPARE(group.readEntry("DelKey", QString("still deleted")), QString("still deleted"));
}

void KConfigTest::testDelete()
{
    KConfig sc(TEST_SUBDIR "kconfigtest");

    KConfigGroup ct(&sc, "Complex Types");

    // First delete a nested group
    KConfigGroup delgr(&ct, "Nested Group 3");
    QVERIFY(delgr.exists());
    QVERIFY(ct.hasGroup("Nested Group 3"));
    delgr.deleteGroup();
    QVERIFY(!delgr.exists());
    QVERIFY(!ct.hasGroup("Nested Group 3"));
    QVERIFY(ct.groupList().contains("Nested Group 3"));

    KConfigGroup ng(&ct, "Nested Group 2");
    QVERIFY(sc.hasGroup("Complex Types"));
    QVERIFY(!sc.hasGroup("Does not exist"));
    sc.deleteGroup("Complex Types");
    QCOMPARE(sc.group("Complex Types").keyList().count(), 0);
    QVERIFY(!sc.hasGroup("Complex Types")); // #192266
    QVERIFY(!sc.group("Complex Types").exists());
    QVERIFY(!ct.hasGroup("Nested Group 1"));

    QCOMPARE(ct.group("Nested Group 1").keyList().count(), 0);
    QCOMPARE(ct.group("Nested Group 2").keyList().count(), 0);
    QCOMPARE(ng.group("Nested Group 2.1").keyList().count(), 0);

    KConfigGroup cg(&sc, "AAA");
    cg.deleteGroup();
    QVERIFY(sc.entryMap("Complex Types").isEmpty());
    QVERIFY(sc.entryMap("AAA").isEmpty());
    QVERIFY(!sc.entryMap("Hello").isEmpty());   //not deleted group
    QVERIFY(sc.entryMap("FooBar").isEmpty());   //inexistant group

    QVERIFY(cg.sync());
    // Check what happens on disk
    const QList<QByteArray> lines = readLines();
    //qDebug() << lines;
    QVERIFY(!lines.contains("[Complex Types]\n"));
    QVERIFY(!lines.contains("[Complex Types][Nested Group 1]\n"));
    QVERIFY(!lines.contains("[Complex Types][Nested Group 2]\n"));
    QVERIFY(!lines.contains("[Complex Types][Nested Group 2.1]\n"));
    QVERIFY(!lines.contains("[AAA]\n"));
    QVERIFY(lines.contains("[Hello]\n")); // a group that was not deleted

    // test for entries that are marked as deleted when there is no default
    KConfig cf(TEST_SUBDIR "kconfigtest", KConfig::SimpleConfig); // make sure there are no defaults
    cg = cf.group("Portable Devices");
    cg.writeEntry("devices|manual|(null)", "whatever");
    cg.writeEntry("devices|manual|/mnt/ipod", "/mnt/ipod");
    QVERIFY(cf.sync());

    int count = 0;
    Q_FOREACH (const QByteArray &item, readLines())
        if (item.startsWith("devices|")) { // krazy:exclude=strings
            count++;
        }
    QCOMPARE(count, 2);
    cg.deleteEntry("devices|manual|/mnt/ipod");
    QVERIFY(cf.sync());
    Q_FOREACH (const QByteArray &item, readLines()) {
        QVERIFY(!item.contains("ipod"));
    }
}

void KConfigTest::testDefaultGroup()
{
    KConfig sc(TEST_SUBDIR "kconfigtest");
    KConfigGroup defaultGroup(&sc, "<default>");
    QCOMPARE(defaultGroup.name(), QString("<default>"));
    QVERIFY(!defaultGroup.exists());
    defaultGroup.writeEntry("TestKey", "defaultGroup");
    QVERIFY(defaultGroup.exists());
    QCOMPARE(defaultGroup.readEntry("TestKey", QString()), QString("defaultGroup"));
    QVERIFY(sc.sync());

    {
        // Test reading it
        KConfig sc2(TEST_SUBDIR "kconfigtest");
        KConfigGroup defaultGroup2(&sc2, "<default>");
        QCOMPARE(defaultGroup2.name(), QString("<default>"));
        QVERIFY(defaultGroup2.exists());
        QCOMPARE(defaultGroup2.readEntry("TestKey", QString()), QString("defaultGroup"));
    }
    {
        // Test reading it
        KConfig sc2(TEST_SUBDIR "kconfigtest");
        KConfigGroup emptyGroup(&sc2, "");
        QCOMPARE(emptyGroup.name(), QString("<default>"));
        QVERIFY(emptyGroup.exists());
        QCOMPARE(emptyGroup.readEntry("TestKey", QString()), QString("defaultGroup"));
    }

    QList<QByteArray> lines = readLines();
    QVERIFY(!lines.contains("[]\n"));
    QCOMPARE(lines.first(), QByteArray("TestKey=defaultGroup\n"));

    // Now that the group exists make sure it isn't returned from groupList()
    Q_FOREACH (const QString &group, sc.groupList()) {
        QVERIFY(!group.isEmpty() && group != "<default>");
    }

    defaultGroup.deleteGroup();
    QVERIFY(sc.sync());

    // Test if deleteGroup worked
    lines = readLines();
    QVERIFY(lines.first() != QByteArray("TestKey=defaultGroup\n"));
}

void KConfigTest::testEmptyGroup()
{
    KConfig sc(TEST_SUBDIR "kconfigtest");
    KConfigGroup emptyGroup(&sc, "");
    QCOMPARE(emptyGroup.name(), QString("<default>")); // confusing, heh?
    QVERIFY(!emptyGroup.exists());
    emptyGroup.writeEntry("TestKey", "emptyGroup");
    QVERIFY(emptyGroup.exists());
    QCOMPARE(emptyGroup.readEntry("TestKey", QString()), QString("emptyGroup"));
    QVERIFY(sc.sync());

    {
        // Test reading it
        KConfig sc2(TEST_SUBDIR "kconfigtest");
        KConfigGroup defaultGroup(&sc2, "<default>");
        QCOMPARE(defaultGroup.name(), QString("<default>"));
        QVERIFY(defaultGroup.exists());
        QCOMPARE(defaultGroup.readEntry("TestKey", QString()), QString("emptyGroup"));
    }
    {
        // Test reading it
        KConfig sc2(TEST_SUBDIR "kconfigtest");
        KConfigGroup emptyGroup2(&sc2, "");
        QCOMPARE(emptyGroup2.name(), QString("<default>"));
        QVERIFY(emptyGroup2.exists());
        QCOMPARE(emptyGroup2.readEntry("TestKey", QString()), QString("emptyGroup"));
    }

    QList<QByteArray> lines = readLines();
    QVERIFY(!lines.contains("[]\n")); // there's no support for the [] group, in fact.
    QCOMPARE(lines.first(), QByteArray("TestKey=emptyGroup\n"));

    // Now that the group exists make sure it isn't returned from groupList()
    Q_FOREACH (const QString &group, sc.groupList()) {
        QVERIFY(!group.isEmpty() && group != "<default>");
    }
    emptyGroup.deleteGroup();
    QVERIFY(sc.sync());

    // Test if deleteGroup worked
    lines = readLines();
    QVERIFY(lines.first() != QByteArray("TestKey=defaultGroup\n"));
}

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC) && !defined(Q_OS_BLACKBERRY) && !defined(Q_OS_ANDROID)
#define Q_XDG_PLATFORM
#endif

void KConfigTest::testCascadingWithLocale()
{
    // This test relies on XDG_CONFIG_DIRS, which only has effect on Unix.
    // Cascading (more than two levels) isn't available at all on Windows.
#ifdef Q_XDG_PLATFORM
    QTemporaryDir middleDir;
    QTemporaryDir globalDir;
    qputenv("XDG_CONFIG_DIRS", qPrintable(middleDir.path() + QString(":") + globalDir.path()));

    const QString globalConfigDir = globalDir.path() + "/" TEST_SUBDIR;
    QVERIFY(QDir().mkpath(globalConfigDir));
    QFile global(globalConfigDir + "foo.desktop");
    QVERIFY(global.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream globalOut(&global);
    globalOut << "[Group]" << endl
              << "FromGlobal=true" << endl
              << "FromGlobal[fr]=vrai" << endl
              << "Name=Testing" << endl
              << "Name[fr]=FR" << endl
              << "Other=Global" << endl
              << "Other[fr]=Global_FR" << endl;
    global.close();

    const QString middleConfigDir = middleDir.path() + "/" TEST_SUBDIR;
    QVERIFY(QDir().mkpath(middleConfigDir));
    QFile local(middleConfigDir + "foo.desktop");
    QVERIFY(local.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&local);
    out << "[Group]" << endl
        << "FromLocal=true" << endl
        << "FromLocal[fr]=vrai" << endl
        << "Name=Local Testing" << endl
        << "Name[fr]=FR" << endl
        << "Other=English Only" << endl;
    local.close();

    KConfig config(TEST_SUBDIR "foo.desktop");
    KConfigGroup group = config.group("Group");
    QCOMPARE(group.readEntry("FromGlobal"), QString("true"));
    QCOMPARE(group.readEntry("FromLocal"), QString("true"));
    QCOMPARE(group.readEntry("Name"), QString("Local Testing"));
    config.setLocale("fr");
    QCOMPARE(group.readEntry("FromGlobal"), QString("vrai"));
    QCOMPARE(group.readEntry("FromLocal"), QString("vrai"));
    QCOMPARE(group.readEntry("Name"), QString("FR"));
    QCOMPARE(group.readEntry("Other"), QString("English Only")); // Global_FR is locally overriden
#endif
}

void KConfigTest::testMerge()
{
    DefaultLocale defaultLocale;
    QLocale::setDefault(QLocale::c());
    KConfig config(TEST_SUBDIR "mergetest", KConfig::SimpleConfig);

    KConfigGroup cg = config.group("some group");
    cg.writeEntry("entry", " random entry");
    cg.writeEntry("another entry", "blah blah blah");

    {
        // simulate writing by another process
        QFile file(testConfigDir() + "/mergetest");
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << "[Merged Group]" << endl
            << "entry1=Testing" << endl
            << "entry2=More Testing" << endl
            << "[some group]" << endl
            << "entry[fr]=French" << endl
            << "entry[es]=Spanish" << endl
            << "entry[de]=German" << endl;
    }
    QVERIFY(config.sync());

    {
        QList<QByteArray> lines;
        // this is what the file should look like
        lines << "[Merged Group]\n"
              << "entry1=Testing\n"
              << "entry2=More Testing\n"
              << "\n"
              << "[some group]\n"
              << "another entry=blah blah blah\n"
              << "entry=\\srandom entry\n"
              << "entry[de]=German\n"
              << "entry[es]=Spanish\n"
              << "entry[fr]=French\n";
        QFile file(testConfigDir() + "/mergetest");
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        Q_FOREACH (const QByteArray &line, lines) {
            QCOMPARE(line, file.readLine());
        }
    }
}

void KConfigTest::testImmutable()
{
    {
        QFile file(testConfigDir() + "/immutabletest");
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << "[$i]" << endl
            << "entry1=Testing" << endl
            << "[group][$i]" << endl
            << "[group][subgroup][$i]" << endl;
    }

    KConfig config(TEST_SUBDIR "immutabletest", KConfig::SimpleConfig);
    QVERIFY(config.isGroupImmutable(QByteArray()));
    KConfigGroup cg = config.group(QByteArray());
    QVERIFY(cg.isEntryImmutable("entry1"));
    KConfigGroup cg1 = config.group("group");
    QVERIFY(cg1.isImmutable());
    KConfigGroup cg1a = cg.group("group");
    QVERIFY(cg1a.isImmutable());
    KConfigGroup cg2 = cg1.group("subgroup");
    QVERIFY(cg2.isImmutable());
}

void KConfigTest::testOptionOrder()
{
    {
        QFile file(testConfigDir() + "/doubleattrtest");
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << "[group3]" << endl
            << "entry2=unlocalized" << endl
            << "entry2[$i][de_DE]=t2" << endl;
    }
    KConfig config(TEST_SUBDIR "doubleattrtest", KConfig::SimpleConfig);
    config.setLocale("de_DE");
    KConfigGroup cg3 = config.group("group3");
    QVERIFY(!cg3.isImmutable());
    QCOMPARE(cg3.readEntry("entry2", ""), QString("t2"));
    QVERIFY(cg3.isEntryImmutable("entry2"));
    config.setLocale("C");
    QCOMPARE(cg3.readEntry("entry2", ""), QString("unlocalized"));
    QVERIFY(!cg3.isEntryImmutable("entry2"));
    cg3.writeEntry("entry2", "modified");
    QVERIFY(config.sync());

    {
        QList<QByteArray> lines;
        // this is what the file should look like
        lines << "[group3]\n"
              << "entry2=modified\n"
              << "entry2[de_DE][$i]=t2\n";

        QFile file(testConfigDir() + "/doubleattrtest");
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        Q_FOREACH (const QByteArray &line, lines) {
            QCOMPARE(line, file.readLine());
        }
    }
}

void KConfigTest::testGroupEscape()
{
    KConfig config(TEST_SUBDIR "groupescapetest", KConfig::SimpleConfig);
    QVERIFY(config.group(DOLLARGROUP).exists());
}

void KConfigTest::testSubGroup()
{
    KConfig sc(TEST_SUBDIR "kconfigtest");
    KConfigGroup cg(&sc, "ParentGroup");
    QCOMPARE(cg.readEntry("parentgrpstring", ""), QString("somevalue"));
    KConfigGroup subcg1(&cg, "SubGroup1");
    QCOMPARE(subcg1.name(), QString("SubGroup1"));
    QCOMPARE(subcg1.readEntry("somestring", ""), QString("somevalue"));
    KConfigGroup subcg2(&cg, "SubGroup2");
    QCOMPARE(subcg2.name(), QString("SubGroup2"));
    QCOMPARE(subcg2.readEntry("substring", ""), QString("somevalue"));
    KConfigGroup subcg3(&cg, "SubGroup/3");
    QCOMPARE(subcg3.readEntry("sub3string", ""), QString("somevalue"));
    QCOMPARE(subcg3.name(), QString("SubGroup/3"));
    KConfigGroup rcg(&sc, "");
    KConfigGroup srcg(&rcg, "ParentGroup");
    QCOMPARE(srcg.readEntry("parentgrpstring", ""), QString("somevalue"));

    QStringList groupList = cg.groupList();
    groupList.sort(); // comes from QSet, so order is undefined
    QCOMPARE(groupList, (QStringList() << "SubGroup/3" << "SubGroup1" << "SubGroup2"));

    const QStringList expectedSubgroup3Keys = (QStringList() << "sub3string");
    QCOMPARE(subcg3.keyList(), expectedSubgroup3Keys);
    const QStringList expectedParentGroupKeys(QStringList() << "parentgrpstring");

    QCOMPARE(cg.keyList(), expectedParentGroupKeys);

    QCOMPARE(QStringList(cg.entryMap().keys()), expectedParentGroupKeys);
    QCOMPARE(QStringList(subcg3.entryMap().keys()), expectedSubgroup3Keys);

    // Create A group containing only other groups. We want to make sure it
    // shows up in groupList of sc
    KConfigGroup neg(&sc, "NoEntryGroup");
    KConfigGroup negsub1(&neg, "NEG Child1");
    negsub1.writeEntry("entry", "somevalue");
    KConfigGroup negsub2(&neg, "NEG Child2");
    KConfigGroup negsub3(&neg, "NEG Child3");
    KConfigGroup negsub31(&negsub3, "NEG Child3-1");
    KConfigGroup negsub4(&neg, "NEG Child4");
    KConfigGroup negsub41(&negsub4, "NEG Child4-1");
    negsub41.writeEntry("entry", "somevalue");

    // A group exists if it has content
    QVERIFY(negsub1.exists());

    // But it doesn't exist if it has no content
    // Ossi and David say: this is how it's supposed to work.
    // However you could add a dummy entry for now, or we could add a "Persist" feature to kconfig groups
    // which would make it written out, much like "immutable" already makes them persistent.
    QVERIFY(!negsub2.exists());

    // A subgroup does not qualify as content if it is also empty
    QVERIFY(!negsub3.exists());

    // A subgroup with content is ok
    QVERIFY(negsub4.exists());

    // Only subgroups with content show up in groupList()
    //QEXPECT_FAIL("", "Empty subgroups do not show up in groupList()", Continue);
    //QCOMPARE(neg.groupList(), QStringList() << "NEG Child1" << "NEG Child2" << "NEG Child3" << "NEG Child4");
    // This is what happens
    QStringList groups = neg.groupList();
    groups.sort(); // Qt5 made the ordering unreliable, due to QHash
    QCOMPARE(groups, QStringList() << "NEG Child1" << "NEG Child4");

    // make sure groupList() isn't returning something it shouldn't
    Q_FOREACH (const QString &group, sc.groupList()) {
        QVERIFY(!group.isEmpty() && group != "<default>");
        QVERIFY(!group.contains(QChar(0x1d)));
        QVERIFY(!group.contains("subgroup"));
        QVERIFY(!group.contains("SubGroup"));
    }

    QVERIFY(sc.sync());

    // Check that the empty groups are not written out.
    const QList<QByteArray> lines = readLines();
    QVERIFY(lines.contains("[NoEntryGroup][NEG Child1]\n"));
    QVERIFY(!lines.contains("[NoEntryGroup][NEG Child2]\n"));
    QVERIFY(!lines.contains("[NoEntryGroup][NEG Child3]\n"));
    QVERIFY(!lines.contains("[NoEntryGroup][NEG Child4]\n")); // implicit group, not written out
    QVERIFY(lines.contains("[NoEntryGroup][NEG Child4][NEG Child4-1]\n"));
}

void KConfigTest::testAddConfigSources()
{
    KConfig cf(TEST_SUBDIR "specificrc");

    cf.addConfigSources(QStringList() << testConfigDir() + "/baserc");
    cf.reparseConfiguration();

    KConfigGroup specificgrp(&cf, "Specific Only Group");
    QCOMPARE(specificgrp.readEntry("ExistingEntry", ""), QString("DevValue"));

    KConfigGroup sharedgrp(&cf, "Shared Group");
    QCOMPARE(sharedgrp.readEntry("SomeSpecificOnlyEntry", ""), QString("DevValue"));
    QCOMPARE(sharedgrp.readEntry("SomeBaseOnlyEntry", ""), QString("BaseValue"));
    QCOMPARE(sharedgrp.readEntry("SomeSharedEntry", ""), QString("DevValue"));

    KConfigGroup basegrp(&cf, "Base Only Group");
    QCOMPARE(basegrp.readEntry("ExistingEntry", ""), QString("BaseValue"));
    basegrp.writeEntry("New Entry Base Only", "SomeValue");

    KConfigGroup newgrp(&cf, "New Group");
    newgrp.writeEntry("New Entry", "SomeValue");

    QVERIFY(cf.sync());

    KConfig plaincfg(TEST_SUBDIR "specificrc");

    KConfigGroup newgrp2(&plaincfg, "New Group");
    QCOMPARE(newgrp2.readEntry("New Entry", ""), QString("SomeValue"));

    KConfigGroup basegrp2(&plaincfg, "Base Only Group");
    QCOMPARE(basegrp2.readEntry("New Entry Base Only", ""), QString("SomeValue"));
}

void KConfigTest::testGroupCopyTo()
{
    KConfig cf1(TEST_SUBDIR "kconfigtest");
    KConfigGroup original = cf1.group("Enum Types");

    KConfigGroup copy = cf1.group("Enum Types Copy");
    original.copyTo(&copy); // copy from one group to another
    QCOMPARE(copy.entryMap(), original.entryMap());

    KConfig cf2(TEST_SUBDIR "copy_of_kconfigtest", KConfig::SimpleConfig);
    QVERIFY(!cf2.hasGroup(original.name()));
    QVERIFY(!cf2.hasGroup(copy.name()));

    KConfigGroup newGroup = cf2.group(original.name());
    original.copyTo(&newGroup); // copy from one file to another
    QVERIFY(cf2.hasGroup(original.name()));
    QVERIFY(!cf2.hasGroup(copy.name())); // make sure we didn't copy more than we wanted
    QCOMPARE(newGroup.entryMap(), original.entryMap());
}

void KConfigTest::testConfigCopyToSync()
{
    KConfig cf1(TEST_SUBDIR "kconfigtest");
    // Prepare source file
    KConfigGroup group(&cf1, "CopyToTest");
    group.writeEntry("Type", "Test");
    QVERIFY(cf1.sync());

    // Copy to "destination"
    const QString destination = testConfigDir() + "/kconfigcopytotest";
    QFile::remove(destination);

    KConfig cf2(TEST_SUBDIR "kconfigcopytotest");
    KConfigGroup group2(&cf2, "CopyToTest");

    group.copyTo(&group2);

    QString testVal = group2.readEntry("Type");
    QCOMPARE(testVal, QString("Test"));
    // should write to disk the copied data from group
    QVERIFY(cf2.sync());
    QVERIFY(QFile::exists(destination));
}

void KConfigTest::testConfigCopyTo()
{
    KConfig cf1(TEST_SUBDIR "kconfigtest");
    {
        // Prepare source file
        KConfigGroup group(&cf1, "CopyToTest");
        group.writeEntry("Type", "Test");
        QVERIFY(cf1.sync());
    }

    {
        // Copy to "destination"
        const QString destination = testConfigDir() + "/kconfigcopytotest";
        QFile::remove(destination);
        KConfig cf2;
        cf1.copyTo(destination, &cf2);
        KConfigGroup group2(&cf2, "CopyToTest");
        QString testVal = group2.readEntry("Type");
        QCOMPARE(testVal, QString("Test"));
        QVERIFY(cf2.sync());
        QVERIFY(QFile::exists(destination));
    }

    // Check copied config file on disk
    KConfig cf3(TEST_SUBDIR "kconfigcopytotest");
    KConfigGroup group3(&cf3, "CopyToTest");
    QString testVal = group3.readEntry("Type");
    QCOMPARE(testVal, QString("Test"));
}

void KConfigTest::testReparent()
{
    KConfig cf(TEST_SUBDIR "kconfigtest");
    const QString name("Enum Types");
    KConfigGroup group = cf.group(name);
    const QMap<QString, QString> originalMap = group.entryMap();
    KConfigGroup parent = cf.group("Parent Group");

    QVERIFY(!parent.hasGroup(name));

    QVERIFY(group.entryMap() == originalMap);

    group.reparent(&parent); // see if it can be made a sub-group of another group
    QVERIFY(parent.hasGroup(name));
    QCOMPARE(group.entryMap(), originalMap);

    group.reparent(&cf); // see if it can make it a top-level group again
//    QVERIFY(!parent.hasGroup(name));
    QCOMPARE(group.entryMap(), originalMap);
}

static void ageTimeStamp(const QString &path, int nsec)
{
#ifdef Q_OS_UNIX
    QDateTime mtime = QFileInfo(path).lastModified().addSecs(-nsec);
    struct utimbuf utbuf;
    utbuf.actime = mtime.toTime_t();
    utbuf.modtime = utbuf.actime;
    utime(QFile::encodeName(path), &utbuf);
#else
    QTest::qSleep(nsec * 1000);
#endif
}

void KConfigTest::testWriteOnSync()
{
    QDateTime oldStamp, newStamp;
    KConfig sc(TEST_SUBDIR "kconfigtest", KConfig::IncludeGlobals);

    // Age the timestamp of global config file a few sec, and collect it.
    QString globFile = kdeGlobalsPath();
    ageTimeStamp(globFile, 2); // age 2 sec
    oldStamp = QFileInfo(globFile).lastModified();

    // Add a local entry and sync the config.
    // Should not rewrite the global config file.
    KConfigGroup cgLocal(&sc, "Locals");
    cgLocal.writeEntry("someLocalString", "whatever");
    QVERIFY(sc.sync());

    // Verify that the timestamp of global config file didn't change.
    newStamp = QFileInfo(globFile).lastModified();
    QCOMPARE(newStamp, oldStamp);

    // Age the timestamp of local config file a few sec, and collect it.
    QString locFile = testConfigDir() + "/kconfigtest";
    ageTimeStamp(locFile, 2); // age 2 sec
    oldStamp = QFileInfo(locFile).lastModified();

    // Add a global entry and sync the config.
    // Should not rewrite the local config file.
    KConfigGroup cgGlobal(&sc, "Globals");
    cgGlobal.writeEntry("someGlobalString", "whatever",
                        KConfig::Persistent | KConfig::Global);
    QVERIFY(sc.sync());

    // Verify that the timestamp of local config file didn't change.
    newStamp = QFileInfo(locFile).lastModified();
    QCOMPARE(newStamp, oldStamp);
}

void KConfigTest::testFailOnReadOnlyFileSync()
{
    KConfig sc(TEST_SUBDIR "kconfigfailonreadonlytest");
    KConfigGroup cgLocal(&sc, "Locals");

    cgLocal.writeEntry("someLocalString", "whatever");
    QVERIFY(cgLocal.sync());

    QFile f(testConfigDir() + "kconfigfailonreadonlytest");
    QVERIFY(f.exists());
    QVERIFY(f.setPermissions(QFileDevice::ReadOwner));

#ifndef Q_OS_WIN
    if (::getuid() == 0)
        QSKIP("Root can write to read-only files");
#endif
    cgLocal.writeEntry("someLocalString", "whatever2");
    QVERIFY(!cgLocal.sync());

    QVERIFY(f.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner));
    QVERIFY(f.remove());
}

void KConfigTest::testDirtyOnEqual()
{
    QDateTime oldStamp, newStamp;
    KConfig sc(TEST_SUBDIR "kconfigtest");

    // Initialize value
    KConfigGroup cgLocal(&sc, "random");
    cgLocal.writeEntry("theKey", "whatever");
    QVERIFY(sc.sync());

    // Age the timestamp of local config file a few sec, and collect it.
    QString locFile = testConfigDir() + "/kconfigtest";
    ageTimeStamp(locFile, 2); // age 2 sec
    oldStamp = QFileInfo(locFile).lastModified();

    // Write exactly the same again
    cgLocal.writeEntry("theKey", "whatever");
    // This should be a no-op
    QVERIFY(sc.sync());

    // Verify that the timestamp of local config file didn't change.
    newStamp = QFileInfo(locFile).lastModified();
    QCOMPARE(newStamp, oldStamp);
}

void KConfigTest::testDirtyOnEqualOverdo()
{
    QByteArray val1("\0""one", 4);
    QByteArray val2("\0""two", 4);
    QByteArray defvalr;

    KConfig sc(TEST_SUBDIR "kconfigtest");
    KConfigGroup cgLocal(&sc, "random");
    cgLocal.writeEntry("someKey", val1);
    QCOMPARE(cgLocal.readEntry("someKey", defvalr), val1);
    cgLocal.writeEntry("someKey", val2);
    QCOMPARE(cgLocal.readEntry("someKey", defvalr), val2);
}

void KConfigTest::testCreateDir()
{
    // Test auto-creating the parent directory when needed (KConfigIniBackend::createEnclosing)
    QString kdehome = QDir::home().canonicalPath() + "/.kde-unit-test";
    QString subdir = kdehome + "/newsubdir";
    QString file = subdir + "/foo.desktop";
    QFile::remove(file);
    QDir().rmdir(subdir);
    QVERIFY(!QDir().exists(subdir));
    KDesktopFile desktopFile(file);
    desktopFile.desktopGroup().writeEntry("key", "value");
    QVERIFY(desktopFile.sync());
    QVERIFY(QFile::exists(file));

    // Cleanup
    QFile::remove(file);
    QDir().rmdir(subdir);
}

void KConfigTest::testSyncOnExit()
{
    // Often, the KGlobalPrivate global static's destructor ends up calling ~KConfig ->
    // KConfig::sync ... and if that code triggers KGlobal code again then things could crash.
    // So here's a test for modifying KSharedConfig::openConfig() and not syncing, the process exit will sync.
    KConfigGroup grp(KSharedConfig::openConfig(TEST_SUBDIR "syncOnExitRc"), "syncOnExit");
    grp.writeEntry("key", "value");
}

void KConfigTest::testSharedConfig()
{
    // Can I use a KConfigGroup even after the KSharedConfigPtr goes out of scope?
    KConfigGroup myConfigGroup;
    {
        KSharedConfigPtr config = KSharedConfig::openConfig(TEST_SUBDIR "kconfigtest");
        myConfigGroup = KConfigGroup(config, "Hello");
    }
    QCOMPARE(myConfigGroup.readEntry("stringEntry1"), QString(STRINGENTRY1));

    // Get the main config
    KSharedConfigPtr mainConfig = KSharedConfig::openConfig();
    KConfigGroup mainGroup(mainConfig, "Main");
    QCOMPARE(mainGroup.readEntry("Key", QString()), QString("Value"));
}

void KConfigTest::testLocaleConfig()
{
    // Initialize the testdata
    QDir dir;
    QString subdir = testConfigDir();
    dir.mkpath(subdir);
    QString file = subdir + "/localized.test";
    QFile::remove(file);
    QFile f(file);
    QVERIFY(f.open(QIODevice::WriteOnly));
    QTextStream ts(&f);
    ts << "[Test_Wrong]\n";
    ts << "foo[ca]=5\n";
    ts << "foostring[ca]=nice\n";
    ts << "foobool[ca]=true\n";
    ts << "[Test_Right]\n";
    ts << "foo=5\n";
    ts << "foo[ca]=5\n";
    ts << "foostring=primary\n";
    ts << "foostring[ca]=nice\n";
    ts << "foobool=primary\n";
    ts << "foobool[ca]=true\n";
    f.close();

    // Load the testdata
    QVERIFY(QFile::exists(file));
    KConfig config(file);
    config.setLocale("ca");

    // This group has only localized values. That is not supported. The values
    // should be dropped on loading.
    KConfigGroup cg(&config, "Test_Wrong");
    QEXPECT_FAIL("", "The localized values are not dropped", Continue);
    QVERIFY(!cg.hasKey("foo"));
    QEXPECT_FAIL("", "The localized values are not dropped", Continue);
    QVERIFY(!cg.hasKey("foostring"));
    QEXPECT_FAIL("", "The localized values are not dropped", Continue);
    QVERIFY(!cg.hasKey("foobool"));

    // Now check the correct config group
    KConfigGroup cg2(&config, "Test_Right");
    QCOMPARE(cg2.readEntry("foo"), QString("5"));
    QCOMPARE(cg2.readEntry("foo", 3), 5);
    QCOMPARE(cg2.readEntry("foostring"), QString("nice"));
    QCOMPARE(cg2.readEntry("foostring", "ugly"), QString("nice"));
    QCOMPARE(cg2.readEntry("foobool"), QString("true"));
    QCOMPARE(cg2.readEntry("foobool", false), true);

    // Clean up after the testcase
    QFile::remove(file);
}

void KConfigTest::testDeleteWhenLocalized()
{
    // Initialize the testdata
    QDir dir;
    QString subdir = QDir::home().canonicalPath() + "/.kde-unit-test/";
    dir.mkpath(subdir);
    QString file = subdir + "/localized_delete.test";
    QFile::remove(file);
    QFile f(file);
    QVERIFY(f.open(QIODevice::WriteOnly));
    QTextStream ts(&f);
    ts << "[Test4711]\n";
    ts << "foo=3\n";
    ts << "foo[ca]=5\n";
    ts << "foo[de]=7\n";
    ts << "foostring=ugly\n";
    ts << "foostring[ca]=nice\n";
    ts << "foostring[de]=schoen\n";
    ts << "foobool=false\n";
    ts << "foobool[ca]=true\n";
    ts << "foobool[de]=true\n";
    f.close();

    // Load the testdata. We start in locale "ca".
    QVERIFY(QFile::exists(file));
    KConfig config(file);
    config.setLocale("ca");
    KConfigGroup cg(&config, "Test4711");

    // Delete a value. Once with localized, once with Normal
    cg.deleteEntry("foostring", KConfigBase::Persistent | KConfigBase::Localized);
    cg.deleteEntry("foobool");
    QVERIFY(config.sync());

    // The value is now gone. The others are still there. Everything correct
    // here.
    QVERIFY(!cg.hasKey("foostring"));
    QVERIFY(!cg.hasKey("foobool"));
    QVERIFY(cg.hasKey("foo"));

    // The current state is: (Just return before this comment.)
    // [...]
    // foobool[ca]=true
    // foobool[de]=wahr
    // foostring=ugly
    // foostring[de]=schoen

    // Now switch the locale to "de" and repeat the checks. Results should be
    // the same. But they currently are not. The localized value are
    // independent of each other. All values are still there in "de".
    config.setLocale("de");
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foostring"));
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foobool"));
    QVERIFY(cg.hasKey("foo"));
    // Check where the wrong values come from.
    // We get the "de" value.
    QCOMPARE(cg.readEntry("foostring", "nothing"), QString("schoen"));
    // We get the "de" value.
    QCOMPARE(cg.readEntry("foobool", false), true);

    // Now switch the locale back "ca" and repeat the checks. Results are
    // again different.
    config.setLocale("ca");
    // This line worked above. But now it fails.
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foostring"));
    // This line worked above too.
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foobool"));
    QVERIFY(cg.hasKey("foo"));
    // Check where the wrong values come from.
    // We get the primary value because the "ca" value was deleted.
    QCOMPARE(cg.readEntry("foostring", "nothing"), QString("ugly"));
    // We get the "ca" value.
    QCOMPARE(cg.readEntry("foobool", false), true);

    // Now test the deletion of a group.
    cg.deleteGroup();
    QVERIFY(config.sync());

    // Current state: [ca] and [de] entries left... oops.
    //qDebug() << readLinesFrom(file);

    // Bug: The group still exists [because of the localized entries]...
    QVERIFY(cg.exists());
    QVERIFY(!cg.hasKey("foo"));
    QVERIFY(!cg.hasKey("foostring"));
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foobool"));

    // Now switch the locale to "de" and repeat the checks. All values
    // still here because only the primary values are deleted.
    config.setLocale("de");
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foo"));
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foostring"));
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foobool"));
    // Check where the wrong values come from.
    // We get the "de" value.
    QCOMPARE(cg.readEntry("foostring", "nothing"), QString("schoen"));
    // We get the "de" value.
    QCOMPARE(cg.readEntry("foobool", false), true);
    // We get the "de" value.
    QCOMPARE(cg.readEntry("foo", 0), 7);

    // Now switch the locale to "ca" and repeat the checks
    // "foostring" is now really gone because both the primary value and the
    // "ca" value are deleted.
    config.setLocale("ca");
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foo"));
    QVERIFY(!cg.hasKey("foostring"));
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foobool"));
    // Check where the wrong values come from.
    // We get the "ca" value.
    QCOMPARE(cg.readEntry("foobool", false), true);
    // We get the "ca" value.
    QCOMPARE(cg.readEntry("foo", 0), 5);

    // Cleanup
    QFile::remove(file);
}

void KConfigTest::testKdeGlobals()
{
    {
        KConfig glob("kdeglobals");
        KConfigGroup general(&glob, "General");
        general.writeEntry("testKG", "1");
        QVERIFY(glob.sync());
    }

    KConfig globRead("kdeglobals");
    const KConfigGroup general(&globRead, "General");
    QCOMPARE(general.readEntry("testKG"), QString("1"));

    // Check we wrote into kdeglobals
    const QList<QByteArray> lines = readLines("kdeglobals");
    QVERIFY(lines.contains("[General]\n"));
    QVERIFY(lines.contains("testKG=1\n"));

    // Writing using NoGlobals
    {
        KConfig glob("kdeglobals", KConfig::NoGlobals);
        KConfigGroup general(&glob, "General");
        general.writeEntry("testKG", "2");
        QVERIFY(glob.sync());
    }
    globRead.reparseConfiguration();
    QCOMPARE(general.readEntry("testKG"), QString("2"));

    // Reading using NoGlobals
    {
        KConfig globReadNoGlob("kdeglobals", KConfig::NoGlobals);
        const KConfigGroup generalNoGlob(&globReadNoGlob, "General");
        QCOMPARE(generalNoGlob.readEntry("testKG"), QString("2"));
    }

    // TODO now use kconfigtest and writeEntry(,Global) -> should go into kdeglobals
}

void KConfigTest::testAnonymousConfig()
{
    KConfig anonConfig(QString(), KConfig::SimpleConfig);
    KConfigGroup general(&anonConfig, "General");
    QCOMPARE(general.readEntry("testKG"), QString()); // no kdeglobals merging
    general.writeEntry("Foo", "Bar");
    QCOMPARE(general.readEntry("Foo"), QString("Bar"));
}

void KConfigTest::testNewlines()
{
    // test that kconfig always uses the native line endings
    QTemporaryFile file;
    QVERIFY(file.open());
    qWarning() << file.fileName();
    KConfig anonConfig(file.fileName(), KConfig::SimpleConfig);
    KConfigGroup general(&anonConfig, "General");
    general.writeEntry("Foo", "Bar");
    general.writeEntry("Bar", "Foo");
    anonConfig.sync();
    file.flush();
    file.close();
    QFile readFile(file.fileName());
    QVERIFY(readFile.open(QFile::ReadOnly));
#ifndef Q_OS_WIN
    QCOMPARE(readFile.readAll(), QByteArrayLiteral("[General]\nBar=Foo\nFoo=Bar\n"));
#else
    QCOMPARE(readFile.readAll(), QByteArrayLiteral("[General]\r\nBar=Foo\r\nFoo=Bar\r\n"));
#endif

}

void KConfigTest::testXdgListEntry()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    QTextStream out(&file);
    out <<  "[General]" << endl
        << "Key1=" << endl // empty list
        // emtpty entries
        << "Key2=;" << endl
        << "Key3=;;" << endl
        << "Key4=;;;" << endl
        << "Key5=\\;" << endl
        << "Key6=1;2\\;3;;" << endl;
    out.flush();
    file.close();
    KConfig anonConfig(file.fileName(), KConfig::SimpleConfig);
    KConfigGroup grp = anonConfig.group("General");
    QStringList invalidList; // use this as a default when an empty list is expected
    invalidList << "Error! Default value read!";
    QCOMPARE(grp.readXdgListEntry("Key1", invalidList), QStringList());
    QCOMPARE(grp.readXdgListEntry("Key2", invalidList), QStringList() << QString());
    QCOMPARE(grp.readXdgListEntry("Key3", invalidList), QStringList() << QString() << QString());
    QCOMPARE(grp.readXdgListEntry("Key4", invalidList), QStringList()<< QString() << QString() << QString());
    QCOMPARE(grp.readXdgListEntry("Key5", invalidList), QStringList() << ";");
    QCOMPARE(grp.readXdgListEntry("Key6", invalidList), QStringList() << "1" << "2;3" << QString());
}

#include <QThreadPool>
#include <qtconcurrentrun.h>

// To find multithreading bugs: valgrind --tool=helgrind --track-lockorders=no ./kconfigtest testThreads
void KConfigTest::testThreads()
{
    QThreadPool::globalInstance()->setMaxThreadCount(6);
    QList<QFuture<void> > futures;
    // Run in parallel some tests that work on different config files,
    // otherwise unexpected things might indeed happen.
    futures << QtConcurrent::run(this, &KConfigTest::testAddConfigSources);
    futures << QtConcurrent::run(this, &KConfigTest::testSimple);
    futures << QtConcurrent::run(this, &KConfigTest::testDefaults);
    futures << QtConcurrent::run(this, &KConfigTest::testSharedConfig);
    futures << QtConcurrent::run(this, &KConfigTest::testSharedConfig);
    // QEXPECT_FAIL triggers race conditions, it should be fixed to use QThreadStorage...
    //futures << QtConcurrent::run(this, &KConfigTest::testDeleteWhenLocalized);
    //futures << QtConcurrent::run(this, &KConfigTest::testEntryMap);
    Q_FOREACH (QFuture<void> f, futures) { // krazy:exclude=foreach
        f.waitForFinished();
    }
}
