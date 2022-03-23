/*
    SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: MIT
*/
#include <QFunctionPointer>
#include <QGuiApplication>

#include "test2.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    Test2 *t = new Test2();
    delete t;
    return 0;
}
