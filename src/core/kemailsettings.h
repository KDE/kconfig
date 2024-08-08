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

/*!
 * \class KEMailSettings
 * \inmodule KConfigCore
 *
 * \brief Access to e-mail settings.
 *
 * This is just a small class to facilitate accessing e-mail settings in
 * a sane way, and allowing any program to manage multiple e-mail
 * profiles effortlessly
 *
 * The default profile is automatically selected in the constructor.
 **/
class KCONFIGCORE_EXPORT KEMailSettings
{
    Q_DECLARE_TR_FUNCTIONS(KEMailSettings)
public:
    /*!
     * The list of settings that I thought of when I wrote this
     * class.
     *
     * \value ClientProgram
     * \value ClientTerminal
     * \value RealName
     * \value EmailAddress
     * \value ReplyToAddress
     * \value Organization
     *
     * \sa getSetting()
     * \sa setSetting()
     **/
    enum Setting {
        ClientProgram,
        ClientTerminal,
        RealName,
        EmailAddress,
        ReplyToAddress,
        Organization,
    };

    /*!
     * Default constructor, just sets things up and sets the default profile
     * as the current profile
     **/
    KEMailSettings();

    KEMailSettings(const KEMailSettings &) = delete;
    KEMailSettings &operator=(const KEMailSettings &) = delete;

    ~KEMailSettings();

    /*!
     * Returns the list of profiles available.
     **/
    QStringList profiles() const;

    /*!
     * Change the current profile.
     *
     * \a s the name of the new profile
     **/
    void setProfile(const QString &s);

    /*!
     * Returns the name of the default profile.
     **/
    QString defaultProfileName() const;

    /*!
     * Sets a new default.
     *
     * \a def the new default
     **/
    void setDefault(const QString &def);

    /*!
     * Get one of the predefined "basic" settings.
     *
     * \a s the setting to get
     *
     * Returns the value of the setting, or QString() if not
     *         set
     **/
    QString getSetting(KEMailSettings::Setting s) const;

    /*!
     * Set one of the predefined "basic" settings.
     *
     * \a s the setting to set
     *
     * \a v the new value of the setting, or QString() to unset
     **/
    void setSetting(KEMailSettings::Setting s, const QString &v);

private:
    KEMailSettingsPrivate *const p;
};

#endif
