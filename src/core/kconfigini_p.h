/*
   This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
   SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
   SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGINI_P_H
#define KCONFIGINI_P_H

#include <kconfigbase.h>
#include <kconfigcore_export.h>
#include <kconfigdata_p.h>

#include <QFile>

class QLockFile;
class QIODevice;

class KConfigIni : public QSharedData
{
    Q_GADGET
private:
    QLockFile *lockFile;
    QString localFileName;

public:
    class BufferFragment;

    static QExplicitlySharedDataPointer<KConfigIni> create(const QString &file);

    KConfigIni();
    /** Allows the behaviour of parseConfig() to be tuned */
    enum ParseOption {
        ParseGlobal = 1, /// entries should be marked as @em global
        ParseDefaults = 2, /// entries should be marked as @em default
        ParseExpansions = 4, /// entries are allowed to be marked as @em expandable
    };
    Q_FLAG(ParseOption)
    /// @typedef typedef QFlags<ParseOption> ParseOptions
    Q_DECLARE_FLAGS(ParseOptions, ParseOption)

    /** Allows the behaviour of writeConfig() to be tuned */
    enum WriteOption {
        WriteGlobal = 1 /// only write entries marked as "global"
    };
    Q_FLAG(WriteOption)
    /// @typedef typedef QFlags<WriteOption> WriteOptions
    Q_DECLARE_FLAGS(WriteOptions, WriteOption)

    /** Return value from parseConfig() */
    enum ParseInfo {
        ParseOk, /// the configuration was opened read/write
        ParseImmutable, /// the configuration is @em immutable
        ParseOpenError, /// the configuration could not be opened
    };

    /**
     * Read persistent storage
     *
     * @param locale the locale to read entries for (if the backend supports localized entries)
     * @param pWriteBackMap the KEntryMap where the entries are placed
     * @param options See ParseOptions
     * @return See ParseInfo
     */
    ParseInfo parseConfig(const QByteArray &locale, KEntryMap &entryMap, ParseOptions options);
    ParseInfo parseConfig(const QByteArray &locale, KEntryMap &entryMap, ParseOptions options, bool merging);

    /**
     * Write the @em dirty entries to permanent storage
     *
     * @param locale the locale to write entries for (if the backend supports localized entries)
     * @param entryMap the KEntryMap containing the config object's entries.
     * @param options See WriteOptions
     *
     * @return @c true if the write was successful, @c false if writing the configuration failed
     */
    bool writeConfig(const QByteArray &locale, KEntryMap &entryMap, WriteOptions options);

    /**
     * If isWritable() returns false, writeConfig() will always fail.
     *
     * @return @c true if the configuration is writable, @c false if it is immutable
     */
    bool isWritable() const;

    /**
     * When isWritable() returns @c false, return an error message to
     * explain to the user why saving configuration will not work.
     *
     * The return value when isWritable() returns @c true is undefined.
     *
     * @returns a translated user-visible explanation for the configuration
     *          object not being writable
     */
    QString nonWritableErrorMessage() const;

    /**
     * @return the read/write status of the configuration object
     *
     * @see KConfigBase::AccessMode
     */
    KConfigBase::AccessMode accessMode() const;

    /**
     * Create the enclosing object of the configuration object
     *
     * For example, if the configuration object is a file, this should create
     * the parent directory.
     */
    void createEnclosing();

    /**
     * Set the file path.
     *
     * @note @p path @b MUST be @em absolute.
     *
     * @param path the absolute file path
     */
    void setFilePath(const QString &path);

    /**
     * Lock the file
     */
    bool lock();

    /**
     * Release the lock on the file
     */
    void unlock();

    /**
     * @return @c true if the file is locked, @c false if it is not locked
     */
    bool isLocked() const;

    /**
     * @return the absolute path to the object
     */
    QString filePath() const;

protected:
    enum StringType {
        GroupString = 0,
        KeyString = 1,
        ValueString = 2,
    };
    // Warning: this modifies data in-place. Other BufferFragment objects referencing the same buffer
    // fragment will get their data modified too.
    static void printableToString(BufferFragment *aString, const QFile &file, int line);
    static QByteArray stringToPrintable(const QByteArray &aString, StringType type);
    static char charFromHex(const char *str, const QFile &file, int line);
    static QString warningProlog(const QFile &file, int line);

    void writeEntries(const QByteArray &locale, QIODevice &file, const KEntryMap &map);
    void writeEntries(const QByteArray &locale, QIODevice &file, const KEntryMap &map, bool defaultGroup, bool &firstEntry);

    void setLocalFilePath(const QString &file);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KConfigIni::ParseOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(KConfigIni::WriteOptions)

#endif // KCONFIGINI_P_H
