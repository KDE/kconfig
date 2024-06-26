// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef KWINDOWSTATESAVER_QUICK_H
#define KWINDOWSTATESAVER_QUICK_H

#include <QQmlEngine>

/**
 * @brief Creates a @c KWindowStateSaver in QML, and assigns it to the window it's parented to.
 *
 * Example:
 *
 * import org.kde.config as KConfig
 *
 * Kirigami.ApplicationWindow {
 *     id: root
 *
 *     title: i18n("My Window")
 *
 *     KConfig.KWindowStateSaver {}
 * }
 */
class KWindowStateSaverQuick : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    QML_NAMED_ELEMENT(KWindowStateSaver)
    Q_INTERFACES(QQmlParserStatus)

public:
    void classBegin() override;
    void componentComplete() override;
};

#endif
