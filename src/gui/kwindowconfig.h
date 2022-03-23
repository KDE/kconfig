/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2012 Benjamin Port <benjamin.port@ben2367.fr>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KWINDOWCONFIG_H
#define KWINDOWCONFIG_H

#include <kconfiggroup.h>
#include <kconfiggui_export.h>

#include "kconfigbase.h"

class QWindow;

/**
 * Save and load window sizes into a config
 */
namespace KWindowConfig
{
/**
 * Saves the window's size dependent on the screen dimension either to the
 * global or application config file.
 *
 * @note the group must be set before calling
 *
 * @param window The window to save size.
 * @param config The config group to read from.
 * @param options passed to KConfigGroup::writeEntry()
 * @since 5.0
 */
KCONFIGGUI_EXPORT void saveWindowSize(const QWindow *window, KConfigGroup &config, KConfigGroup::WriteConfigFlags options = KConfigGroup::Normal);

/**
 * Restores the dialog's size from the configuration according to
 * the screen size.
 *
 * If you're calling this from a constructor (for a mainwindow or dialog, for instance)
 * you should first call winId() so that a QWindow is created, then you can call windowHandle()
 * to pass to this method.
 *
 * Example code:
 * @code
 *   create(); // ensure there's a window created
 *   const QSize availableSize = windowHandle()->screen()->availableSize();
 *   windowHandle()->resize(availableSize.width() * 0.7, availableSize.height() * 0.5); // default size
 *   KWindowConfig::restoreWindowSize(windowHandle(), KSharedConfig::openConfig()->group("MyDialog"));
 *   resize(windowHandle()->size()); // workaround for QTBUG-40584
 * @endcode
 *
 * @note the group must be set before calling
 *
 * @param window The window to restore size.
 * @param config The config group to read from.
 * @since 5.0.
 */
KCONFIGGUI_EXPORT void restoreWindowSize(QWindow *window, const KConfigGroup &config);

/**
 * Saves the window's position either to the global or application config file.
 * This function has no effect on Wayland, where the compositor is responsible
 * for window positioning.
 *
 * @note the group must be set before calling
 *
 * @param window The window whose position to save.
 * @param config The config group to read from.
 * @param options passed to KConfigGroup::writeEntry()
 * @since 5.74
 */
KCONFIGGUI_EXPORT void saveWindowPosition(const QWindow *window, KConfigGroup &config, KConfigGroup::WriteConfigFlags options = KConfigGroup::Normal);

/**
 * Restores the window's position from the configuration.
 * This function has no effect on Wayland, where the compositor is responsible
 * for window positioning.
 *
 * @note the group must be set before calling
 *
 * @param window The window whose position to restore.
 * @param config The config group to read from.
 * @since 5.74
 */
KCONFIGGUI_EXPORT void restoreWindowPosition(QWindow *window, const KConfigGroup &config);
}
#endif // KWINDOWCONFIG_H
