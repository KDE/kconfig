/*
    SPDX-FileCopyrightText: 2004 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <QFunctionPointer>
#include <QGuiApplication>
#include <QString>

#include "test6.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    Test6 *t = new Test6(QString());
    delete t;
    return 0;
}
