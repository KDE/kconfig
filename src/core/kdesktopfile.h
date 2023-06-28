/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Pietro Iglio <iglio@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KDESKTOPFILE_H
#define KDESKTOPFILE_H

#include <kconfig.h>

class KConfigGroup;
class KDesktopFilePrivate;

/**
 * \class KDesktopFile kdesktopfile.h <KDesktopFile>
 *
 * %KDE Desktop File Management.
 * This class implements %KDE's support for the freedesktop.org
 * <em>Desktop Entry Spec</em>. It only provides methods for reading, copying
 * and validating desktop files.
 *
 * After the KDesktopFile is constructed, its main entrypoint will likely
 * be desktopGroup(), which returns a KConfigGroup that can then be used
 * to write and save the file to disk. For example:
 *
 * @code
 * KDesktopFile file(QStandardPaths::ApplicationsLocation, "org.kde.myapp.desktop");
 * if(file.hasApplicationType()) {
 *     KConfigGroup desktopGroup = file.desktopGroup();
 *     desktopGroup.writeEntry("Terminal", false);
 *     desktopGroup.sync();
 * }
 * @endcode
 *
 * The second main entrypoint will likely be actionGroup(), following
 * similar code usage.
 *
 * @author Pietro Iglio <iglio@kde.org>
 * @see KConfigBase
 * @see KConfig
 * @see <a href="https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html">Desktop Entry Spec</a>
 */
class KCONFIGCORE_EXPORT KDesktopFile : public KConfig
{
public:
    /**
     * @brief Constructs a KDesktopFile object.
     *
     * This creates an object in memory that can then be used to access and
     * read or write to a file in a specified location.
     *
     * @param resourceType   Allows you to change what sort of resource
     *                       to search for if @p fileName is not absolute.
     * @param fileName       The name or path of the desktop file. It should
     *                       include the .desktop extension. If it is not an
     *                       absolute path, it will be located using the
     *                       resource type @p resourceType.
     * @see QStandardPaths
     */
    explicit KDesktopFile(QStandardPaths::StandardLocation resourceType, const QString &fileName);

    /**
     * @brief Constructs a KDesktopFile object.
     *
     * This creates an object in memory that can then be used to access and
     * read or write to a file in the standard
     * QStandardPaths::ApplicationsLocation where .desktop files are
     * typically stored.
     *
     * See QStandardPaths for more information on resources.
     *
     * @param fileName       The name or path of the desktop file. If it
     *                       is not absolute, it will be located
     *                       using the resource type
     *                       QStandardPaths::ApplicationsLocation.
     * @see QStandardPaths
     */
    explicit KDesktopFile(const QString &fileName);

    /**
     * @brief Destructs the KDesktopFile object.
     *
     * If any unsaved changes are left before the object is destructed, sync()
     * will be run first.
     */
    ~KDesktopFile() override;

    /**
     * @brief Whether this is really a desktop file.
     *
     * The check is performed by looking at the file extension without
     * opening it. The only valid extension is ".desktop".
     *
     * @param path The path of the file to check.
     *
     * @return @c true if the file appears to be a desktop file,
     *         @c false otherwise.
     */
    static bool isDesktopFile(const QString &path);

    /**
     * @brief Whether the user is authorized to run this desktop file.
     *
     * By default users are authorized to run all desktop files, but
     * the Kiosk framework can be used to restrict this action.
     *
     * Note that desktop files that are not in a standard location (as
     * specified by XDG_DATA_DIRS) must have their executable bit set
     * to be authorized, regardless of Kiosk settings.
     * This ensures users will only execute desktop files in non-standard
     * locations when there is clear intent.
     *
     * @param path The file to check.
     * @return @c true if the user is authorized to run the file,
     *         @c false otherwise.
     */
    static bool isAuthorizedDesktopFile(const QString &path);

    /**
     * @brief The location where changes for the desktop file @p path
     * should be written to.
     */
    static QString locateLocal(const QString &path);

    /**
     * @brief The main config group (named "Desktop Entry") in a desktop file.
     *
     * This can be used to manipulate desktop file entries using KConfigGroup.
     *
     * Example usage:
     * @code
     * KDesktopFile file("org.kde.myapp.desktop");
     * KConfigGroup desktopGroup = file.desktopGroup();
     * @endcode
     */
    KConfigGroup desktopGroup() const;

    /**
     * @brief The value of the "Type=" entry.
     * @return The desktop file type or QString() if unset.
     */
    QString readType() const;

    /**
     * @brief The value of the "Icon=" entry.
     * @return The icon name or QString() if unset.
     */
    QString readIcon() const;

    /**
     * @brief The value of the "Name=" entry.
     * @return The name or QString() if unset.
     */
    QString readName() const;

    /**
     * @brief The value of the "Comment=" entry.
     * @return The comment or QString() if unset.
     */
    QString readComment() const;

    /**
     * @brief The value of the "GenericName=" entry.
     * @return The generic name or QString() if unset.
     */
    QString readGenericName() const;

    /**
     * @brief The value of the "Path=" entry.
     * @return The path or QString() if unset.
     */
    QString readPath() const;

    /**
     * @brief The value of the "URL=" entry.
     * @return The URL or QString() if unset.
     */
    QString readUrl() const;

    /**
     * @brief A list of "Actions=" entries.
     * @return The list of actions.
     */
    QStringList readActions() const;

    /**
     * @brief A list of "MimeType=" entries.
     * @return The list of mime types.
     * @since 5.15
     */
    QStringList readMimeTypes() const;

    /**
     * @brief Creates a new desktop action group object.
     *
     * The action group will not be saved upon sync() if it contains no entries.
     *
     * Example usage:
     * @code
     * KDesktopFile file("org.kde.myapp.desktop");
     * KConfigGroup actionGroup = file.actionGroup("NewAction");
     * actGroup.writeEntry("Name", "New Entry");
     * file.sync();
     * @endcode
     *
     * @param group The new action group.
     *
     * @see <a href="https://specifications.freedesktop.org/desktop-entry-spec/latest/ar01s11.html">Desktop Entry Spec - Additional Application Actions</a>
     */
    KConfigGroup actionGroup(const QString &group);

    // TODO KF6, don't return by const value
    const KConfigGroup actionGroup(const QString &group) const;

    /**
     * @brief Whether the specified action group exists in the desktop file.
     *
     * @param group The action group to check.
     *
     * @return @c true if the action group exists,
     *         @c false otherwise.
     *
     * @see <a href="https://specifications.freedesktop.org/desktop-entry-spec/latest/ar01s11.html">Desktop Entry Spec - Additional Application Actions</a>
     */
    bool hasActionGroup(const QString &group) const;

    /**
     * @brief Whether the desktop file type is "Link".
     *
     * The link points to the "URL=" entry instead of "Exec=".
     *
     * @return @c true if there is a "Type=Link" entry,
     *         @c false otherwise.
     */
    bool hasLinkType() const;

    /**
     * @brief Whether the desktop file type is "Application".
     *
     * @return @c true if there is a "Type=Application" entry,
     *         @c false otherwise.
     */
    bool hasApplicationType() const;

    /**
     * @brief Whether the desktop file type is "FSDevice".
     *
     * FSDevice corresponds to the type of file system that will be mounted.
     *
     * @return @c true if there is a "Type=FSDevice" entry,
     *         @c false otherwise.
     *
     * @see <a href="https://specifications.freedesktop.org/desktop-entry-spec/latest/apb.html">Keys currently reserved for use in KDE</a>
     */
    bool hasDeviceType() const;

    /**
     * @brief Whether the TryExec field contains a binary
     * that is found on the local system.
     *
     * @return @c true if TryExec contains an existing binary,
     *         @c false otherwise.
     */
    bool tryExec() const;

    /**
     * @brief The value of the "X-DocPath=" Or "DocPath=" entry.
     *
     * This is a custom entry made by KDE that corresponds to the path for
     * the offline HTML documentation. This path might be, for instance,
     * stored in "/usr/share/doc/HTML/en/" (or any other directory defined by
     * the distribution, as well as localized directory equivalents), and its
     * value typically uses the format "appname/index.html", although it may
     * include more subdirectory levels.
     *
     * @return The value of the "X-DocPath=" Or "DocPath=" entry.
     *
     * @see <a href="https://specifications.freedesktop.org/desktop-entry-spec/latest/apb.html">Keys currently reserved for use in KDE</a>
     */
    QString readDocPath() const;

    /**
     * @brief Whether the entry should be suppressed in menus.
     *
     * This handles the NoDisplay and OnlyShowIn / NotShowIn keys.
     *
     * @return @c true if NoDisplay=true, OnlyShowIn does not include "KDE",
     * or NotShowIn does include "KDE"; @c false otherwise.
     * @since 4.1
     */
    bool noDisplay() const;

    /**
     * @brief Copies all entries from this config object to a new
     * KDesktopFile object that will save itself to @p file.
     *
     * Actual saving to @p file happens when the returned object is
     * destructed or when sync() is called upon it.
     *
     * @param file The new KDesktopFile object it will save itself to.
     */
    KDesktopFile *copyTo(const QString &file) const;

    /**
     * @brief The name of the desktop file that was used to construct this KDesktopFile.
     * @return The desktop file name without path and with extension.
     */
    QString fileName() const;

private:
    Q_DISABLE_COPY(KDesktopFile)

    Q_DECLARE_PRIVATE(KDesktopFile)
};

#endif
