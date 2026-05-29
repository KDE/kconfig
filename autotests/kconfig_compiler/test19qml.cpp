/*
    SPDX-FileCopyrightText: 2026 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: MIT
*/

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTest>

class Test19Qml : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testQml()
    {
        QQmlApplicationEngine engine;

        engine.loadFromModule("org.kde.kconfig.test19", "Test19Qml");

        QVERIFY(!engine.rootObjects().isEmpty());
    }
};

QTEST_MAIN(Test19Qml)

#include "test19qml.moc"
