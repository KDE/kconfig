/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1997-1999 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSHAREDCONFIG_H
#define KSHAREDCONFIG_H

#include <QExplicitlySharedDataPointer>
#include <kconfig.h>

/**
 * \class KSharedConfig ksharedconfig.h <KSharedConfig>
 *
 * @brief KConfig variant using shared memory.
 *
 * KSharedConfig provides a shared (reference-counted) variant
 * of KConfig. This allows you to use/manipulate the same configuration
 * files from different places in your code without worrying about
 * accidentally overwriting changes. It is preferable over multiple
 * KConfig objects.
 *
 * There are a few different ways to use KSharedConfig.
 *
 * With a KSharedConfigPtr:
 *
 * @code
 * KSharedConfigPtr config = KSharedConfig::openConfig();
 * KConfigGroup configGroup = config->group("General");
 * configGroup.writeEntry("RunOnce", false);
 * @endcode
 *
 * To access data or methods from the global shared object:
 *
 * @code
 * QStringList groupList = KSharedConfig::openConfig()->groupList();
 * // ...
 * configGroup.writeEntry("RunOnce", true);
 * KSharedConfig::openConfig()->sync();
 * @endcode
 *
 * Creating a read-only KConfigGroup directly:
 *
 * @code
 * KConfigGroup configGroup(KSharedConfig::openConfig(), "General");
 * @endcode
 *
 * @see KConfig
 * @see KConfigGroup
 */
class KCONFIGCORE_EXPORT KSharedConfig : public KConfig, public QSharedData // krazy:exclude=dpointer (only for refcounting)
{
public:
    /**
     * @brief A convenience, thread-safe, reference-counted pointer to access
     * the global KSharedConfig object.
     *
     * @see QExplicitlySharedDataPointer
     */
    typedef QExplicitlySharedDataPointer<KSharedConfig> Ptr;

public:
    /**
     * @brief A pointer to a KSharedConfig object to manipulate a
     * configuration file.
     *
     * The openConfig() method is threadsafe: every thread gets a separate
     * repository of shared KConfig objects. This means, however, that you'll
     * be responsible for synchronizing the instances of KConfig for the same
     * filename between threads, using KConfig::reparseConfiguration() after
     * a manual change notification, just like you have to do between processes.
     *
     * If an absolute path is specified for @p fileName, only that file will
     * be used as the store for the configuration settings.
     *
     * @code
     * KSharedConfigPtr config( "/etc/kderc" );
     * // Only /etc/kderc is used.
     * @endcode
     *
     * If a non-absolute path is provided, the file will be looked for
     * in the standard directory specified by @p type.
     *
     * @code
     * KSharedConfigPtr config( "appconfigrc" );
     * // The file ~/.config/appconfigrc is used.
     * @endcode
     *
     * If no path is provided, a default configuration file will be used
     * based on the name of the main application component, whether
     * it was defined with QCoreApplication::setApplicationName or
     * KAboutData, with an "rc" suffix appended to it.
     *
     * @code
     * QCoreApplication::setApplicationName("appname");
     * // Elsewhere:
     * KSharedConfigPtr config;
     * // The file ~/.config/appnamerc is used.
     * @endcode
     *
     * @note This static method cannot be used together with KConfig,
     * but the returned object is able to use the same methods.
     *
     * @p mode determines whether the user or global settings will be allowed
     * to influence the values returned by this object. See
     * KConfig::OpenFlags for more details.
     *
     * @param fileName  The configuration file to open. If empty, it will be
     *                  determined automatically (from --config on the command
     *                  line, otherwise from the application name + "rc").
     * @param mode      How global settings should affect the configuration
     *                  options exposed by this KConfig object.
     * @param type      The standard directory to look for the configuration
     *                  file.
     *
     * @see KConfig
     * @see QStandardPaths
     */
    static KSharedConfig::Ptr
    openConfig(const QString &fileName = QString(), OpenFlags mode = FullConfig, QStandardPaths::StandardLocation type = QStandardPaths::GenericConfigLocation);

    /**
     * @brief Creates a KSharedConfig object to manipulate a configuration
     * file suitable for storing state information.
     *
     * Use this for storing information that changes frequently and should
     * not be saved by configuration backup utilities. The standard
     * location is QStandardPaths::AppDataLocation, which corresponds to
     * a configuration file under `$XDG_DATA_DIRS/appname/` on Linux
     * systems.
     *
     * If an absolute path is specified for @p fileName, that file will be used
     * as the store for the configuration settings.
     *
     * If a non-absolute path is provided, the file will be looked for in the
     * standard data directory (QStandardPaths::AppDataLocation).
     *
     * If no path is provided, a default configuration file will be used based
     * on the name of the main application component, without path and with
     * the "staterc" suffix.
     *
     * @note This static method cannot be used together with KConfig,
     * but the returned object is able to use the same methods.
     *
     * @param fileName The configuration file to open.
     *
     * @since 5.67
     *
     * @see KConfig
     * @see openConfig()
     */
    static KSharedConfig::Ptr openStateConfig(const QString &fileName = QString());

    ~KSharedConfig() override;

private:
    Q_DISABLE_COPY(KSharedConfig)
    KConfigGroup groupImpl(const QByteArray &aGroup) override;
    const KConfigGroup groupImpl(const QByteArray &aGroup) const override;

    KCONFIGCORE_NO_EXPORT KSharedConfig(const QString &file, OpenFlags mode, QStandardPaths::StandardLocation resourceType);
};

typedef KSharedConfig::Ptr KSharedConfigPtr;

#endif // multiple inclusion guard
