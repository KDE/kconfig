/*
    SPDX-FileCopyrightText: 2000 Alex Zepeda <zipzippy@sonic.net>

    SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef _KEMAILSETTINGS_H
#define _KEMAILSETTINGS_H

#include <QCoreApplication> // Q_DECLARE_TR_FUNCTIONS
#include <QStringList>

#include <kconfigcore_export.h>

class KEMailSettingsPrivate;

/**
 * \class KEMailSettings kemailsettings.h <KEMailSettings>
 *
 * This is a small class that uses KConfig to facilitate accessing e-mail
 * settings in a centralized manner, allowing any program to manage
 * multiple e-mail profiles effortlessly.
 *
 * The email profile management is handled via the
 * $XDG_CONFIG_DIR/emaildefaults configuration file. The main "[Defaults]"
 * group should contain a "Profile" entry stating which profile is currently
 * active, while profiles themselves are prefixed with "PROFILE_".
 *
 * Common email metadata is available via the Setting enum, which is used to
 * read from emaildefaults using getSetting() and write to the disk with
 * setSetting(), which runs KConfig::sync() internally.
 *
 * The default profile (PROFILE_Default) is automatically selected in the
 * constructor.
 *
 * @author Alex Zepeda zipzippy@sonic.net
 **/
class KCONFIGCORE_EXPORT KEMailSettings
{
    Q_DECLARE_TR_FUNCTIONS(KEMailSettings)
public:
    /**
     * @brief A list of common metadata used for email.
     *
     * @see getSetting()
     * @see setSetting()
     **/
    enum Setting {
        /** The program used to manage email. */
        ClientProgram,
        /** The preferred terminal of the user associated with the email. */
        ClientTerminal,
        /** The name of the user associated with the email. */
        RealName,
        /** The address of the user associated with the email. */
        EmailAddress,
        /**
         * The address used to reply to the user associated with the email,
         * which may or may not be the same as EmailAddress.
         */
        ReplyToAddress,
        /** The organization the user belongs to. */
        Organization,
    };

    /**
     * @brief This constructs an object that points to the emaildefaults
     * configuration file.
     *
     * Profile information is always read from and written to the
     * emaildefaults file, and it is not possible to specify a different
     * configuration file.
     **/
    KEMailSettings();

    KEMailSettings(const KEMailSettings &) = delete;
    KEMailSettings &operator=(const KEMailSettings &) = delete;

    /**
     * @brief Default destructor.
     **/
    ~KEMailSettings();

    /**
     * @brief The list of profiles available.
     *
     * The list of profiles can be used to retrieve profile names, which can
     * then be used with setProfile() and setDefault().
     *
     * @return The list of profiles in the emaildefaults configuration file.
     **/
    QStringList profiles() const;

    /**
     * @brief Change the current profile or create a new one.
     *
     * New profiles follow the pattern "PROFILE_" + newprofilename.
     *
     * @param s The name of the new profile.
     **/
    void setProfile(const QString &s);

    /**
     * @brief The name of the current default profile.
     * @returns The name of the current default profile, or QString() if unset.
     **/
    QString defaultProfileName() const;

    /**
     * @brief Sets a new default profile.
     * @param def The new default profile to be set.
     **/
    void setDefault(const QString &def);

    /**
     * @brief The value assigned to one of the predefined entries from the
     * @ref Setting enum.
     *
     * @param s The @ref Setting whose value will be returned.
     *
     * @return The value of the @ref Setting, or QString() if unset.
     **/
    QString getSetting(KEMailSettings::Setting s) const;

    /**
     * @brief The value to be assigned to one of the predefined entries
     * from the @ref Setting enum.
     *
     * The write operation will run KConfig::sync() after each write.
     *
     * @param s The @ref Setting that will be changed.
     * @param v The new value of the @ref Setting, or QString() if unset.
     **/
    void setSetting(KEMailSettings::Setting s, const QString &v);

private:
    KEMailSettingsPrivate *const p;
};

#endif
