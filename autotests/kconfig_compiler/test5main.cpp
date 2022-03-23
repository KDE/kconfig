/*
    SPDX-FileCopyrightText: 2004 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <QFunctionPointer>
#include <QGuiApplication>

#include "test5.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    Test5 *t = Test5::self();
    delete t;
    return 0;
}
