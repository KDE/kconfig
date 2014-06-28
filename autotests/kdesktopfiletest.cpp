/*
 *  Copyright (C) 2006 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */
#include "kdesktopfiletest.h"
#include "helper.h"
#include <kconfiggroup.h>
#include <qtemporaryfile.h>

#include "kdesktopfile.h"

#include <QtTest>

QTEST_MAIN(KDesktopFileTest)

void KDesktopFileTest::testRead()
{
    QTemporaryFile file("testReadXXXXXX.desktop");
    QVERIFY(file.open());
    const QString fileName = file.fileName();
    QTextStream ts(&file);
    ts <<
       "[Desktop Entry]\n"
       "Type=Application\n"
       "Name=My Application\n"
       "Icon=foo\n"
       "\n";
    file.close();
    QVERIFY(QFile::exists(fileName));
    QVERIFY(KDesktopFile::isDesktopFile(fileName));
    KDesktopFile df(fileName);
    QCOMPARE(df.readType(), QString::fromLatin1("Application"));
    QVERIFY(df.hasApplicationType());
    QCOMPARE(df.readName(), QString::fromLatin1("My Application"));
    QCOMPARE(df.readIcon(), QString::fromLatin1("foo"));
    QVERIFY(!df.hasLinkType());
    QCOMPARE(df.fileName(), QFileInfo(fileName).canonicalFilePath());
}

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
    QTemporaryFile file("testReadLocalizedXXXXXX.desktop");
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
       "Actions=encrypt;\n"
       "[Desktop Action encrypt]\n"
       "Name=Encrypt file\n"
       "\n";
    file.close();
    QVERIFY(QFile::exists(fileName));
    KDesktopFile df(fileName);
    QCOMPARE(df.readType(), QString());
    QCOMPARE(df.fileName(), fileName);
    QCOMPARE(df.readActions(), QStringList() << "encrypt");
    QCOMPARE(df.hasActionGroup("encrypt"), true);
    QCOMPARE(df.hasActionGroup("doesnotexist"), false);
    KConfigGroup cg = df.actionGroup("encrypt");
    QVERIFY(cg.hasKey("Name"));
    QCOMPARE(cg.readEntry("Name"), QString("Encrypt file"));
}

void KDesktopFileTest::testIsAuthorizedDesktopFile()
{
    QTemporaryFile file("testAuthXXXXXX.desktop");
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
