/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2012 Benjamin Port <benjamin.port@ben2367.fr>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kwindowconfig.h"

#include <QGuiApplication>
#include <QScreen>
#include <QWindow>

static const char s_initialSizePropertyName[] = "_kconfig_initial_size";
static const char s_initialScreenSizePropertyName[] = "_kconfig_initial_screen_size";

// Convenience function to get a space-separated list of all connected screens
static QString allConnectedScreens()
{
    QStringList names;
    const auto screens = QGuiApplication::screens();
    names.reserve(screens.length());
    for (auto screen : screens) {
#ifdef Q_OS_WIN
        // QScreen::name() returns garbage on Windows; see https://bugreports.qt.io/browse/QTBUG-74317
        // So we use the screens' serial numbers to identify them instead
        names << screen->serialNumber();
#else
        names << screen->name();
#endif
    }
    return names.join(QLatin1Char(' '));
}

// Convenience function to get an appropriate config file key under which to
// save window size, position, or maximization information.
static QString configFileString(const QRect &desk, const QString &key)
{
    // We include resolution data to also save data on a per-resolution basis
    const QString returnString = QStringLiteral("%1 %2 %3x%4").arg(allConnectedScreens(), key, QString::number(desk.width()), QString::number(desk.height()));
    return returnString;
}

// Convenience function for "window is maximized" string
static QString screenMaximizedString(const QRect &desk)
{
    return configFileString(desk, QStringLiteral("Window-Maximized"));
}
// Convenience function for window width string
static QString windowWidthString(const QRect &desk)
{
    return configFileString(desk, QStringLiteral("Width"));
}
// Convenience function for window height string
static QString windowHeightString(const QRect &desk)
{
    return configFileString(desk, QStringLiteral("Height"));
}
// Convenience function for window X position string
static QString windowXPositionString(const QRect &desk)
{
    return configFileString(desk, QStringLiteral("XPosition"));
}
// Convenience function for window Y position string
static QString windowYPositionString(const QRect &desk)
{
    return configFileString(desk, QStringLiteral("YPosition"));
}

void KWindowConfig::saveWindowSize(const QWindow *window, KConfigGroup &config, KConfigGroup::WriteConfigFlags options)
{
    // QWindow::screen() shouldn't return null, but it sometimes does due to bugs.
    if (!window || !window->screen()) {
        return;
    }
    const QRect desk = window->screen()->geometry();

    const QSize sizeToSave = window->size();
    const bool isMaximized = window->windowState() & Qt::WindowMaximized;

    // Save size only if window is not maximized
    if (!isMaximized) {
        const QSize defaultSize(window->property(s_initialSizePropertyName).toSize());
        const QSize defaultScreenSize(window->property(s_initialScreenSizePropertyName).toSize());
        const bool sizeValid = defaultSize.isValid() && defaultScreenSize.isValid();
        if (!sizeValid || (sizeValid && (defaultSize != sizeToSave || defaultScreenSize != desk.size()))) {
            config.writeEntry(windowWidthString(desk), sizeToSave.width(), options);
            config.writeEntry(windowHeightString(desk), sizeToSave.height(), options);
            // Don't keep the maximized string in the file since the window is
            // no longer maximized at this point
            config.deleteEntry(screenMaximizedString(desk));
        }
    }
    if ((isMaximized == false) && !config.hasDefault(screenMaximizedString(desk))) {
        config.revertToDefault(screenMaximizedString(desk));
    } else {
        config.writeEntry(screenMaximizedString(desk), isMaximized, options);
    }
}

void KWindowConfig::restoreWindowSize(QWindow *window, const KConfigGroup &config)
{
    if (!window) {
        return;
    }

    const QRect desk = window->screen()->geometry();

    // Fall back to non-per-screen-arrangement info if it's available but
    // per-screen-arrangement information is not
    // TODO: Remove in KF6 or maybe even KF5.80 or something. It really only needs
    // to be here to transition existing users once they upgrade from 5.73 -> 5.74
    const int fallbackWidth = config.readEntry(QStringLiteral("Width %1").arg(desk.width()), window->size().width());
    const int fallbackHeight = config.readEntry(QStringLiteral("Height %1").arg(desk.height()), window->size().height());

    const int width = config.readEntry(windowWidthString(desk), fallbackWidth);
    const int height = config.readEntry(windowHeightString(desk), fallbackHeight);
    const bool isMaximized = config.readEntry(configFileString(desk, QStringLiteral("Window-Maximized")), false);

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

void KWindowConfig::saveWindowPosition(const QWindow *window, KConfigGroup &config, KConfigGroup::WriteConfigFlags options)
{
    // On Wayland, the compositor is solely responsible for window positioning,
    // So this needs to be a no-op
    if (!window || QGuiApplication::platformName() == QLatin1String{"wayland"}) {
        return;
    }

    // If the window is maximized, saving the position will only serve to mis-position
    // it once de-maximized, so let's not do that
    if (window->windowState() & Qt::WindowMaximized) {
        return;
    }

    const QRect desk = window->screen()->geometry();
    config.writeEntry(windowXPositionString(desk), window->x(), options);
    config.writeEntry(windowYPositionString(desk), window->y(), options);
}

void KWindowConfig::restoreWindowPosition(QWindow *window, const KConfigGroup &config)
{
    // On Wayland, the compositor is solely responsible for window positioning,
    // So this needs to be a no-op
    if (!window || QGuiApplication::platformName() == QLatin1String{"wayland"}) {
        return;
    }

    const QRect desk = window->screen()->geometry();
    const bool isMaximized = config.readEntry(configFileString(desk, QStringLiteral("Window-Maximized")), false);

    // Don't need to restore position if the window was maximized
    if (isMaximized) {
        window->setWindowState(Qt::WindowMaximized);
        return;
    }

    // Fall back to non-per-resolution info if it's available but
    // per-resolution information is not
    // TODO: Remove in KF6 or maybe even KF5.85 or something. It really only needs
    // to be here to transition existing users once they upgrade from 5.78 -> 5.79
    const int fallbackXPosition = config.readEntry(QStringLiteral("%1 XPosition %2").arg(allConnectedScreens(), QString::number(desk.width())), -1);
    const int fallbackYPosition = config.readEntry(QStringLiteral("%1 YPosition %2").arg(allConnectedScreens(), QString::number(desk.height())), -1);
    const int xPos = config.readEntry(windowXPositionString(desk), fallbackXPosition);
    const int yPos = config.readEntry(windowYPositionString(desk), fallbackYPosition);

    if (xPos == -1 || yPos == -1) {
        return;
    }

    window->setX(xPos);
    window->setY(yPos);
}
