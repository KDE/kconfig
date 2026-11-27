/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2003 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KRECENTFILESMODEL_P_H
#define KRECENTFILESMODEL_P_H

#include "krecentfilesmodel.h"

class KRecentFilesModelPrivate
{
    Q_DECLARE_PUBLIC(KRecentFilesModel)

public:
    explicit KRecentFilesModelPrivate(KRecentFilesModel *parent)
        : q_ptr(parent)
    {
    }

    int m_maxItems = 10;

    struct RecentFileInfo {
        QString title;
        QUrl url;
        QMimeType mimeType;
    };
    mutable std::vector<RecentFileInfo> m_recentFiles;

    KRecentFilesModel *const q_ptr;
};

#endif // KRECENTFILESMODEL_P_H
