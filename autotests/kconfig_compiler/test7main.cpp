/*
    SPDX-FileCopyrightText: 2004 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <QFunctionPointer>
#include <QGuiApplication>

#include "test7.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    Test7 *t = new Test7(42);
    delete t;
    return 0;
}
