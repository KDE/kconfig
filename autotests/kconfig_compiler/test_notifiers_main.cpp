/*
    SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: MIT
*/

#include <QFunctionPointer>
#include <QGuiApplication>

#include "test_notifiers.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    TestNotifiers *t = new TestNotifiers(42);
    delete t;
    return 0;
}
