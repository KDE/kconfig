/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: MIT
*/

#include <QFunctionPointer>
#include <QGuiApplication>
#include <QStaticStringData>
#include <QStringLiteral>

#include "test_emptyentries.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    QMakeBuilderSettings::instance(QStringLiteral("abc"));
    auto *t = QMakeBuilderSettings::self();
    delete t;
    return 0;
}
