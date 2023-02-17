/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Canonical Ltd
    SPDX-FileContributor: Aurélien Gâteau <aurelien.gateau@canonical.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "test_kconf_update.h"

#include <memory>

// Qt
#include "config-kconf.h"
#include <QDir>
#include <QFile>
#include <QProcess>
#include <qstandardpaths.h>
#include <qtemporaryfile.h>

#include <qtest.h>

QTEST_GUILESS_MAIN(TestKConfUpdate)

void TestKConfUpdate::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    // Ensure it all works with spaces in paths (as happens more commonly on OSX where it's ~/Library/Application Support/)
    qputenv("XDG_DATA_HOME", "/tmp/a b");
}

static void writeFile(const QString &path, const QString &content)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qFatal("Could not write to '%s'", qPrintable(path));
    }
    file.write(content.toUtf8());
}

static QString readFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qFatal("Could not read '%s'", qPrintable(path));
    }
    QString ret = QString::fromUtf8(file.readAll());
#ifdef Q_OS_WIN
    // KConfig always writes files with the native line ending, the test comparison uses \n
    ret.replace(QLatin1String{"\r\n"}, QLatin1String{"\n"});
#endif
    return ret;
}

static std::unique_ptr<QTemporaryFile> writeUpdFile(const QString &content)
{
    std::unique_ptr<QTemporaryFile> file{new QTemporaryFile(QDir::tempPath() + QLatin1String("/test_kconf_update_XXXXXX.upd"))};
    bool ok = file->open();
    Q_UNUSED(ok) // silence warnings
    Q_ASSERT(ok);
    file->write(content.toUtf8());
    file->flush();
    return file;
}

static void runKConfUpdate(const QString &updPath)
{
    QVERIFY(QFile::exists(QStringLiteral(KCONF_UPDATE_EXECUTABLE)));
    QCOMPARE(0, QProcess::execute(QStringLiteral(KCONF_UPDATE_EXECUTABLE), QStringList{QStringLiteral("--testmode"), QStringLiteral("--debug"), updPath}));
}

void TestKConfUpdate::testScript_data()
{
#ifdef Q_OS_WIN
    // add sh.exe and sed.exe to PATH on Windows
    // uncomment and adapt path to run all tests
    // qputenv("PATH", qgetenv("PATH") + ";C:\\kde\\msys\\bin");
#endif

    QTest::addColumn<QString>("updContent");
    QTest::addColumn<QString>("updScript");
    QTest::addColumn<QString>("oldConfContent");
    QTest::addColumn<QString>("expectedNewConfContent");

    QTest::newRow("delete-key") << "File=testrc\n"
                                   "Group=group\n"
                                   "Script=test.sh,sh\n"
                                << "echo '# DELETE deprecated'\n"
                                << "[group]\n"
                                   "deprecated=foo\n"
                                   "valid=bar\n"
                                << "[$Version]\n"
                                   "update_info=%1\n"
                                   "\n"
                                   "[group]\n"
                                   "valid=bar\n";

    QTest::newRow("delete-key2") << "File=testrc\n"
                                    "Script=test.sh,sh\n"
                                 << "echo '# DELETE [group]deprecated'\n"
                                    "echo '# DELETE [group][sub]deprecated2'\n"
                                 << "[group]\n"
                                    "deprecated=foo\n"
                                    "valid=bar\n"
                                    "\n"
                                    "[group][sub]\n"
                                    "deprecated2=foo\n"
                                    "valid2=bar\n"
                                 << "[$Version]\n"
                                    "update_info=%1\n"
                                    "\n"
                                    "[group]\n"
                                    "valid=bar\n"
                                    "\n"
                                    "[group][sub]\n"
                                    "valid2=bar\n";

    QTest::newRow("delete-group") << "File=testrc\n"
                                     "Script=test.sh,sh\n"
                                  << "echo '# DELETEGROUP [group1]'\n"
                                     "echo '# DELETEGROUP [group2][sub]'\n"
                                  << "[group1]\n"
                                     "key=value\n"
                                     "\n"
                                     "[group2]\n"
                                     "valid=bar\n"
                                     "\n"
                                     "[group2][sub]\n"
                                     "key=value\n"
                                  << "[$Version]\n"
                                     "update_info=%1\n"
                                     "\n"
                                     "[group2]\n"
                                     "valid=bar\n";

    QTest::newRow("delete-group2") << "File=testrc\n"
                                      "Group=group\n"
                                      "Script=test.sh,sh\n"
                                   << "echo '# DELETEGROUP'\n"
                                   << "[group]\n"
                                      "key=value\n"
                                      "\n"
                                      "[group2]\n"
                                      "valid=bar\n"
                                   << "[$Version]\n"
                                      "update_info=%1\n"
                                      "\n"
                                      "[group2]\n"
                                      "valid=bar\n";

    QTest::newRow("new-key") << "File=testrc\n"
                                "Script=test.sh,sh\n"
                             << "echo '[group]'\n"
                                "echo 'new=value'\n"
                             << "[group]\n"
                                "valid=bar\n"
                             << "[$Version]\n"
                                "update_info=%1\n"
                                "\n"
                                "[group]\n"
                                "new=value\n"
                                "valid=bar\n";

    QTest::newRow("modify-key-no-overwrite") << "File=testrc\n"
                                                "Script=test.sh,sh\n"
                                             << "echo '[group]'\n"
                                                "echo 'existing=new'\n"
                                             << "[group]\n"
                                                "existing=old\n"
                                             << "[$Version]\n"
                                                "update_info=%1\n"
                                                "\n"
                                                "[group]\n"
                                                "existing=old\n";

    QTest::newRow("modify-key-overwrite") << "File=testrc\n"
                                             "Options=overwrite\n"
                                             "Script=test.sh,sh\n"
                                          << "echo '[group]'\n"
                                             "echo 'existing=new'\n"
                                          << "[group]\n"
                                             "existing=old\n"
                                          << "[$Version]\n"
                                             "update_info=%1\n"
                                             "\n"
                                             "[group]\n"
                                             "existing=new\n";

    QTest::newRow("new-key-in-subgroup") << "File=testrc\n"
                                            "Script=test.sh,sh\n"
                                         << "echo '[group][sub]'\n"
                                            "echo 'new=value2'\n"
                                         << "[group][sub]\n"
                                            "existing=foo\n"
                                         << "[$Version]\n"
                                            "update_info=%1\n"
                                            "\n"
                                            "[group][sub]\n"
                                            "existing=foo\n"
                                            "new=value2\n";

    QTest::newRow("new-key-in-subgroup2") << "File=testrc\n"
                                             "Script=test.sh,sh\n"
                                          << "echo '[group][sub]'\n"
                                             "echo 'new=value3'\n"
                                          << "[group][sub]\n"
                                             "existing=foo\n"
                                          << "[$Version]\n"
                                             "update_info=%1\n"
                                             "\n"
                                             "[group][sub]\n"
                                             "existing=foo\n"
                                             "new=value3\n";

    if (QStandardPaths::findExecutable(QStringLiteral("sed")).isEmpty()) {
        qWarning("sed executable not found, cannot run all tests!");
    } else {
        QTest::newRow("filter") << "File=testrc\n"
                                   "Script=test.sh,sh\n"
                                << "echo '# DELETE [group]changed'\n"
                                   "sed s/value/VALUE/\n"
                                << "[group]\n"
                                   "changed=value\n"
                                   "unchanged=value\n"
                                << "[$Version]\n"
                                   "update_info=%1\n"
                                   "\n"
                                   "[group]\n"
                                   "changed=VALUE\n"
                                   "unchanged=value\n";

        QTest::newRow("filter-subgroup") << "File=testrc\n"
                                            "Script=test.sh,sh\n"
                                         << "echo '# DELETE [group][sub]changed'\n"
                                            "sed s/value/VALUE/\n"
                                         << "[group]\n"
                                            "unchanged=value\n"
                                            "\n"
                                            "[group][sub]\n"
                                            "changed=value\n"
                                            "unchanged=value\n"
                                         << "[$Version]\n"
                                            "update_info=%1\n"
                                            "\n"
                                            "[group]\n"
                                            "unchanged=value\n"
                                            "\n"
                                            "[group][sub]\n"
                                            "changed=VALUE\n"
                                            "unchanged=value\n";
    }
}

void TestKConfUpdate::testScript()
{
    if (QStandardPaths::findExecutable(QStringLiteral("sh")).isEmpty()) {
        QSKIP("Could not find sh executable, cannot run test");
        return;
    }

    QFETCH(QString, updContent);
    QFETCH(QString, updScript);
    QFETCH(QString, oldConfContent);
    QFETCH(QString, expectedNewConfContent);

    // Prepend the Version and Id= field to the upd content
    updContent.prepend(QLatin1String("Version=5\nId=%1\n").arg(QLatin1String{QTest::currentDataTag()}));

    std::unique_ptr<QTemporaryFile> updFile(writeUpdFile(updContent));

    const QString scriptDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String{"/kconf_update"};
    QVERIFY(QDir().mkpath(scriptDir));
    const QString scriptPath = scriptDir + QLatin1String{"/test.sh"};
    writeFile(scriptPath, updScript);
    QCOMPARE(readFile(scriptPath), updScript);

    const QString confPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String{"/testrc"};
    writeFile(confPath, oldConfContent);
    QCOMPARE(readFile(confPath), oldConfContent);

    runKConfUpdate(updFile->fileName());

    const QString updateInfo = QLatin1String("%1:%2").arg(updFile->fileName().section(QLatin1Char{'/'}, -1), QLatin1String{QTest::currentDataTag()});
    QString newConfContent = readFile(confPath);
    expectedNewConfContent = expectedNewConfContent.arg(updateInfo);
    QCOMPARE(newConfContent, expectedNewConfContent);
}
