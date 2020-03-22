/*
    SPDX-FileCopyrightText: 2006 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#include "kdesktopfiletest.h"
#include "helper.h"
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <qtemporaryfile.h>

#include "kdesktopfile.h"

#include <QtTest>

QTEST_MAIN(KDesktopFileTest)

void KDesktopFileTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    KConfigGroup actionRestrictions(KSharedConfig::openConfig(), "KDE Action Restrictions");
    actionRestrictions.writeEntry("someBlockedAction", false);
}

void KDesktopFileTest::testRead()
{
    QTemporaryFile file(QStringLiteral("testReadXXXXXX.desktop"));
    QVERIFY(file.open());
    const QString fileName = file.fileName();
    QTextStream ts(&file);
    ts <<
       "[Desktop Entry]\n"
       "Type= Application\n"
       "Name=My Application\n"
       "Icon = foo\n"
       "MimeType =text/plain;image/png;\n"
       "\n";
    file.close();
    QVERIFY(QFile::exists(fileName));
    QVERIFY(KDesktopFile::isDesktopFile(fileName));
    KDesktopFile df(fileName);
    QCOMPARE(df.readType(), QString::fromLatin1("Application"));
    QVERIFY(df.hasApplicationType());
    QCOMPARE(df.readName(), QString::fromLatin1("My Application"));
    QCOMPARE(df.readIcon(), QString::fromLatin1("foo"));
    QCOMPARE(df.readMimeTypes(), QStringList() << QString::fromLatin1("text/plain")
                                              << QString::fromLatin1("image/png"));
    QVERIFY(!df.hasLinkType());
    QCOMPARE(df.fileName(), QFileInfo(fileName).canonicalFilePath());
}

#if KCONFIGCORE_ENABLE_DEPRECATED_SINCE(5, 0)
void KDesktopFileTest::testReadDirectory()
{
    QTemporaryFile file("testReadDirectoryXXXXXX.directory");
    QVERIFY(file.open());
    const QString fileName = file.fileName();
    QTextStream ts(&file);
    ts <<
       "[Desktop Entry]\n"
       "Type=Directory\n"
       "SortOrder=2.desktop;1.desktop;\n"
       "\n";
    file.close();
    QVERIFY(QFile::exists(fileName));
    KDesktopFile df(fileName);
    QCOMPARE(df.readType(), QString::fromLatin1("Directory"));
    QCOMPARE(df.sortOrder(), QStringList() << QString::fromLatin1("2.desktop")
                                           << QString::fromLatin1("1.desktop"));
}
#endif

void KDesktopFileTest::testReadLocalized_data()
{
    QTest::addColumn<QLocale>("locale");
    QTest::addColumn<QString>("translation");

    const QString german = QStringLiteral("Meine Anwendung");
    const QString swiss  = QStringLiteral("Mein Anwendungsli");

    QTest::newRow("de")           << QLocale(QLocale::German)                << german;
    QTest::newRow("de_DE")        << QLocale(QStringLiteral("de_DE"))        << german;
    QTest::newRow("de_DE@bayern") << QLocale(QStringLiteral("de_DE@bayern")) << german;
    QTest::newRow("de@bayern")    << QLocale(QStringLiteral("de@bayern"))    << german;
    QTest::newRow("de@freiburg")  << QLocale(QStringLiteral("de@freiburg"))  << QStringLiteral("Mein Anwendungsle");
    // For CH we have a special translation
    QTest::newRow("de_CH")      << QLocale(QLocale::German, QLocale::Switzerland) << swiss;
    QTest::newRow("de_CH@bern") << QLocale(QStringLiteral("de_CH@bern"))          << swiss;
    // Austria should fall back to "de"
    QTest::newRow("de_AT")       << QLocale(QLocale::German, QLocale::Austria) << german;
    QTest::newRow("de_AT@tirol") << QLocale(QStringLiteral("de_AT@tirol"))              << german;
    // no translation for French
    QTest::newRow("fr") << QLocale(QLocale::French) << QStringLiteral("My Application");
}

void KDesktopFileTest::testReadLocalized()
{
    QTemporaryFile file(QStringLiteral("testReadLocalizedXXXXXX.desktop"));
    QVERIFY(file.open());
    const QString fileName = file.fileName();
    QTextStream ts(&file);
    ts <<
       "[Desktop Entry]\n"
       "Type=Application\n"
       "Name=My Application\n"
       "Name[de]=Meine Anwendung\n"
       "Name[de@freiburg]=Mein Anwendungsle\n"
       "Name[de_CH]=Mein Anwendungsli\n"
       "Icon=foo\n"
       "\n";
    file.close();
    QVERIFY(QFile::exists(fileName));
    QVERIFY(KDesktopFile::isDesktopFile(fileName));

    DefaultLocale defaultLocale;

    QFETCH(QLocale, locale);
    QLocale::setDefault(locale);
    KDesktopFile df(fileName);

    QEXPECT_FAIL("de@freiburg", "QLocale doesn't support modifiers", Continue);
    QTEST(df.readName(), "translation");
}

void KDesktopFileTest::testSuccessfulTryExec()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    const QString fileName = file.fileName();
    QTextStream ts(&file);
    ts <<
       "[Desktop Entry]\n"
       "TryExec=whoami\n"
       "\n";
    file.close();
    QVERIFY(QFile::exists(fileName));
    KDesktopFile df(fileName);
    QCOMPARE(df.tryExec(), true);
}

void KDesktopFileTest::testUnsuccessfulTryExec()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    const QString fileName = file.fileName();
    QTextStream ts(&file);
    ts <<
       "[Desktop Entry]\n"
       "TryExec=/does/not/exist\n"
       "\n";
    file.close();
    QVERIFY(QFile::exists(fileName));
    KDesktopFile df(fileName);
    QCOMPARE(df.tryExec(), false);
}

void KDesktopFileTest::testActionGroup()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    const QString fileName = file.fileName();
    QTextStream ts(&file);
    ts <<
       "[Desktop Entry]\n"
       // make sure escaping of ';' using "\;" works
       "Actions=encrypt;semi\\;colon;decrypt;\n"
       "[Desktop Action encrypt]\n"
       "Name=Encrypt file\n"
       "[Desktop Action decrypt]\n"
       "Name=Decrypt file\n"
       // no escaping needed in group header
       "[Desktop Action semi;colon]\n"
       "Name=With semicolon\n"
       "\n";
    file.close();
    QVERIFY(QFile::exists(fileName));
    KDesktopFile df(fileName);
    QCOMPARE(df.readType(), QString());
    QCOMPARE(df.fileName(), QFileInfo(fileName).canonicalFilePath());
    QCOMPARE(df.readActions(), QStringList() << "encrypt" << "semi;colon" << "decrypt");
    QCOMPARE(df.hasActionGroup("encrypt"), true);
    QCOMPARE(df.hasActionGroup("semi;colon"), true);
    QCOMPARE(df.hasActionGroup("decrypt"), true);
    QCOMPARE(df.hasActionGroup("doesnotexist"), false);
    KConfigGroup cg = df.actionGroup(QStringLiteral("encrypt"));
    QVERIFY(cg.hasKey("Name"));
    QCOMPARE(cg.readEntry("Name"), QString("Encrypt file"));
    cg = df.actionGroup(QStringLiteral("decrypt"));
    QVERIFY(cg.hasKey("Name"));
    QCOMPARE(cg.readEntry("Name"), QString("Decrypt file"));
    cg = df.actionGroup(QStringLiteral("semi;colon"));
    QVERIFY(cg.hasKey("Name"));
    QCOMPARE(cg.readEntry("Name"), QString("With semicolon"));
}

void KDesktopFileTest::testIsAuthorizedDesktopFile()
{
    QTemporaryFile file(QStringLiteral("testAuthXXXXXX.desktop"));
    QVERIFY(file.open());
    const QString fileName = file.fileName();
    QTextStream ts(&file);
    ts <<
       "[Desktop Entry]\n"
       "Type=Application\n"
       "Name=My Application\n"
       "Exec=kfoo\n"
       "\n";
    file.close();
    QVERIFY(QFile::exists(fileName));
    QVERIFY(!KDesktopFile::isAuthorizedDesktopFile(fileName));

    const QString installedFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("/kservices5/http_cache_cleaner.desktop"));
    if (!installedFile.isEmpty()) {
        QVERIFY(KDesktopFile::isAuthorizedDesktopFile(installedFile));
    } else {
        qWarning("Skipping test for http_cache_cleaner.desktop, not found. kio not installed?");
    }

    const QString autostartFile = QStandardPaths::locate(QStandardPaths::GenericConfigLocation, QStringLiteral("autostart/plasma-desktop.desktop"));
    if (!autostartFile.isEmpty()) {
        QVERIFY(KDesktopFile::isAuthorizedDesktopFile(autostartFile));
    } else {
        qWarning("Skipping test for plasma-desktop.desktop, not found. kde-workspace not installed?");
    }
}

void KDesktopFileTest::testTryExecWithAuthorizeAction()
{
    {
        QTemporaryFile file(QStringLiteral("testAuthActionXXXXXX.desktop"));
        QVERIFY(file.open());
        const QString fileName = file.fileName();
        QTextStream ts(&file);
        ts <<
        "[Desktop Entry]\n"
        "Type=Application\n"
        "Name=My Application\n"
        "Exec=kfoo\n"
        "TryExec=bash\n"
        "X-KDE-AuthorizeAction=someAction"
        "\n";
        file.close();

        KDesktopFile desktopFile(fileName);
        QVERIFY(desktopFile.tryExec());
    }
    {
        QTemporaryFile file(QStringLiteral("testAuthActionXXXXXX.desktop"));
        QVERIFY(file.open());
        const QString fileName = file.fileName();
        QTextStream ts(&file);
        ts <<
        "[Desktop Entry]\n"
        "Type=Application\n"
        "Name=My Application\n"
        "Exec=kfoo\n"
        "TryExec=bash\n"
        "X-KDE-AuthorizeAction=someBlockedAction"
        "\n";
        file.close();

        KDesktopFile desktopFile(fileName);
        QVERIFY(!desktopFile.tryExec());
    }
}

void KDesktopFileTest::testLocateLocal_data()
{
    QString systemConfigLocation = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation).last();
    QString writableConfigLocation = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QString systemDataLocation = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).last();
    QString writableDataLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);

    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("result");

    QTest::newRow("configLocation, system-wide") << systemConfigLocation + "/test.desktop" << writableConfigLocation + "/test.desktop";
    QTest::newRow("autostart, system-wide") << systemConfigLocation + "/autostart/test.desktop" << writableConfigLocation + "/autostart/test.desktop";
    QTest::newRow("dataLocation, system-wide") << systemDataLocation + "/test.desktop" << writableDataLocation + "/test.desktop";
    QTest::newRow("applications, system-wide") << systemDataLocation + "/applications/test.desktop" << writableDataLocation + "/applications/test.desktop";
    QTest::newRow("desktop-directories, system-wide") << systemDataLocation + "/desktop-directories/test.directory" << writableDataLocation + "/desktop-directories/test.directory";
    QTest::newRow("configLocation, writable") << writableConfigLocation + "/test.desktop" << writableConfigLocation + "/test.desktop";
    QTest::newRow("autostart, writable") << writableConfigLocation + "/autostart/test.desktop" << writableConfigLocation + "/autostart/test.desktop";
    QTest::newRow("dataLocation, writable") << writableDataLocation + "/test.desktop" << writableDataLocation + "/test.desktop";
    QTest::newRow("applications, writable") << writableDataLocation + "/applications/test.desktop" << writableDataLocation + "/applications/test.desktop";
    QTest::newRow("desktop-directories, writable") << writableDataLocation + "/desktop-directories/test.directory" << writableDataLocation + "/desktop-directories/test.directory";
    QTest::newRow("unknown location") << "/test.desktop" << writableDataLocation + "/test.desktop";
}

void KDesktopFileTest::testLocateLocal()
{
    QFETCH(QString, path);
    QFETCH(QString, result);

    QCOMPARE(KDesktopFile::locateLocal(path), result);
}
