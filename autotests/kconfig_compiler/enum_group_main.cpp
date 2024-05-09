/*
    SPDX-FileCopyrightText: 2009 Pino Toscano <pino@kde.org>

    SPDX-License-Identifier: MIT
*/
#include "test_enums_and_properties.h"
#include <QGuiApplication>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    TestEnumsAndProperties t = TestEnumsAndProperties();
    return 0;
}
