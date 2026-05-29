/*
    SPDX-FileCopyrightText: 2026 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: MIT
*/
#include "test19.h"

int main(int, char **)
{
    QStandardPaths::setTestModeEnabled(true);
    Foo::Test19::self();
    return 0;
}
