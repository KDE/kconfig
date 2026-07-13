// SPDX-License-Identifier: LGPL-2.0-or-later
// SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

#pragma once

#include <QChar>
#include <QLatin1Char>
#include <QString>

inline QString kconfigDBusSanitizePath(QString path)
{
    for (auto &c : path) {
        // Quote DBus spec: 'Each element must only contain the ASCII characters "[A-Z][a-z][0-9]_"'
        // (and '/' is the element separator)
        const bool isAscii = c.unicode() < 128; // note: char16_t is always unsigned
        if (!isAscii || (!c.isLetterOrNumber() && c != QLatin1Char('_') && c != QLatin1Char('/'))) {
            c = QLatin1Char('_');
        }
    }
    // KConfig notifying or watching on / makes no sense
    Q_ASSERT_X(path.length() > 1, Q_FUNC_INFO, qUtf8Printable(path));
    // As per spec the path must start with a slash
    Q_ASSERT_X(path.at(0) == QLatin1Char('/'), Q_FUNC_INFO, qUtf8Printable(path));
    // ...but not end with a slash
    Q_ASSERT_X(path.at(path.size() - 1) != QLatin1Char('/'), Q_FUNC_INFO, qUtf8Printable(path));
    // ...it also must not contain multiple slashes in sequence
    Q_ASSERT_X(!path.contains(QLatin1String("//")), Q_FUNC_INFO, qUtf8Printable(path));
    return path;
}
