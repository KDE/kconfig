/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2022 g10 Code GmbH
    SPDX-FileContributor: Andre Heinecke <aheinecke@gnupg.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "registry_win_p.h"
#include "kconfig_core_log_settings.h"

#include <QSettings>

void parseRegValues(const QString &groupName, QSettings &settings, KEntryMap &entryMap, bool groupImmutable)
{
    for (auto &key : settings.childKeys()) {
        KEntryMap::EntryOptions entryOptions = KEntryMap::EntryDefault;
        const auto value = settings.value(key).toByteArray();
        if (key.endsWith(QStringLiteral("[$i]")) || groupImmutable) {
            key.chop(4);
            entryOptions |= KEntryMap::EntryImmutable;
        }
        if (entryMap.getEntryOption(groupName.toUtf8(), key.toUtf8(), KEntryMap::SearchDefaults, KEntryMap::EntryImmutable)) {
            continue;
        }
        entryMap.setEntry(groupName.toUtf8(), key.toUtf8(), settings.value(key).toByteArray(), entryOptions);
        qWarning() << "Addings Entry" << key << value << "to group" << groupName;
    }
}

void parseRegSubkeys(const QString &regKey, KEntryMap &entryMap, bool userRegistry)
{
    QSettings settings((userRegistry ? QStringLiteral("HKEY_LOCAL_MACHINE\\") : QStringLiteral("HKEY_CURRENT_USER")) + regKey, QSettings::NativeFormat);

    parseRegValues(QStringLiteral("<default>"), settings, entryMap, userRegistry);

    for (auto &group : settings.childGroups()) {
        bool immutable = false;
        if (group.endsWith(QStringLiteral("[$i]"))) {
            group.chop(4);
            immutable = true;
        }
        settings.beginGroup(group);
        parseRegValues(group, settings, entryMap, immutable);
        settings.endGroup();
    }
}

void parseWindowsRegistry(const QString &regKey, KEntryMap &entryMap)
{
    // First take the HKLM values into account
    parseRegSubkeys(regKey, entryMap, false);
    // Then HKCU
    parseRegSubkeys(regKey, entryMap, true);
}
