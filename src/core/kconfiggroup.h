/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>
    SPDX-FileCopyrightText: 2001 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGGROUP_H
#define KCONFIGGROUP_H

#include "kconfigbase.h"

#include <kconfigcore_export.h>

#include <QExplicitlySharedDataPointer>
#include <QStringList>
#include <QVariant>

class KConfig;
class KConfigGroupPrivate;
class KSharedConfig;

/*!
 * \class KConfigGroup
 * \inmodule KConfigCore
 *
 * \brief A class for one specific group in a KConfig object.
 *
 * If you want to access the top-level entries of a KConfig
 * object, which are not associated with any group, use an
 * empty group name.
 *
 * A KConfigGroup will be read-only if it is constructed from a
 * const config object or from another read-only group.
 */
class KCONFIGCORE_EXPORT KConfigGroup : public KConfigBase
{
public:
    /*!
     * Constructs an invalid group.
     *
     * \sa isValid
     */
    KConfigGroup();

    /*!
     * Construct a config group corresponding to \a group in \a master.
     *
     * This allows the creation of subgroups by passing another
     * group as \a master.
     *
     * \a group name of group
     */
    KConfigGroup(KConfigBase *master, const QString &group);

    /*!
     * Construct a read-only config group.
     *
     * A read-only group will silently ignore any attempts to write to it.
     *
     * This allows the creation of subgroups by passing an existing group
     * as \a master.
     */
    KConfigGroup(const KConfigBase *master, const QString &group);

    /*!
     * \overload
     * Overload for KConfigGroup(const KConfigBase*,const QString&)
     */
    KConfigGroup(const QExplicitlySharedDataPointer<KSharedConfig> &master, const QString &group);

    KConfigGroup(const KConfigGroup &);
    KConfigGroup &operator=(const KConfigGroup &);

    ~KConfigGroup() override;

    /*!
     * Whether the group is valid.
     *
     * A group is invalid if it was constructed without arguments.
     *
     * You should not call any functions on an invalid group.
     *
     * Returns \c true if the group is valid, \c false if it is invalid.
     */
    bool isValid() const;

    /*!
     * The name of this group.
     *
     * The root group is named "<default>".
     */
    QString name() const;

    /*!
     * Check whether the containing KConfig object actually contains a
     * group with this name.
     */
    bool exists() const;

    /*!
     * \reimp
     *
     * Syncs the parent config.
     */
    bool sync() override;

    void markAsClean() override;

    AccessMode accessMode() const override;

    /*!
     * Return the config object that this group belongs to
     */
    KConfig *config();
    /*!
     * Return the config object that this group belongs to
     */
    const KConfig *config() const;

    /*!
     * Copies the entries in this group to another configuration object
     *
     * \note \a other can be either another group or a different file.
     *
     * \a other  the configuration object to copy this group's entries to
     *
     * \a pFlags the flags to use when writing the entries to the
     *               other configuration object
     *
     * \since 4.1
     */
    void copyTo(KConfigBase *other, WriteConfigFlags pFlags = Normal) const;

    /*!
     * Changes the configuration object that this group belongs to
     *
     * If \a parent is already the parent of this group, this method will have
     * no effect.
     *
     * \a parent the config object to place this group under
     *
     * \a pFlags the flags to use in determining which storage source to
     *               write the data to
     *
     * \since 4.1
     */
    void reparent(KConfigBase *parent, WriteConfigFlags pFlags = Normal);

    /*!
     * Moves the key-value pairs from one config group to the other.
     * In case the entries do not exist the key is ignored.
     *
     * \since 5.88
     */
    void moveValuesTo(const QList<const char *> &keys, KConfigGroup &other, WriteConfigFlags pFlags = Normal);

    /*!
     * Moves the key-value pairs from one config group to the other.
     *
     * \since 6.3
     */
    void moveValuesTo(KConfigGroup &other, WriteConfigFlags pFlags = Normal);

    /*!
     * Returns the group that this group belongs to
     *
     * Returns the parent group, or an invalid group if this is a top-level
     *          group
     *
     * \since 4.1
     */
    KConfigGroup parent() const;

    QStringList groupList() const override;

    /*!
     * Returns a list of keys this group contains
     */
    QStringList keyList() const;

    /*!
     * Delete all entries in the entire group
     *
     * \a pFlags flags passed to KConfig::deleteGroup
     *
     * \sa deleteEntry()
     */
    void deleteGroup(WriteConfigFlags pFlags = Normal);
    using KConfigBase::deleteGroup;

    /*!
     * Reads the value of an entry specified by \a pKey in the current group
     *
     * This template method makes it possible to write
     *    QString foo = readEntry("...", QString("default"));
     * and the same with all other types supported by QVariant.
     *
     * The return type of the method is simply the same as the type of the default value.
     *
     * \note readEntry("...", Qt::white) will not compile because Qt::white is an enum.
     * You must turn it into readEntry("...", QColor(Qt::white)).
     *
     * \note Only the following QVariant types are allowed : String,
     * StringList, List, Font, Point, PointF, Rect, RectF, Size, SizeF, Color, Int, UInt, Bool,
     * Double, LongLong, ULongLong, DateTime and Date.
     *
     * \a key The key to search for
     *
     * \a aDefault A default value returned if the key was not found
     *
     * Returns the value for this key, or \a aDefault.
     *
     * \sa writeEntry(), deleteEntry(), hasKey()
     */
    template<typename T>
    T readEntry(const QString &key, const T &aDefault) const
    {
        return readEntry(key.toUtf8().constData(), aDefault);
    }
    /*!
     * \overload
     *
     * Overload for readEntry<T>(const QString&, const T&) const
     *
     * \a key name of key, encoded in UTF-8
     */
    template<typename T>
    T readEntry(const char *key, const T &aDefault) const;

    /*!
     * Reads the value of an entry specified by \a key in the current group
     *
     * \a key the key to search for
     *
     * \a aDefault a default value returned if the key was not found
     *
     * Returns the value for this key, or \a aDefault if the key was not found
     *
     * \sa writeEntry(), deleteEntry(), hasKey()
     */
    QVariant readEntry(const QString &key, const QVariant &aDefault) const;
    /*!
     * Overload for readEntry(const QString&, const QVariant&) const
     * \a key name of key, encoded in UTF-8
     */
    QVariant readEntry(const char *key, const QVariant &aDefault) const;

    /*!
     * Reads the string value of an entry specified by \a key in the current group
     *
     * If you want to read a path, please use readPathEntry().
     *
     * \a key the key to search for
     *
     * \a aDefault a default value returned if the key was not found
     *
     * Returns the value for this key, or \a aDefault if the key was not found
     *
     * \sa readPathEntry(), writeEntry(), deleteEntry(), hasKey()
     */
    QString readEntry(const QString &key, const QString &aDefault) const;
    /*!
     * \overload
     *
     * Overload for readEntry(const QString&, const QString&) const
     *
     * \a key name of key, encoded in UTF-8
     */
    QString readEntry(const char *key, const QString &aDefault) const;

    /*!
     * \overload
     *
     * Overload for readEntry(const QString&, const QString&) const
     */
    QString readEntry(const QString &key, const char *aDefault = nullptr) const;
    /*!
     * \overload
     *
     * Overload for readEntry(const QString&, const QString&) const
     *
     * \a key name of key, encoded in UTF-8
     */
    QString readEntry(const char *key, const char *aDefault = nullptr) const;

    /*!
     * \sa readEntry()
     *
     * \warning This function doesn't convert the items returned
     *          to any type. It's actually a list of QVariant::String's. If you
     *          want the items converted to a specific type use
     *          readEntry(const char*, const QList<T>&) const
     */
    QVariantList readEntry(const QString &key, const QVariantList &aDefault) const;
    /*!
     * \overload
     *
     * Overload for readEntry(const QString&, const QVariantList&) const
     *
     * \a key name of key, encoded in UTF-8
     */
    QVariantList readEntry(const char *key, const QVariantList &aDefault) const;

    /*!
     * Reads a list of strings from the config object
     *
     * \a key The key to search for
     *
     * \a aDefault The default value to use if the key does not exist
     *
     * Returns the list, or \a aDefault if \a key does not exist
     *
     * \sa readXdgListEntry(), writeEntry(), deleteEntry(), hasKey()
     */
    QStringList readEntry(const QString &key, const QStringList &aDefault) const;
    /*!
     * \overload
     *
     * Overload for readEntry(const QString&, const QStringList&) const
     *
     * \a key name of key, encoded in UTF-8
     */
    QStringList readEntry(const char *key, const QStringList &aDefault) const;

    /*!
     * Reads a list of values from the config object
     *
     * \a key the key to search for
     *
     * \a aDefault the default value to use if the key does not exist
     *
     * Returns the list, or \a aDefault if \a key does not exist
     *
     * \sa readXdgListEntry(), writeEntry(), deleteEntry(), hasKey()
     */
    template<typename T>
    QList<T> readEntry(const QString &key, const QList<T> &aDefault) const
    {
        return readEntry(key.toUtf8().constData(), aDefault);
    }
    /*!
     * \overload
     *
     * Overload for readEntry<T>(const QString&, const QList<T>&) const
     *
     * \a key name of key, encoded in UTF-8
     */
    template<typename T>
    QList<T> readEntry(const char *key, const QList<T> &aDefault) const;

    /*!
     * \overload
     *
     * Overload for readEntry(const QString&, std::chrono::duration<Rep, Period>)
     *
     * \a key name of key, encoded in UTF-8
     *
     * \since 6.22
     */
    template<typename Rep, typename Period>
    std::chrono::duration<Rep, Period> readEntry(const char *key, std::chrono::duration<Rep, Period> defaultValue) const;

    /*!
     * Reads a std::chrono duration from the config object with the given \a key. If the config
     * contains no specified \a key, the \a value will be returned instead.
     *
     * \since 6.22
     */
    template<typename Rep, typename Period>
    std::chrono::duration<Rep, Period> readEntry(const QString &key, std::chrono::duration<Rep, Period> defaultValue) const;

    /*!
     * Reads a list of strings from the config object with semicolons separating
     * them (i.e. following desktop entry spec separator semantics).
     *
     * \a pKey the key to search for
     *
     * \a aDefault the default value to use if the key does not exist
     *
     * Returns the list, or \a aDefault if \a pKey does not exist
     *
     * \sa readEntry(const QString&, const QStringList&)
     */
    QStringList readXdgListEntry(const QString &pKey, const QStringList &aDefault = QStringList()) const;
    /*!
     * Overload for readXdgListEntry(const QString&, const QStringList&) const
     *
     * \a key name of key, encoded in UTF-8
     */
    QStringList readXdgListEntry(const char *key, const QStringList &aDefault = QStringList()) const;

    /*!
     * Reads a path
     *
     * Read the value of an entry specified by \a pKey in the current group
     * and interpret it as a path. This means, dollar expansion is activated
     * for this value, so that e.g. $HOME gets expanded.
     *
     * \a pKey The key to search for.
     *
     * \a aDefault A default value returned if the key was not found.
     *
     * Returns The value for this key. Can be QString() if \a aDefault is null.
     */
    QString readPathEntry(const QString &pKey, const QString &aDefault) const;
    /*!
     * Overload for readPathEntry(const QString&, const QString&) const
     *
     * \a key name of key, encoded in UTF-8
     */
    QString readPathEntry(const char *key, const QString &aDefault) const;

    /*!
     * Reads a list of paths
     *
     * Read the value of an entry specified by \a pKey in the current group
     * and interpret it as a list of paths. This means, dollar expansion is activated
     * for this value, so that e.g. $HOME gets expanded.
     *
     * \a pKey the key to search for
     *
     * \a aDefault a default value returned if the key was not found
     *
     * Returns the list, or \a aDefault if the key does not exist
     */
    QStringList readPathEntry(const QString &pKey, const QStringList &aDefault) const;
    /*!
     * Overload for readPathEntry(const QString&, const QStringList&) const
     *
     * \a key name of key, encoded in UTF-8
     */
    QStringList readPathEntry(const char *key, const QStringList &aDefault) const;

    /*!
     * Reads an untranslated string entry
     *
     * You should not normally need to use this.
     *
     * \a pKey the key to search for
     *
     * \a aDefault a default value returned if the key was not found
     *
     * Returns the value for this key, or \a aDefault if the key does not exist
     */
    QString readEntryUntranslated(const QString &pKey, const QString &aDefault = QString()) const;
    /*!
     * \overload
     *
     * Overload for readEntryUntranslated(const QString&, const QString&) const
     *
     * \a key name of key, encoded in UTF-8
     */
    QString readEntryUntranslated(const char *key, const QString &aDefault = QString()) const;

    /*!
     * Writes a value to the configuration object.
     *
     * \a key the key to write to
     *
     * \a value the value to write
     *
     * \a pFlags the flags to use when writing this entry
     *
     * \sa readEntry(), writeXdgListEntry(), deleteEntry()
     */
    void writeEntry(const QString &key, const QVariant &value, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload
     *
     * Overload for writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     * \a key name of key, encoded in UTF-8
     */
    void writeEntry(const char *key, const QVariant &value, WriteConfigFlags pFlags = Normal);

    /*!
     * \overload
     *
     * Overload for writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     */
    void writeEntry(const QString &key, const QString &value, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload
     *
     * Overload for writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     *
     * \a key name of key, encoded in UTF-8
     */
    void writeEntry(const char *key, const QString &value, WriteConfigFlags pFlags = Normal);

    /*!
     * \overload
     *
     * Overload for writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     */
    void writeEntry(const QString &key, const QByteArray &value, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload
     *
     * Overload for writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     *
     * \a key name of key, encoded in UTF-8
     */
    void writeEntry(const char *key, const QByteArray &value, WriteConfigFlags pFlags = Normal);

    /*!
     * \overload
     *
     * Overload for writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     */
    void writeEntry(const QString &key, const char *value, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload
     *
     * Overload for writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     *
     * \a key name of key, encoded in UTF-8
     */
    void writeEntry(const char *key, const char *value, WriteConfigFlags pFlags = Normal);

    /*!
     * \overload
     *
     * Overload for writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     *
     * \a key name of key, encoded in UTF-8
     */
    template<typename T>
    void writeEntry(const char *key, const T &value, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload
     *
     * Overload for writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     */
    template<typename T>
    void writeEntry(const QString &key, const T &value, WriteConfigFlags pFlags = Normal)
    {
        writeEntry(key.toUtf8().constData(), value, pFlags);
    }

    /*!
     * \overload
     *
     * Overload for writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     */
    void writeEntry(const QString &key, const QStringList &value, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload
     *
     * Overload for writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     *
     * \a key name of key, encoded in UTF-8
     */
    void writeEntry(const char *key, const QStringList &value, WriteConfigFlags pFlags = Normal);

    /*!
     * \overload
     *
     * Overload for writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     */
    void writeEntry(const QString &key, const QVariantList &value, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload
     *
     * Overload for writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     *
     * \a key name of key, encoded in UTF-8
     */
    void writeEntry(const char *key, const QVariantList &value, WriteConfigFlags pFlags = Normal);

    /*!
     * \overload
     *
     * Overload for writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     */
    template<typename T>
    void writeEntry(const QString &key, const QList<T> &value, WriteConfigFlags pFlags = Normal)
    {
        writeEntry(key.toUtf8().constData(), value, pFlags);
    }
    /*!
     * \overload
     *
     * Overload for writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     *
     * \a key name of key, encoded in UTF-8
     */
    template<typename T>
    void writeEntry(const char *key, const QList<T> &value, WriteConfigFlags pFlags = Normal);

    /*!
     * \overload
     *
     * Overload for writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     *
     * \since 6.22
     */
    template<typename Rep, typename Period>
    void writeEntry(const QString &key, std::chrono::duration<Rep, Period> value, WriteConfigFlags pFlags = Normal);

    /*!
     * \overload
     * Overload for writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     *
     * \a key name of key, encoded in UTF-8
     *
     * \since 6.22
     */
    template<typename Rep, typename Period>
    void writeEntry(const char *key, std::chrono::duration<Rep, Period> value, WriteConfigFlags pFlags = Normal);

    /*!
     * Writes a list of strings to the config object, following XDG
     * desktop entry spec separator semantics
     *
     * \a pKey the key to write to
     *
     * \a value the list to write
     *
     * \a pFlags the flags to use when writing this entry
     *
     * \sa writeEntry(), readXdgListEntry()
     */
    void writeXdgListEntry(const QString &pKey, const QStringList &value, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload
     *
     * Overload for writeXdgListEntry(const QString&, const QStringList&, WriteConfigFlags)
     *
     * \a key name of key, encoded in UTF-8
     */
    void writeXdgListEntry(const char *key, const QStringList &value, WriteConfigFlags pFlags = Normal);

    /*!
     * Writes a file path to the configuration
     *
     * If the path is located under $HOME, the user's home directory
     * is replaced with $HOME in the persistent storage.
     * The path should therefore be read back with readPathEntry()
     *
     * \a pKey the key to write to
     *
     * \a path the path to write
     *
     * \a pFlags the flags to use when writing this entry
     *
     * \sa writeEntry(), readPathEntry()
     */
    void writePathEntry(const QString &pKey, const QString &path, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload
     *
     * Overload for writePathEntry(const QString&, const QString&, WriteConfigFlags)
     *
     * \a key name of key, encoded in UTF-8
     */
    void writePathEntry(const char *Key, const QString &path, WriteConfigFlags pFlags = Normal);

    /*!
     * Writes a list of paths to the configuration
     *
     * If any of the paths are located under $HOME, the user's home directory
     * is replaced with $HOME in the persistent storage.
     * The paths should therefore be read back with readPathEntry()
     *
     * \a pKey the key to write to
     *
     * \a value the list to write
     *
     * \a pFlags the flags to use when writing this entry
     *
     * \sa writeEntry(), readPathEntry()
     */
    void writePathEntry(const QString &pKey, const QStringList &value, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload
     *
     * Overload for writePathEntry(const QString&, const QStringList&, WriteConfigFlags)
     *
     * \a key name of key, encoded in UTF-8
     */
    void writePathEntry(const char *key, const QStringList &value, WriteConfigFlags pFlags = Normal);

    /*!
     * Deletes the entry specified by \a pKey in the current group
     *
     * This also hides system wide defaults.
     *
     * \a pKey the key to delete
     *
     * \a pFlags the flags to use when deleting this entry
     *
     * \sa deleteGroup(), readEntry(), writeEntry()
     */
    void deleteEntry(const QString &pKey, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload
     *
     * Overload for deleteEntry(const QString&, WriteConfigFlags)
     *
     * \a key name of key, encoded in UTF-8
     */
    void deleteEntry(const char *key, WriteConfigFlags pFlags = Normal);

    /*!
     * Checks whether the key has an entry in this group
     *
     * Use this to determine if a key is not specified for the current
     * group (hasKey() returns false).
     *
     * If this returns \c false for a key, readEntry() (and its variants)
     * will return the default value passed to them.
     *
     * \a key the key to search for
     *
     * Returns \c true if the key is defined in this group by any of the
     *         configuration sources, \c false otherwise
     *
     * \sa readEntry()
     */
    bool hasKey(const QString &key) const;
    /*!
     * \overload
     *
     * Overload for hasKey(const QString&) const
     *
     * \a key name of key, encoded in UTF-8
     */
    bool hasKey(const char *key) const;

    /*!
     * Whether this group may be changed
     *
     * Returns \c false if the group may be changed, \c true otherwise
     */
    bool isImmutable() const override;

    /*!
     * Checks if it is possible to change the given entry
     *
     * If isImmutable() returns \c true, then this method will return
     * \c true for all inputs.
     *
     * \a key the key to check
     *
     * Returns \c false if the key may be changed using this configuration
     *         group object, \c true otherwise
     */
    bool isEntryImmutable(const QString &key) const;
    /*!
     * \overload
     *
     * Overload for isEntryImmutable(const QString&) const
     *
     * \a key name of key, encoded in UTF-8
     */
    bool isEntryImmutable(const char *key) const;

    /*!
     * Reverts an entry to the default settings.
     *
     * Reverts the entry with key \a key in the current group in the
     * application specific config file to either the system wide (default)
     * value or the value specified in the global KDE config file.
     *
     * To revert entries in the global KDE config file, the global KDE config
     * file should be opened explicitly in a separate config object.
     *
     * \note This is not the same as deleting the key, as instead the
     * global setting will be copied to the configuration file that this
     * object manipulates.
     *
     * \a key The key of the entry to revert.
     */
    void revertToDefault(const QString &key, WriteConfigFlags pFlag = WriteConfigFlags());

    /*!
     * Overload for revertToDefault(const QString&, WriteConfigFlags)
     *
     * \a key name of key, encoded in UTF-8
     */
    void revertToDefault(const char *key, WriteConfigFlags pFlag = WriteConfigFlags());

    /*!
     * Whether a default is specified for an entry in either the
     * system wide configuration file or the global KDE config file
     *
     * If an application computes a default value at runtime for
     * a certain entry, e.g. like:
     * \code
     * QColor computedDefault = qApp->palette().color(QPalette::Active, QPalette::Text);
     * QColor color = group.readEntry(key, computedDefault);
     * \endcode
     * then it may wish to make the following check before
     * writing back changes:
     * \code
     * if ( (value == computedDefault) && !group.hasDefault(key) )
     *    group.revertToDefault(key);
     * else
     *    group.writeEntry(key, value);
     * \endcode
     *
     * This ensures that as long as the entry is not modified to differ from
     * the computed default, the application will keep using the computed default
     * and will follow changes the computed default makes over time.
     *
     * \a key the key of the entry to check
     *
     * Returns \c true if the global or system settings files specify a default
     *          for \a key in this group, \c false otherwise
     */
    bool hasDefault(const QString &key) const;
    /*!
     * \overload
     *
     * Overload for hasDefault(const QString&) const
     *
     * \a key name of key, encoded in UTF-8
     */
    bool hasDefault(const char *key) const;

    /*!
     * Returns a map (tree) of entries for all entries in this group
     *
     * Only the actual entry string is returned, none of the
     * other internal data should be included.
     *
     * Returns a map of entries in this group, indexed by key
     */
    QMap<QString, QString> entryMap() const;

protected:
    bool hasGroupImpl(const QString &groupName) const override;
    KConfigGroup groupImpl(const QString &groupName) override;
    const KConfigGroup groupImpl(const QString &groupName) const override;
    void deleteGroupImpl(const QString &groupName, WriteConfigFlags flags) override;
    bool isGroupImmutableImpl(const QString &groupName) const override;

private:
    QExplicitlySharedDataPointer<KConfigGroupPrivate> d;

    friend class KConfigGroupPrivate;

    /*!
     * \internal
     *
     * Return the data in \a value converted to a QVariant
     *
     * \a pKey the name of the entry being converted, this is only used for error
     * reporting
     *
     * \a value the UTF-8 data to be converted
     *
     * \a aDefault the default value if \a pKey is not found
     *
     * Returns \a value converted to QVariant, or \a aDefault if \a value is invalid or cannot be converted.
     */
    static QVariant convertToQVariant(const char *pKey, const QByteArray &value, const QVariant &aDefault);

    KCONFIGCORE_NO_EXPORT void moveValue(const char *key, KConfigGroup &other, WriteConfigFlags pFlags);

    // exported for usage by KServices' KService & KServiceAction
    friend class KServicePrivate; // XXX yeah, ugly^5
    friend class KServiceAction;
};

Q_DECLARE_TYPEINFO(KConfigGroup, Q_RELOCATABLE_TYPE);

#define KCONFIGGROUP_ENUMERATOR_ERROR(ENUM) "The Qt MetaObject system does not seem to know about \"" ENUM "\" please use Q_ENUM or Q_FLAG to register it."

/*!
 * \macro KCONFIGGROUP_DECLARE_ENUM_QOBJECT(Class, Enum)
 * \relates KConfigGroup
 * To add support for your own enums in KConfig, you can declare them with Q_ENUM()
 * in a QObject subclass (which will make moc generate the code to turn the
 * enum into a string and vice-versa), and then (in the cpp code)
 * use the macro
 * \code
 * KCONFIGGROUP_DECLARE_ENUM_QOBJECT(MyClass, MyEnum)
 * \endcode
 *
 */
#define KCONFIGGROUP_DECLARE_ENUM_QOBJECT(Class, Enum)                                                                                                         \
    template<>                                                                                                                                                 \
    Class::Enum KConfigGroup::readEntry(const char *key, const Class::Enum &def) const                                                                         \
    {                                                                                                                                                          \
        const QMetaObject *M_obj = &Class::staticMetaObject;                                                                                                   \
        const int M_index = M_obj->indexOfEnumerator(#Enum);                                                                                                   \
        if (M_index == -1)                                                                                                                                     \
            qFatal(KCONFIGGROUP_ENUMERATOR_ERROR(#Enum));                                                                                                      \
        const QMetaEnum M_enum = M_obj->enumerator(M_index);                                                                                                   \
        const QByteArray M_data = readEntry(key, QByteArray(M_enum.valueToKey(def)));                                                                          \
        return static_cast<Class::Enum>(M_enum.keyToValue(M_data.constData()));                                                                                \
    }                                                                                                                                                          \
    template<>                                                                                                                                                 \
    void KConfigGroup::writeEntry(const char *key, const Class::Enum &value, KConfigBase::WriteConfigFlags flags)                                              \
    {                                                                                                                                                          \
        const QMetaObject *M_obj = &Class::staticMetaObject;                                                                                                   \
        const int M_index = M_obj->indexOfEnumerator(#Enum);                                                                                                   \
        if (M_index == -1)                                                                                                                                     \
            qFatal(KCONFIGGROUP_ENUMERATOR_ERROR(#Enum));                                                                                                      \
        const QMetaEnum M_enum = M_obj->enumerator(M_index);                                                                                                   \
        writeEntry(key, QByteArray(M_enum.valueToKey(value)), flags);                                                                                          \
    }

/*!
 * \macro KCONFIGGROUP_DECLARE_FLAGS_QOBJECT(Class, Flags)
 * \relates KConfigGroup
 * Similar to KCONFIGGROUP_DECLARE_ENUM_QOBJECT but for flags declared with Q_FLAG()
 * (where multiple values can be set at the same time)
 */
#define KCONFIGGROUP_DECLARE_FLAGS_QOBJECT(Class, Flags)                                                                                                       \
    template<>                                                                                                                                                 \
    Class::Flags KConfigGroup::readEntry(const char *key, const Class::Flags &def) const                                                                       \
    {                                                                                                                                                          \
        const QMetaObject *M_obj = &Class::staticMetaObject;                                                                                                   \
        const int M_index = M_obj->indexOfEnumerator(#Flags);                                                                                                  \
        if (M_index == -1)                                                                                                                                     \
            qFatal(KCONFIGGROUP_ENUMERATOR_ERROR(#Flags));                                                                                                     \
        const QMetaEnum M_enum = M_obj->enumerator(M_index);                                                                                                   \
        const QByteArray M_data = readEntry(key, QByteArray(M_enum.valueToKeys(def)));                                                                         \
        return static_cast<Class::Flags>(M_enum.keysToValue(M_data.constData()));                                                                              \
    }                                                                                                                                                          \
    template<>                                                                                                                                                 \
    void KConfigGroup::writeEntry(const char *key, const Class::Flags &value, KConfigBase::WriteConfigFlags flags)                                             \
    {                                                                                                                                                          \
        const QMetaObject *M_obj = &Class::staticMetaObject;                                                                                                   \
        const int M_index = M_obj->indexOfEnumerator(#Flags);                                                                                                  \
        if (M_index == -1)                                                                                                                                     \
            qFatal(KCONFIGGROUP_ENUMERATOR_ERROR(#Flags));                                                                                                     \
        const QMetaEnum M_enum = M_obj->enumerator(M_index);                                                                                                   \
        writeEntry(key, QByteArray(M_enum.valueToKeys(value)), flags);                                                                                         \
    }

#include "kconfigconversioncheck_p.h"

template<typename T>
T KConfigGroup::readEntry(const char *key, const T &defaultValue) const
{
    KConfigConversionCheck::to_QVariant<T>();
    return qvariant_cast<T>(readEntry(key, QVariant::fromValue(defaultValue)));
}

template<typename T>
QList<T> KConfigGroup::readEntry(const char *key, const QList<T> &defaultValue) const
{
    KConfigConversionCheck::to_QVariant<T>();
    KConfigConversionCheck::to_QString<T>();

    QVariantList data;

    for (const T &value : defaultValue) {
        data.append(QVariant::fromValue(value));
    }

    QList<T> list;
    const auto variantList = readEntry<QVariantList>(key, data);
    for (const QVariant &value : variantList) {
        Q_ASSERT(value.canConvert<T>());
        list.append(qvariant_cast<T>(value));
    }

    return list;
}

template<typename Rep, typename Period>
std::chrono::duration<Rep, Period> KConfigGroup::readEntry(const char *key, std::chrono::duration<Rep, Period> value) const
{
    return std::chrono::duration<Rep, Period>(readEntry(key, value.count()));
}

template<typename Rep, typename Period>
std::chrono::duration<Rep, Period> KConfigGroup::readEntry(const QString &key, std::chrono::duration<Rep, Period> value) const
{
    return std::chrono::duration<Rep, Period>(readEntry(key, value.count()));
}

template<typename T>
void KConfigGroup::writeEntry(const char *key, const T &value, WriteConfigFlags pFlags)
{
    KConfigConversionCheck::to_QVariant<T>();
    writeEntry(key, QVariant::fromValue(value), pFlags);
}

template<typename T>
void KConfigGroup::writeEntry(const char *key, const QList<T> &list, WriteConfigFlags pFlags)
{
    KConfigConversionCheck::to_QVariant<T>();
    KConfigConversionCheck::to_QString<T>();
    QVariantList data;
    for (const T &value : list) {
        data.append(QVariant::fromValue(value));
    }

    writeEntry(key, data, pFlags);
}

template<typename Rep, typename Period>
void KConfigGroup::writeEntry(const QString &key, std::chrono::duration<Rep, Period> value, WriteConfigFlags pFlags)
{
    writeEntry(key, value.count(), pFlags);
}

template<typename Rep, typename Period>
void KConfigGroup::writeEntry(const char *key, std::chrono::duration<Rep, Period> value, WriteConfigFlags pFlags)
{
    writeEntry(key, value.count(), pFlags);
}

#endif // KCONFIGGROUP_H
