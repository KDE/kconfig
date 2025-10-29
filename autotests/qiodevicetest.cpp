// SPDX-License-Identifier: LGPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Harald Sitter <sitter@kde.org>

#include <QBuffer>
#include <QTemporaryFile>
#include <QTest>

#include <KConfig>
#include <KConfigGroup>

using namespace Qt::StringLiterals;

class QIODeviceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testWriteRead()
    {
        // write to file
        auto buffer = std::make_shared<QBuffer>();
        QVERIFY(buffer->open(QIODevice::ReadWrite | QIODevice::Text));
        {
            KConfig config(buffer, KConfig::OpenFlag::SimpleConfig);
            config.group(u"Test"_s).writeEntry("key", "value");
        }
        buffer->close();

        QVERIFY(buffer->open(QIODevice::ReadOnly | QIODevice::Text));
        QVERIFY(buffer->isReadable());

        // check content
        QCOMPARE(buffer->readAll(), "[Test]\nkey=value\n");
        buffer->seek(0);

        // read back the config
        {
            KConfig config(buffer, KConfig::OpenFlag::SimpleConfig);

            QVERIFY(config.hasGroup(u"Test"_s));
            QCOMPARE(config.group(u"Test"_s).readEntry("key"), u"value"_s);
        }
    }

    void testKdeGlobals()
    {
        {
            KConfig glob(QStringLiteral("kdeglobals"));
            KConfigGroup general(&glob, QStringLiteral("General"));
            general.writeEntry("testKG", "1");
            QVERIFY(glob.sync());
        }

        KConfig globRead(QStringLiteral("kdeglobals"));
        const KConfigGroup general(&globRead, QStringLiteral("General"));
        QCOMPARE(general.readEntry("testKG"), QStringLiteral("1"));
        qDebug() << "globRead" << globRead.name();

        auto buffer = std::make_shared<QBuffer>();
        QVERIFY(buffer->open(QIODevice::ReadOnly | QIODevice::Text));
        {
            KConfig config(buffer, KConfig::OpenFlag::FullConfig);
            QVERIFY(config.hasGroup(u"General"_s));
            config.group(u"General"_s).writeEntry("testKG", "1");
        }
    }

    void testAddConfigSources()
    {
        QTemporaryFile extrafile;
        extrafile.setAutoRemove(false);
        QVERIFY(extrafile.open());
        extrafile.close();

        {
            KConfig glob(extrafile.fileName());
            KConfigGroup extra(&glob, QStringLiteral("Extra"));
            extra.writeEntry("testKG", "1");
            QVERIFY(glob.sync());
        }

        KConfig globRead(extrafile.fileName());
        const KConfigGroup extra(&globRead, QStringLiteral("General"));
        QCOMPARE(extra.readEntry("testKG"), QStringLiteral("1"));

        auto buffer = std::make_shared<QBuffer>();
        QVERIFY(buffer->open(QIODevice::ReadOnly | QIODevice::Text));
        {
            KConfig config(buffer, KConfig::OpenFlag::CascadeConfig);
            config.addConfigSources({extrafile.fileName()});
            QVERIFY(config.hasGroup(u"Extra"_s));
            config.group(u"Extra"_s).writeEntry("testKG", "1");
        }

        QVERIFY(extrafile.remove());
    }
};

QTEST_MAIN(QIODeviceTest)

#include "qiodevicetest.moc"
