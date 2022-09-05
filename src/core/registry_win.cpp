/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2022 g10 Code GmbH
    SPDX-FileContributor: Andre Heinecke <aheinecke@gnupg.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "registry_win_p.h"
#include "kconfig_core_log_settings.h"

#include <windows.h>

#define RRF_SUBKEY_WOW6464KEY 0x00010000

QString readRegString (HKEY key, const wchar_t *name)
{
    DWORD nbytes = 0;
    DWORD err = RegGetValueW(key,
                             nullptr,
                             name,
                             RRF_RT_REG_EXPAND_SZ | RRF_RT_REG_SZ,
                             nullptr,
                             nullptr,
                             &nbytes);
    if (err != ERROR_SUCCESS) {
        qCWarning(KCONFIG_CORE_LOG) << "Failed to get size of value:" << std::wstring(name) << err;
        return QString();
    }
    std::vector<wchar_t> result(nbytes / sizeof (wchar_t));

    err = RegGetValueW(key,
                       nullptr,
                       name,
                       RRF_RT_REG_EXPAND_SZ | RRF_RT_REG_SZ,
                       nullptr,
                       &result[0],
                       &nbytes);
    if (err != ERROR_SUCCESS) {
        qCWarning(KCONFIG_CORE_LOG) << "Invalid type of value:" << name << err;
        return QString();
    }
    return QString::fromWCharArray(&result[0]);
}

void parseRegValues(const QString &regKey, const QString &groupName, KEntryMap &entryMap, bool userRegistry)
{
    const bool groupOptionImmutable = regKey.endsWith(QStringLiteral("[$i]"));
    HKEY key;
    if (RegOpenKeyExW(userRegistry ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE,
                      (wchar_t *) regKey.utf16(),
                      0, KEY_ENUMERATE_SUB_KEYS | KEY_READ,
                      &key) != ERROR_SUCCESS) {
        return;
    }
    DWORD values = 0,
          maxNameLen = 0;

    DWORD err = RegQueryInfoKey (key,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 &values,
                                 &maxNameLen,
                                 nullptr,
                                 nullptr,
                                 nullptr);

    if (err != ERROR_SUCCESS) {
        qCWarning(KCONFIG_CORE_LOG) << "Failed to query key info for" << regKey;
        RegCloseKey(key);
        return;
    }

    maxNameLen++;
    std::vector<wchar_t> buf(maxNameLen + 1);
    wchar_t *name = &buf[0];
    for (DWORD i = 0; i < values; i++) {
        DWORD nameLen = maxNameLen;
        err = RegEnumValueW(key, i,
                            name,
                            &nameLen,
                            nullptr,
                            nullptr,
                            nullptr,
                            nullptr);

        if (err != ERROR_SUCCESS) {
            qCWarning(KCONFIG_CORE_LOG) << "Failed to enum value " << i << "of" << regKey << groupName << *name << "err:" << err << values << maxNameLen << nameLen;
            continue;
        }

        KEntryMap::EntryOptions entryOptions = KEntryMap::EntryDefault;
        QString entryName = QString::fromWCharArray(name, nameLen);

        if (entryName.endsWith(QStringLiteral("[$i]"))) {
            entryOptions |= KEntryMap::EntryImmutable;
            entryName.chop(4);
        }

        if (entryMap.getEntryOption(groupName.toUtf8(), entryName.toUtf8(), KEntryMap::SearchDefaults, KEntryMap::EntryImmutable)) {
            continue;
        }
        const auto value = readRegString(key, name);
        if (groupOptionImmutable) {
            entryOptions |= KEntryMap::EntryImmutable;
        }
        entryMap.setEntry(groupName.toUtf8(), entryName.toUtf8(), value.toUtf8(), entryOptions);
        // qDebug () << "Adding Entry" << entryName << value << "to group" << groupName << "namelen " << nameLen;
    }
    RegCloseKey(key);
}

void parseRegSubkeys(const QString &regKey, KEntryMap &entryMap, bool userRegistry)
{
    // Parse default group
    parseRegValues(regKey, QStringLiteral("<default>"), entryMap, userRegistry);

    // Enumerate the subkeys (groups)
    HKEY key;
    if (RegOpenKeyEx(userRegistry ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE,
                     (wchar_t *)regKey.utf16(),
                     0, KEY_ENUMERATE_SUB_KEYS | KEY_READ,
                     &key) != ERROR_SUCCESS) {
        return;
    }

    DWORD subKeys = 0,
          maxSubKeyLen = 0;
    DWORD err = RegQueryInfoKey(key,
                                nullptr,
                                nullptr,
                                nullptr,
                                &subKeys,
                                &maxSubKeyLen,
                                nullptr,
                                nullptr,
                                nullptr,
                                nullptr,
                                nullptr,
                                nullptr);

    if (err != ERROR_SUCCESS) {
        qCWarning(KCONFIG_CORE_LOG) << "Failed to query key info for" << regKey << "err:" << err;
        RegCloseKey(key);
        return;
    }

    maxSubKeyLen++;
    wchar_t *subKey = new wchar_t[maxSubKeyLen + 1];
    for (DWORD i = 0; i < subKeys; i++) {
        DWORD keyLen = maxSubKeyLen;
        err = RegEnumKeyEx(key, i,
                           subKey,
                           &keyLen,
                           nullptr,
                           nullptr,
                           nullptr,
                           nullptr);
        if (err != ERROR_SUCCESS) {
            qCWarning(KCONFIG_CORE_LOG) << "Failed to enum key" << i << "err:" << err;
            continue;
        }
        QString subKeyName = QString::fromWCharArray(subKey, keyLen);
        QString subReg = regKey + QLatin1Char('\\') + subKeyName;
        if (subKeyName.endsWith(QStringLiteral("[$i]"))) {
            subKeyName.chop(4);
        }
        parseRegValues (subReg, subKeyName, entryMap, userRegistry);
    }
    delete[] subKey;
    RegCloseKey(key);
}

void parseWindowsRegistry(const QString &regKey, KEntryMap &entryMap)
{
    // First take the HKLM values into account
    parseRegSubkeys(regKey, entryMap, false);
    // Then HKCU
    parseRegSubkeys(regKey, entryMap, true);
}
