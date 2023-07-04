/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
    SPDX-FileCopyrightText: 2001 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIG_H
#define KCONFIG_H

#include "kconfigbase.h"

#include <kconfigcore_export.h>

#include <QByteArray>
#include <QList>
#include <QStandardPaths>
#include <QString>
#include <QVariant>

class KConfigGroup;
class KEntryMap;
class KConfigPrivate;

/**
 * \class KConfig kconfig.h <KConfig>
 *
 * \brief The central class of the KDE configuration data system.
 *
 * This class allows you to easily manage configuration files.
 *
 * Configuration files are typically stored in /etc/ for system configuration
 * or in ~/.config/ for user configuration. By default they have the suffix
 * "rc", which stands for "run commands".
 *
 * Quickstart:
 *
 * Load a specific configuration file:
 * \code{.cpp}
 * KConfig config( "/etc/kderc", KConfig::SimpleConfig );
 * \endcode
 *
 * Load a configuration file in `~/.config`:
 * \code{.cpp}
 * KConfig config( "pluginrc" );
 * \endcode
 *
 * In general it is recommended to use KSharedConfig instead of
 * creating multiple instances of KConfig to avoid the overhead of
 * separate objects or concerns about synchronizing writes to disk
 * even if the configuration object is updated from multiple code paths.
 * KSharedConfig provides a set of open methods as counterparts for the
 * KConfig constructors.
 *
 * Get the default application config object via KSharedConfig::openConfig():
 *
 * @code{.cpp}
 * KSharedConfigPtr config = KSharedConfig::openConfig();
 * @endcode
 *
 * After loading a KConfig or KSharedConfig object, use KConfigGroup to access
 * each individual group and entry:
 *
 * @code{.cpp}
 * KSharedConfigPtr config = KSharedConfig::openConfig( "pluginrc" );
 * KConfigGroup configGroup = config->group( "General" );
 * configGroup.writeEntry( "RunOnce", false );
 * @endcode
 *
 * @see KSharedConfig
 * @see KConfigGroup
 * @see <a href="https://develop.kde.org/docs/features/configuration/">KConfig tutorial</a>
 */
class KCONFIGCORE_EXPORT KConfig : public KConfigBase
{
public:
    /**
     * @brief Determines how the system-wide and user's global settings will
     * affect the reading of the configuration.
     *
     * Note that the main configuration source overrides the cascaded sources,
     * which override those provided to addConfigSources(), which override the
     * global sources. The exception is that if a key or group is marked as
     * being immutable, it will not be overridden.
     *
     * Note that all values other than IncludeGlobals and CascadeConfig are
     * convenience definitions for the basic mode. Do @em not combine them.
     *
     * @see OpenFlags
     */
    enum OpenFlag {
        /**
         * System-wide configuration sources are used to provide defaults
         * for the settings accessed through this object, or
         * possibly to override those settings in certain cases.
         */
        IncludeGlobals = 0x01,
        /**
         * The kdeglobals configuration file is used as a configuration source
         * to provide defaults. This will result in the system-wide kdeglobals
         * sources also getting included.
         */
        CascadeConfig = 0x02,
        /**
         * A single configuration file.
         */
        SimpleConfig = 0x00,
        /**
         * Include user's globals, but omit system settings.
         */
        NoCascade = IncludeGlobals,
        /**
         * Cascade to system settings, but omit user's globals.
         */
        NoGlobals = CascadeConfig,
        /**
         * Full-fledged config, including globals and cascading to
         * system settings. This is the same as `IncludeGlobals | CascadeConfig`.
         */
        FullConfig = IncludeGlobals | CascadeConfig,
    };
    /**
     * @brief Stores a combination of #OpenFlag values.
     */
    Q_DECLARE_FLAGS(OpenFlags, OpenFlag)

    /**
     * @brief Creates a KConfig object to manipulate a configuration file for
     * the current application.
     *
     * If an absolute path is specified for @p file, only that file will be used
     * as the storage for the configuration settings.
     *
     * @code{.cpp}
     * KConfig config( "/etc/kderc" );
     * // Only /etc/kderc is used.
     * @endcode
     *
     * If a non-absolute path is provided, the file will be looked for in the
     * standard directory specified by type.
     *
     * @code{.cpp}
     * KConfig config( "appconfigrc" );
     * // The file ~/.config/appconfigrc is used.
     * @endcode
     *
     * If no path is provided, a default configuration file will be used based
     * on the name of the main application component, whether it was defined
     * with QCoreApplication::setApplicationName or KAboutData, with an "rc"
     * suffix appended to it.
     *
     * @code{.cpp}
     * QCoreApplication::setApplicationName("appname");
     * // Elsewhere:
     * KConfig config();
     * // The file ~/.config/appnamerc is used.
     * @endcode
     *
     * @p mode determines whether the user or global settings will be allowed
     * to influence the values returned by this object. See #OpenFlag for
     * more details.
     *
     * @note You probably want to use KSharedConfig::openConfig() instead.
     *
     * @param file  The name of the file. If an empty string is passed in and
     *              the SimpleConfig mode is used, then an in-memory KConfig
     *              object is created that will not write out to any file
     *              and does not require any file in the filesystem at all.
     * @param mode  How global settings should affect the configuration
     *              options exposed by this KConfig object.
     * @param type  The standard directory to look for the configuration file.
     *
     * @see KSharedConfig::openConfig(const QString&, OpenFlags, QStandardPaths::StandardLocation)
     * @see OpenFlags
     */
    explicit KConfig(const QString &file = QString(),
                     OpenFlags mode = FullConfig,
                     QStandardPaths::StandardLocation type = QStandardPaths::GenericConfigLocation);

    /**
     * @internal
     *
     * Creates a KConfig object using the specified backend. If the backend
     * cannot be found or loaded, then the standard configuration parser is
     * used as a fallback.
     *
     * @param file the file to be parsed
     * @param backend the backend to load
     * @param type where to look for the file if an absolute path is not provided
     *
     * @since 4.1
     */
    KConfig(const QString &file, const QString &backend, QStandardPaths::StandardLocation type = QStandardPaths::GenericConfigLocation);

    ~KConfig() override;

    /**
     * @brief The location used for the KConfig object.
     *
     * @return The standard location enum passed to the constructor.
     *
     * @since 5.0
     */
    QStandardPaths::StandardLocation locationType() const;

    /**
     * @brief The filename used to store the configuration, without path and
     * with "rc" suffix.
     */
    QString name() const;

    /**
     * @brief The flags used to open the KConfig object.
     * @since 5.3
     */
    OpenFlags openFlags() const;

    /// @reimp
    bool sync() override;

    /**
     * @brief Whether writes made to the object are present but not saved to disk.
     *
     * @return @c true if sync() has any changes to write out.
     *
     * @since 4.12
     */
    bool isDirty() const;

    /// @reimp
    void markAsClean() override;

    /// @{ configuration object state
    /// @reimp
    AccessMode accessMode() const override;

    /**
     * @brief Whether the configuration can be written to.
     *
     * If @p warnUser is @c true and the configuration cannot be
     * written to (that is, if this method returns @c false), a warning
     * message box will be shown to the user telling them to
     * contact their system administrator to get the problem fixed.
     *
     * The most likely cause for this method returning @c false
     * is that the user does not have write permission for the
     * configuration file.
     *
     * @param warnUser Whether to show a warning message to the user
     *                 if the configuration cannot be written to.
     *
     * @returns @c true if the configuration can be written to,
     *          @c false otherwise
     */
    bool isConfigWritable(bool warnUser);
    /// @}

    /**
     * @brief Copies all entries from this config object to a new config
     * object that will be saved to @p file.
     *
     * The configuration will not actually be saved to @p file
     * until the returned object is destroyed, or sync() is called
     * on it.
     *
     * Do not forget to delete the returned KConfig object if
     * @p config was nullptr.
     *
     * @param file   The new config object where the entries will be saved to.
     * @param config If unset, copy to the given KConfig object rather
     *               than creating a new one.
     *
     * @return @p config if it was set, otherwise a new KConfig object.
     */
    KConfig *copyTo(const QString &file, KConfig *config = nullptr) const;

    /**
     * @brief Whether the configuration file contains a certain update.
     *
     * If the configuration file associated with the KConfig object
     * does not contain the update @p id present in @p updateFile, the
     * kconf_update binary (stored in LIBEXECDIR) is run to update
     * the configuration file using reparseConfiguration().
     *
     * If you install config update files with critical fixes
     * you may wish to use this method to verify that a critical
     * update has indeed been performed to catch the case where
     * a user restores an old config file from backup that has
     * not been updated yet.
     *
     * @param id The update to check
     * @param updateFile The file containing the update
     */
    void checkUpdate(const QString &id, const QString &updateFile);

    /**
     * @brief Updates the state of this object to match the persistent storage.
     *
     * Note that if this object has pending changes, this method will
     * call sync() first so as not to lose those changes. In other words,
     * the pending changes will be saved to disk, and then the KConfig object
     * will be updated to that stored in the disk.
     */
    void reparseConfiguration();

    /// @{ extra config files
    /**
     * @brief Adds the list of configuration sources to the merge stack.
     *
     * Only files are accepted as configuration sources.
     *
     * The first entry in @p sources is treated as the most general and will
     * be overridden by the second entry.  The settings in the final entry
     * in @p sources will override all the other sources provided in the list.
     *
     * The settings in @p sources will also be overridden by the sources
     * provided by any previous calls to addConfigSources().
     *
     * This is useful to manage settings from many configuration files at once
     * by utilizing a priority system.
     *
     * The settings in the global configuration sources will be overridden by
     * the sources provided to this method (see IncludeGlobals).
     *
     * All the sources provided to any call to this method will be overridden
     * by any files that cascade from the source provided to the constructor
     * (see CascadeConfig), which will in turn be overridden by the source
     * provided to the constructor.
     *
     * Note that only the most specific file, that is, the file provided
     * to the constructor, will be used for when this object is written to disk.
     *
     * The state is automatically updated by this method, so there is no need
     * to call reparseConfiguration().
     *
     * @param sources A list of extra config sources. Each config source must
     *                refer to an absolute path.
     *
     * @see additionalConfigSources()
     * @see KConfig::IncludeGlobals
     * @see KConfig::CascadeConfig
     */
    void addConfigSources(const QStringList &sources);

    /**
     * @brief A list of the additional configuration sources used in this object.
     *
     * This does not include configuration sources added via the destructor.
     *
     * @see addConfigSources()
     */
    QStringList additionalConfigSources() const;

    /// @}
    /// @{ locales
    /**
     * @brief The current locale.
     */
    QString locale() const;
    /**
     * @brief Sets the locale to @p aLocale.
     *
     * The global locale is used by default.
     *
     * @note If set to an empty QString(), @b no locale will be matched.
     * This effectively disables reading translated entries.
     *
     * @return @c true if locale was changed,
     *         @c false if the call had no effect (for example, if @p aLocale
     *         was already the current locale for this object)
     */
    bool setLocale(const QString &aLocale);
    /// @}

    /// @{ defaults
    /**
     * @brief When set, all readEntry() calls return the system-wide (default)
     * values instead of the user's settings.
     *
     * default: @c false
     *
     * @param b Whether to read the system-wide defaults instead of the
     *          user's settings
     */
    void setReadDefaults(bool b);
    /**
     * @returns @c true if the system-wide defaults will be read instead of the
     *          user's settings, @c false otherwise
     */
    bool readDefaults() const;
    /// @}

    /// @{ immutability
    /// @reimp
    bool isImmutable() const override;
    /// @}

    /// @reimp
    QStringList groupList() const override;

    /**
     * @brief A map (tree) of entries in a particular group.
     *
     * The entries are all returned as strings.
     *
     * @param aGroup The group to get entries from.
     *
     * @return A map of entries from the specified group, indexed by key.
     *         The returned map may be empty if the group is empty, or not found.
     * @see   QMap
     */
    QMap<QString, QString> entryMap(const QString &aGroup = QString()) const;

    /**
     * @brief Sets the name of the application config file.
     * @since 5.0
     */
    static void setMainConfigName(const QString &str);

    /**
     * @brief The name of application config file.
     * @since 5.93
     */
    static QString mainConfigName();

protected:
    bool hasGroupImpl(const QByteArray &group) const override;
    KConfigGroup groupImpl(const QByteArray &b) override;
    const KConfigGroup groupImpl(const QByteArray &b) const override;
    void deleteGroupImpl(const QByteArray &group, WriteConfigFlags flags = Normal) override;
    bool isGroupImmutableImpl(const QByteArray &aGroup) const override;

    friend class KConfigGroup;
    friend class KConfigGroupPrivate;
    friend class KSharedConfig;

    /**
     * @internal
     * Virtual hook, used to add new "virtual" functions while maintaining
     * binary compatibility. Unused in this class.
     */
    void virtual_hook(int id, void *data) override;

    KConfigPrivate *const d_ptr;

    KCONFIGCORE_NO_EXPORT explicit KConfig(KConfigPrivate &d);

private:
    friend class KConfigTest;

    Q_DISABLE_COPY(KConfig)

    Q_DECLARE_PRIVATE(KConfig)
};
Q_DECLARE_OPERATORS_FOR_FLAGS(KConfig::OpenFlags)

#endif // KCONFIG_H
