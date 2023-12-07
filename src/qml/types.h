// SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include <QQmlEngine>

#include <kauthorized.h>

struct KAuthorizedForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(KAuthorized)
    QML_SINGLETON
    QML_FOREIGN(KAuthorized)
};
