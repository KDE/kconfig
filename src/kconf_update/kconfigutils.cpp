/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Canonical Ltd
    SPDX-FileContributor: Aurélien Gâteau <aurelien.gateau@canonical.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kconfigutils.h"

// KDE
#include <kconfig.h>
#include <kconfiggroup.h>

namespace KConfigUtils
{

bool hasGroup(KConfig *config, const QStringList &lst)
{
    KConfigGroup group = openGroup(config, lst);
    return group.exists();
}

KConfigGroup openGroup(KConfig *config, const QStringList &lst)
{
    KConfigGroup cg = config->group("");

    for (const auto &i : lst) {
        cg = cg.group(i);
    }
    return cg;
}

QStringList parseGroupString(const QString &_str, bool *ok, QString *error)
{
    QString str = unescapeString(_str.trimmed(), ok, error);
    if (!*ok) {
        return QStringList();
    }

    *ok = true;
    if (str[0] != '[') {
        // Simplified notation, no '['
        return QStringList() << str;
    }

    if (!str.endsWith(']')) {
        *ok = false;
        *error = QStringLiteral("Missing closing ']' in %1").arg(_str);
        return QStringList();
    }
    // trim outer brackets
    str.chop(1);
    str.remove(0, 1);

    return str.split(QStringLiteral("]["));
}

QString unescapeString(const QString &src, bool *ok, QString *error)
{
    QString dst;
    int length = src.length();
    for (int pos = 0; pos < length; ++pos) {
        QChar ch = src.at(pos);
        if (ch != '\\') {
            dst += ch;
        } else {
            ++pos;
            if (pos == length) {
                *ok = false;
                *error = QStringLiteral("Unfinished escape sequence in %1").arg(src);
                return QString();
            }
            ch = src.at(pos);
            if (ch == 's') {
                dst += ' ';
            } else if (ch == 't') {
                dst += '\t';
            } else if (ch == 'n') {
                dst += '\n';
            } else if (ch == 'r') {
                dst += '\r';
            } else if (ch == '\\') {
                dst += '\\';
            } else if (ch == 'x') {
                if (pos + 2 < length) {
                    char value = src.midRef(pos + 1, 2).toInt(ok, 16);
                    if (*ok) {
                        dst += QChar::fromLatin1(value);
                        pos += 2;
                    } else {
                        *error = QStringLiteral("Invalid hex escape sequence at column %1 in %2").arg(pos).arg(src);
                        return QString();
                    }
                } else {
                    *ok = false;
                    *error = QStringLiteral("Unfinished hex escape sequence at column %1 in %2").arg(pos).arg(src);
                    return QString();
                }
            } else {
                *ok = false;
                *error = QStringLiteral("Invalid escape sequence at column %1 in %2").arg(pos).arg(src);
                return QString();
            }
        }
    }

    *ok = true;
    return dst;
}

} // namespace
