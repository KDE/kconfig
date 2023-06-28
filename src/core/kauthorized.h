/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>
    SPDX-FileCopyrightText: 1998, 1999 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KAUTHORIZED_H
#define KAUTHORIZED_H

#include <kconfigcore_export.h>

#include <QMetaEnum>
#include <QObject>
#include <QStringList>
#include <QVariant>

class QUrl;
class QString;

/**
 * This class provides the API needed to read from the Kiosk action
 * restriction system; the KIO and KXmlGui frameworks build on this.
 *
 * The relevant settings are read from the application's KSharedConfig
 * instance, so actions can be disabled on a per-application or global
 * basis (by using the kdeglobals file).
 *
 * @see <a href="https://develop.kde.org/docs/administration/kiosk/">Kiosk -
 * Simple configuration management for large deployment</a>
 */
class KCONFIGCORE_EXPORT KAuthorized : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief This enum consists of common restrictions following the
     * Kiosk framework.
     *
     * These restrictions are not bound to a specific action, and so are generic.
     *
     * Each enum value corresponds to a Kiosk key with the same name in
     * lowercase and represents an action that is authorized. For example,
     * the SHELL_ACCESS value is converted to the "shell_access" string.
     *
     * @since 5.88
     *
     * @see authorize(KAuthorized::GenericRestriction)
     */
    enum GenericRestriction {
        /**
         * Whether the user is authorized to open a shell or execute
         * shell commands.
         */
        SHELL_ACCESS = 1,
        /**
         * Whether the collaborative data sharing framework KNewStuff is authorized.
         */
        GHNS,
        // GUI behavior
        /**
         * Whether typed characters in password fields can be made visible.
         */
        LINEEDIT_REVEAL_PASSWORD,
        /**
         * Whether line edits should be allowed to display completions.
         */
        LINEEDIT_TEXT_COMPLETION,
        /**
         * Whether toolbars of apps should be movable.
         */
        MOVABLE_TOOLBARS,
        /**
         * Whether .desktop files should be run as executables when clicked.
         */
        RUN_DESKTOP_FILES,
    };
    Q_ENUM(GenericRestriction)

    /**
     * @brief This enum consists of common actions to be managed via the
     * Kiosk framework.
     *
     * @see authorizeAction(KAuthorized::GenericAction)
     *
     * @since 5.88
     */
    enum GenericAction {
        /**
         * Whether the open-with menu should be shown for files etc.
         *
         * This corresponds to the "action/openwith" key.
         *
         * @see <a href="https://develop.kde.org/docs/administration/kiosk/keys/#file-manager">Kiosk File Manager key reference</a>
         */
        OPEN_WITH = 1,
        /**
         * Whether mime-type accociations are allowed to be configured.
         *
         * This corresponds to the "action/editfiletype" key.
         *
         * @see <a href="https://develop.kde.org/docs/administration/kiosk/keys/#file-manager">Kiosk File Manager key reference</a>
         */
        EDITFILETYPE,
        /**
         * Whether the toolbar should be displayed in apps.
         *
         * This corresponds to the "action/options_show_toolbar" key.
         *
         * @see <a href="https://develop.kde.org/docs/administration/kiosk/keys/#application-action-restrictions">Kiosk Application Action Restrictions</a>
         */
        OPTIONS_SHOW_TOOLBAR,
        /**
         * Whether an action to switch the app language should be shown.
         */
        SWITCH_APPLICATION_LANGUAGE,
        /**
         * Whether saving bookmarks is allowed.
         *
         * This corresponds to the "action/bookmarks" key.
         *
         * @see <a href="https://develop.kde.org/docs/administration/kiosk/keys/#application-action-restrictions">Kiosk Application Action Restrictions</a>
         */
        BOOKMARKS,
    };
    Q_ENUM(GenericAction)

    /**
     * @brief Whether the user is permitted to perform a certain action.
     *
     * All settings are read from the "[KDE Action Restrictions]" group.
     * For example, if kdeglobals contains
     * @verbatim
     * [KDE Action Restrictions][$i]
     * shell_access=false
     * @endverbatim
     * then
     * @code
     * KAuthorized::authorize("shell_access");
     * @endcode
     * will return @c false.
     *
     * This method is intended for actions that do not necessarily have a
     * one-to-one correspondence with a menu or toolbar item (for example,
     * a QAction in a KXmlGui application). "shell_access" is an example of
     * such a "generic" action.
     *
     * The convention for actions like "File->New" is to prepend the action
     * name with "action/", for example "action/file_new".  This is what
     * authorizeAction() does.
     *
     * @param action  The name of the action.
     * @return        @c true if the action is authorized, @c false
     *                otherwise.
     *
     * @see authorizeAction()
     */
    Q_INVOKABLE static bool authorize(const QString &action);

    /**
     * @brief Whether the user is permitted to perform a common action.
     *
     * @overload
     * @since 5.88
     * @see KAuthorized::GenericRestriction
     */
    Q_INVOKABLE static bool authorize(GenericRestriction action);

    /**
     * @brief Whether the user is permitted to perform a certain action.
     *
     * This behaves like authorize(), except that it accounts fo the "action/"
     * string prepended to @p action in the configuration file.
     *
     * So if kdeglobals contains
     * @verbatim
       [KDE Action Restrictions][$i]
       action/file_new=false
       @endverbatim
     * then
     * @code
     * KAuthorized::authorizeAction("file_new");
     * @endcode
     * will return the value for the string "action/file_new", @c false.
     *
     * KXmlGui-based applications should not need to call this
     * function, as KActionCollection will do it automatically.
     *
     * @param action  The name of a QAction action.
     * @return        @c true if the QAction is authorized, @c false
     *                otherwise.
     * @since 5.24
     *
     * @see authorize()
     */
    Q_INVOKABLE static bool authorizeAction(const QString &action);

    /**
     * @brief Whether the user is permitted to perform a certain action.
     *
     * @overload
     * @since 5.88
     *
     * @see KAuthorized::GenericAction
     */
    Q_INVOKABLE static bool authorizeAction(GenericAction action);

    /**
     * @brief Whether the user is permitted to use a certain control module.
     *
     * Each KDE Control Module (or KDE Configuration Module, KCM) corresponds
     * to a main entry in the sidebar of the KDE Plasma System Settings.
     *
     * The name for each KCM can be found using the kcmshell tool, or returned
     * via KPluginMetaData::pluginId.
     *
     * All settings are read from the "[KDE Control Module Restrictions]"
     * group.  For example, if kdeglobals contains
     * @verbatim
     * [KDE Control Module Restrictions][$i]
     * kcm_lookandfeel=false
     * @endverbatim
     * where "kcm_lookandfeel" corresponds to the Global Theme KCM, then
     * @code
     * KAuthorized::authorizeControlModule("kcm_lookandfeel");
     * @endcode
     * will return @c false.
     *
     * @param pluginId  The desktop menu ID for the control module.
     * @return @c true if access to the module is authorized, @c false otherwise.
     *
     * @see authorizeControlModules()
     * @see <a href="https://develop.kde.org/docs/administration/kiosk/introduction/#kde-control-module-restrictions">KDE Control Module Restrictions</a>
     * @see KPluginMetaData::pluginId
     */
    Q_INVOKABLE static bool authorizeControlModule(const QString &pluginId);

    /**
     * @brief Determines which control modules from a list the user
     * is permitted to use.
     *
     * @param pluginIds A list of KCM plugin IDs as listed by the kcmshell tool.
     * @return The entries in @p pluginIds for which authorizeControlModule() returns @c true.
     *
     * @see authorizeControlModule()
     * @see KPluginMetaData::pluginId
     */
    Q_INVOKABLE static QStringList authorizeControlModules(const QStringList &pluginIds);

private:
    friend class KConfigQmlPlugin;
    explicit KAuthorized();
};

#endif
