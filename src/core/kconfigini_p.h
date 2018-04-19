/*
   This file is part of the KDE libraries
   Copyright (c) 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Portions copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KCONFIGINI_P_H
#define KCONFIGINI_P_H

#include <kconfigcore_export.h>
#include <kconfigbackend_p.h>

class QLockFile;
class QIODevice;

class KConfigIniBackend : public KConfigBackend
{
Q_OBJECT
private:
    QLockFile *lockFile;

public:
    class BufferFragment;

    KConfigIniBackend();
    ~KConfigIniBackend() Q_DECL_OVERRIDE;

    ParseInfo parseConfig(const QByteArray &locale,
                          KEntryMap &entryMap,
                          ParseOptions options) Q_DECL_OVERRIDE;
    ParseInfo parseConfig(const QByteArray &locale,
                          KEntryMap &entryMap,
                          ParseOptions options,
                          bool merging);
    bool writeConfig(const QByteArray &locale, KEntryMap &entryMap,
                     WriteOptions options) Q_DECL_OVERRIDE;

    bool isWritable() const Q_DECL_OVERRIDE;
    QString nonWritableErrorMessage() const Q_DECL_OVERRIDE;
    KConfigBase::AccessMode accessMode() const Q_DECL_OVERRIDE;
    void createEnclosing() Q_DECL_OVERRIDE;
    void setFilePath(const QString &path) Q_DECL_OVERRIDE;
    bool lock() Q_DECL_OVERRIDE;
    void unlock() Q_DECL_OVERRIDE;
    bool isLocked() const Q_DECL_OVERRIDE;

protected:

    enum StringType {
        GroupString = 0,
        KeyString = 1,
        ValueString = 2
    };
    // Warning: this modifies data in-place. Other BufferFragment objects referencing the same buffer
    // fragment will get their data modified too.
    static void printableToString(BufferFragment *aString, const QFile &file, int line);
    static QByteArray stringToPrintable(const QByteArray &aString, StringType type);
    static char charFromHex(const char *str, const QFile &file, int line);
    static QString warningProlog(const QFile &file, int line);

    void writeEntries(const QByteArray &locale, QIODevice &file, const KEntryMap &map);
    void writeEntries(const QByteArray &locale, QIODevice &file, const KEntryMap &map,
                      bool defaultGroup, bool &firstEntry);
};

#endif // KCONFIGINI_P_H
