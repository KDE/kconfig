/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2022 g10 Code GmbH
    SPDX-FileContributor: Andre Heinecke <aheinecke@gnupg.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kconfigdata_p.h"

#ifdef Q_OS_WIN
void parseWindowsRegistry(const QString &regKey, KEntryMap &entryMap);
#else
void parseWindowsRegistry(const QString &, KEntryMap &) {}
#endif
