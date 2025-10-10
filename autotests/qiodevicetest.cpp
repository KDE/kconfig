// SPDX-License-Identifier: LGPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Harald Sitter <sitter@kde.org>

#include <QBuffer>
#include <QTest>

#include <KConfig>
#include <KConfigGroup>

using namespace Qt::StringLiterals;

class QIODeviceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testWrite()
    {
        auto buffer = std::make_shared<QBuffer>();
        QVERIFY(buffer->open(QIODevice::ReadWrite | QIODevice::Text));
        {
            KConfig config(buffer);
            config.group(u"Test"_s).writeEntry("key", "value");
        }
        qDebug().nospace().noquote() << "Buffer:\n" << buffer->buffer();
    }
};

QTEST_MAIN(QIODeviceTest)

#include "qiodevicetest.moc"
