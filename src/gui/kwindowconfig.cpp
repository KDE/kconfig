/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2012 Benjamin Port <benjamin.port@ben2367.fr>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kwindowconfig.h"

#include <QScreen>
#include <QWindow>

static const char s_initialSizePropertyName[] = "_kconfig_initial_size";
static const char s_initialScreenSizePropertyName[] = "_kconfig_initial_screen_size";

void KWindowConfig::saveWindowSize(const QWindow *window, KConfigGroup &config, KConfigGroup::WriteConfigFlags options)
{
    // QWindow::screen() shouldn't return null, but it sometimes does due to bugs.
    if (!window || !window->screen()) {
        return;
    }
    const QRect desk = window->screen()->geometry();

    const QSize sizeToSave = window->size();
    const bool isMaximized = window->windowState() & Qt::WindowMaximized;

    const QString screenMaximizedString(QStringLiteral("Window-Maximized %1x%2").arg(desk.height()).arg(desk.width()));
    // Save size only if window is not maximized
    if (!isMaximized) {
        const QSize defaultSize(window->property(s_initialSizePropertyName).toSize());
        const QSize defaultScreenSize(window->property(s_initialScreenSizePropertyName).toSize());
        const bool sizeValid = defaultSize.isValid() && defaultScreenSize.isValid();
        if (!sizeValid || (sizeValid && (defaultSize != sizeToSave || defaultScreenSize != desk.size()))) {
            const QString wString(QStringLiteral("Width %1").arg(desk.width()));
            const QString hString(QStringLiteral("Height %1").arg(desk.height()));
            config.writeEntry(wString, sizeToSave.width(), options);
            config.writeEntry(hString, sizeToSave.height(), options);
        }
    }
    if ((isMaximized == false) && !config.hasDefault(screenMaximizedString)) {
        config.revertToDefault(screenMaximizedString);
    } else {
        config.writeEntry(screenMaximizedString, isMaximized, options);
    }

}

void KWindowConfig::restoreWindowSize(QWindow *window, const KConfigGroup &config)
{
    if (!window) {
        return;
    }

    const QRect desk = window->screen()->geometry();

    const int width = config.readEntry(QStringLiteral("Width %1").arg(desk.width()), window->size().width());
    const int height = config.readEntry(QStringLiteral("Height %1").arg(desk.height()), window->size().height());
    const bool isMaximized = config.readEntry(QStringLiteral("Window-Maximized %1x%2").arg(desk.height()).arg(desk.width()), false);

    // Check default size
    const QSize defaultSize(window->property(s_initialSizePropertyName).toSize());
    const QSize defaultScreenSize(window->property(s_initialScreenSizePropertyName).toSize());
    if (!defaultSize.isValid() || !defaultScreenSize.isValid()) {
        window->setProperty(s_initialSizePropertyName, window->size());
        window->setProperty(s_initialScreenSizePropertyName, desk.size());
    }

    // If window is maximized set maximized state and in all case set the size
    window->resize(width, height);
    if (isMaximized) {
        window->setWindowState(Qt::WindowMaximized);
    }
}
