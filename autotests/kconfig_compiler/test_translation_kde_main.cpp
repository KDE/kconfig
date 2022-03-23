/*
    SPDX-FileCopyrightText: 2015 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: MIT
*/

#include <QFunctionPointer>
#include <QGuiApplication>

#include "test_translation_kde.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    TestNameSpace::TestTranslationKde *t = new TestNameSpace::TestTranslationKde();
    delete t;
    return 0;
}
