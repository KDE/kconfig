/*
    SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: MIT
*/
#include "test_bug436712.h"

int main(int, char **)
{
    FooSettings *t = new FooSettings();
    delete t;
    return 0;
}
