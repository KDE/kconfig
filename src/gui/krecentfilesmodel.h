/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2003 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2026 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KRECENTFILESMODEL_H
#define KRECENTFILESMODEL_H

#include <QAbstractListModel>
#include <QUrl>
#include <kconfiggui_export.h>

#include <QMimeType>

class KConfigGroup;
class KRecentFilesModelPrivate;

/*!
 * \class KRecentFilesModel
 * \inmodule KConfigGui
 *
 * \brief Recent files model.
 *
 * TODO TODO TODO
 * This class is an action to handle a recent files submenu.
 * The best way to create the action is to use KStandardAction::openRecent.
 * Then you simply need to call loadEntries on startup, saveEntries
 * on shutdown, addURL when your application loads/saves a file.
 */
class KCONFIGGUI_EXPORT KRecentFilesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int maxItems READ maxItems WRITE setMaxItems)
    Q_DECLARE_PRIVATE(KRecentFilesModel)

public:
    /*!
     * Constructs an action with the specified parent.
     *
     * \a parent The parent of this action.
     */
    explicit KRecentFilesModel(QObject *parent);

    ~KRecentFilesModel() override;

    /*!
     * \value TitleRole The title of the recent document, typically its filename
     * \value UrlRole The URL pointing to the document
     * \value DisplayPathRole A pretty-formatted path (e.g. without file:// prefix and tilde-collapsed)
     */
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        UrlRole,
    };

    /*!
     *  Returns the maximum of items in the recent files list.
     */
    int maxItems() const;

    /*!
     *  Sets the maximum of items in the recent files list.
     *  The default for this value is 10 set in the constructor.
     *
     *  If this value is lesser than the number of items currently
     *  in the recent files list the last items are deleted until
     *  the number of items are equal to the new maximum.
     *
     *  Negative values will be normalized to 0.
     */
    void setMaxItems(int maxItems);

    /*!
     *  Loads the recent files entries from a given KConfigGroup object.
     *  You can provide the name of the group used to load the entries.
     *  If the groupname is empty, entries are loaded from a group called 'RecentFiles'.
     *  Local file entries that do not exist anymore are not restored.
     *
     */
    void loadEntries(const KConfigGroup &config);

    /*!
     *  Saves the current recent files entries to a given KConfigGroup object.
     *  You can provide the name of the group used to load the entries.
     *  If the groupname is empty, entries are saved to a group called 'RecentFiles'.
     *
     */
    void saveEntries(const KConfigGroup &config);

    /*!
     * Add URL to the recent files list. This will enable this action.
     *
     * \a url The URL of the file
     *
     * \a name The user visible pretty name that appears before the URL
     *
     * \note URLs corresponding to local files in the temporary directory
     * (see QDir::tempPath()) are automatically ignored by this method.
     */
    void addUrl(const QUrl &url, const QString &name = QString());

    /*!
     * Add URL to the recent files list. This will enable this action.
     *
     * \a url The URL of the file
     *
     * \a name The user visible pretty name that appears before the URL
     *
     * \a mimeType Specify the mimeType of the file
     *
     * \note URLs corresponding to local files in the temporary directory
     * (see QDir::tempPath()) are automatically ignored by this method.
     *
     * \since 6.9
     */
    void addUrl(const QUrl &url, const QString &name, const QString &mimeType);

    void removeAt(const QModelIndex &index);

    /*!
     * Remove an URL from the recent files list.
     *
     * \a url The URL of the file
     */
    void removeUrl(const QUrl &url);

    QModelIndex indexOfUrl(const QUrl &url) const;

    /*!
     *  Retrieve a list of all URLs in the recent files list.
     */
    QList<QUrl> urls() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public Q_SLOTS:
    /*!
     * Clears the recent files list.
     * Note that there is also an action shown to the user for clearing the list.
     */
    virtual void clear();

Q_SIGNALS:
    /*!
     *  This signal gets emitted when the user selects an URL.
     *
     *  \a url The URL that the user selected.
     */
    void urlSelected(const QUrl &url);

    /*!
     *  This signal gets emitted when the user clear list.
     *  So when user store url in specific config file it can saveEntry.
     *  \since 4.3
     */
    void recentListCleared();

private:
    std::unique_ptr<KRecentFilesModelPrivate> const d_ptr;
};

#endif // KRECENTFILESMODEL_H
