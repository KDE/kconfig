#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

from KConfigCore import KConfig, KConfigGroup

kdeglobals = KConfig("kdeglobals")

general = kdeglobals.group("General")

bell = general.readEntry("UseSystemBell", True)

print(f"System bell is on: {bell}")
