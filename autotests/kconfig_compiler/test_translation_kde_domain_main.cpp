/*
    SPDX-FileCopyrightText: 2015 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: MIT
*/

#include <QFunctionPointer>
#include <QGuiApplication>

#include "test_translation_kde_domain.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    TestNameSpace::TestTranslationKdeDomain *t = new TestNameSpace::TestTranslationKdeDomain();
    delete t;
    return 0;
}
