// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "kwindowstatesaverquick.h"

#include <QQuickItem>
#include <QQuickWindow>

#include <KWindowStateSaver>

void KWindowStateSaverQuick::classBegin()
{
}

void KWindowStateSaverQuick::componentComplete()
{
    auto parentItem = qobject_cast<QQuickItem *>(parent());
    Q_ASSERT(parentItem);

    auto window = qobject_cast<QWindow *>(parentItem->window());
    Q_ASSERT(window);

    new KWindowStateSaver(window, QStringLiteral("WindowState"));
}

#include "moc_kwindowstatesaverquick.cpp"