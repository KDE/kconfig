/*
    SPDX-FileCopyrightText: 2019 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <QGuiApplication>
#include <QFunctionPointer>

#include "test_fileextensions.hxx"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    TestFileExtensions *t = TestFileExtensions::self();
    delete t;
    return 0;
}
