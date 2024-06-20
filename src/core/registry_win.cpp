/*
    SPDX-FileCopyrightText: 2022 g10 Code GmbH
    SPDX-FileContributor: Andre Heinecke <aheinecke@gnupg.com>
    SPDX-FileContributor: Tobias Fella <tobias.fella@gnupg.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "registry_win_p.h"

#include <QSettings>

enum Registry {
    HKLM,
    HKCU,
};

void parseRegValues(const QString &groupName, QSettings &settings, KEntryMap &entryMap, bool groupImmutable)
{
    for (auto &key : settings.childKeys()) {
        KEntryMap::EntryOptions entryOptions = KEntryMap::EntryDefault;
        const auto value = settings.value(key).toString();
        if (key.endsWith(QStringLiteral("[$i]"))) {
            key.chop(4);
            entryOptions |= KEntryMap::EntryImmutable;
        }
        if (groupImmutable) {
            entryOptions |= KEntryMap::EntryImmutable;
        }
        key = key.replace(QLatin1Char('\\'), QLatin1Char('/'));
        if (entryMap.getEntryOption(groupName, key.toUtf8(), KEntryMap::SearchDefaults, KEntryMap::EntryImmutable)) {
            continue;
        }
        entryMap.setEntry(groupName, key.toUtf8(), value.toUtf8(), entryOptions);
    }
}

void parseRegSubkeys(const QString &baseGroup, QSettings &settings, KEntryMap &entryMap, bool immutable)
{
    parseRegValues(baseGroup, settings, entryMap, immutable);

    for (auto &group : settings.childGroups()) {
        bool groupImmutable = immutable;
        QString groupWithoutSuffix = group;
        if (group.endsWith(QStringLiteral("[$i]"))) {
            groupWithoutSuffix.chop(4);
            groupImmutable = true;
        }
        settings.beginGroup(group);
        QString groupName = (baseGroup == QStringLiteral("<default>") ? QString() : (baseGroup + QLatin1Char('\x1d'))) + groupWithoutSuffix;
        parseRegSubkeys(groupName, settings, entryMap, groupImmutable);
        settings.endGroup();
        entryMap.setEntry(groupName, {}, QByteArray(), groupImmutable ? KEntryMap::EntryImmutable : KEntryMap::EntryOptions());
    }
}

void parseRegistry(const QString &regKey, KEntryMap &entryMap, Registry registry)
{
    QString registryPath = (registry == HKCU ? QStringLiteral("HKEY_CURRENT_USER\\") : QStringLiteral("HKEY_LOCAL_MACHINE\\")) + regKey;
    QSettings settings(registryPath, QSettings::NativeFormat);
    parseRegSubkeys(QStringLiteral("<default>"), settings, entryMap, false);
}

void WindowsRegistry::parse(const QString &regKey, KEntryMap &entryMap)
{
    parseRegistry(regKey, entryMap, HKLM);
    parseRegistry(regKey, entryMap, HKCU);
}
