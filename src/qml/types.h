// SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef KCONFIGTYPES_H
#define KCONFIGTYPES_H

#include <QQmlEngine>

#include <kauthorized.h>
#include <kconfigskeleton.h>
#include <kcoreconfigskeleton.h>

/*!
 * \qmltype KAuthorized
 * \inqmlmodule org.kde.config
 * \nativetype KAuthorized
 *
 */

struct KAuthorizedForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(KAuthorized)
    QML_SINGLETON
    QML_FOREIGN(KAuthorized)
};

struct KCoreConfigSkeletonForeign {
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(KCoreConfigSkeleton)
};

struct KConfigSkeletonForeign {
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(KConfigSkeleton)
};

#endif
