/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: MIT
*/

#include <QFunctionPointer>
#include <QGuiApplication>

#include "test10.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    Test10 *t = Test10::self();
    delete t;
    return 0;
}
