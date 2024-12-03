/*
    SPDX-FileCopyrightText: 2024 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: MIT
*/

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTest>

class Test16Qml : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testQml()
    {
        QQmlApplicationEngine engine;

        engine.loadFromModule("org.kde.kconfig.test16", "Test16Qml");

        QVERIFY(engine.rootObjects().size());
    }
};

QTEST_MAIN(Test16Qml)

#include "test16qml.moc"
