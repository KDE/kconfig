/*
    SPDX-FileCopyrightText: 2022 Pino Toscano <pino@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "test_64_entries.h"
#include <QTest>

class Test64EntriesConfig : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testLastEnumValue()
    {
        QCOMPARE(Test64Entries::signalKey64Changed, 0x8000000000000000);
    }
};

QTEST_MAIN(Test64EntriesConfig)

#include "test_64_entries_main.moc"
