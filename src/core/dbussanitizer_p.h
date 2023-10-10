// SPDX-License-Identifier: LGPL-2.0-or-later
// SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

#pragma once

#include <cctype>
#include <locale>

#include <QString>

inline QString kconfigDBusSanitizePath(QString path)
{
    for (auto &character : path) {
        if ((std::isalnum(character.toLatin1(), std::locale::classic()) == 0) && character != QLatin1Char('_') && character != QLatin1Char('/')) {
            character = QLatin1Char('_');
        }
    }
    Q_ASSERT(path.length() > 1);
    Q_ASSERT(path.at(0) == QLatin1Char('/'));
    // The spec disallows '//' but QtDBus cleans the path up for us
    return path;
}
