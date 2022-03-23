/*
    SPDX-FileCopyrightText: 2004 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <QFunctionPointer>
#include <QGuiApplication>

#include "test_qdebugcategory.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    TestQCategory *t = new TestQCategory(42);
    delete t;
    return 0;
}
