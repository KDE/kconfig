/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1997-1999 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfigbase.h"

#include "kconfiggroup.h"

#include <QString>

bool KConfigBase::hasGroup(const QString &group) const
{
    return hasGroupImpl(group);
}

#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 241)
bool KConfigBase::hasGroup(const char *group) const
{
    return hasGroupImpl(QString::fromUtf8(group));
}
#endif

#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 241)
bool KConfigBase::hasGroup(const QByteArray &group) const
{
    return hasGroupImpl(QString::fromUtf8(group));
}
#endif

#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 241)
KConfigGroup KConfigBase::group(const QByteArray &b)
{
    return groupImpl(QString::fromUtf8(b));
}
#endif

KConfigGroup KConfigBase::group(const QString &str)
{
    return groupImpl(str);
}

#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 241)
KConfigGroup KConfigBase::group(const char *str)
{
    return groupImpl(QString::fromUtf8(str));
}
#endif

#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 241)
const KConfigGroup KConfigBase::group(const QByteArray &b) const
{
    return groupImpl(QString::fromUtf8(b));
}
#endif

const KConfigGroup KConfigBase::group(const QString &s) const
{
    return groupImpl(s);
}

#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 241)
const KConfigGroup KConfigBase::group(const char *s) const
{
    return groupImpl(QString::fromUtf8(s));
}
#endif

#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 241)
void KConfigBase::deleteGroup(const QByteArray &group, WriteConfigFlags flags)
{
    deleteGroupImpl(QString::fromUtf8(group), flags);
}
#endif

void KConfigBase::deleteGroup(const QString &group, WriteConfigFlags flags)
{
    deleteGroupImpl(group, flags);
}

#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 241)
void KConfigBase::deleteGroup(const char *group, WriteConfigFlags flags)
{
    deleteGroupImpl(QString::fromUtf8(group), flags);
}
#endif

#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 241)
bool KConfigBase::isGroupImmutable(const QByteArray &aGroup) const
{
    return isGroupImmutableImpl(QString::fromUtf8(aGroup));
}
#endif

bool KConfigBase::isGroupImmutable(const QString &aGroup) const
{
    return isGroupImmutableImpl(aGroup);
}

#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 241)
bool KConfigBase::isGroupImmutable(const char *aGroup) const
{
    return isGroupImmutableImpl(QString::fromUtf8(aGroup));
}
#endif

KConfigBase::~KConfigBase()
{
}

KConfigBase::KConfigBase()
{
}

void KConfigBase::virtual_hook(int, void *)
{
}
