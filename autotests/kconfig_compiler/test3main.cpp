/*
    SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: MIT
*/
#include <QFunctionPointer>
#include <QGuiApplication>

#include "test3.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    TestNameSpace::Test3 *t = new TestNameSpace::Test3();
    delete t;
    return 0;
}
