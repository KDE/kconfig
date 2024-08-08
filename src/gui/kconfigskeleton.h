/*
    This file is part of KDE.
    SPDX-FileCopyrightText: 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGSKELETON_H
#define KCONFIGSKELETON_H

#include <kconfiggui_export.h>

#include <kcoreconfigskeleton.h>

#include <QColor>
#include <QFont>

/*!
 * \class KConfigSkeleton
 * \inmodule KConfigGui
 *
 * \brief Class for handling preferences settings for an application.
 *
 * This class extends KCoreConfigSkeleton by support for GUI types.
 */
class KCONFIGGUI_EXPORT KConfigSkeleton : public KCoreConfigSkeleton
{
    Q_OBJECT
public:
    /*!
     * \inmodule KConfigGui
     * Class for handling a color preferences item.
     */
    class KCONFIGGUI_EXPORT ItemColor : public KConfigSkeletonGenericItem<QColor>
    {
    public:
        /*! */
        ItemColor(const QString &_group, const QString &_key, QColor &reference, const QColor &defaultValue = QColor(128, 128, 128));

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;
    };

    /*!
     * \inmodule KConfigGui
     * Class for handling a font preferences item.
     */
    class KCONFIGGUI_EXPORT ItemFont : public KConfigSkeletonGenericItem<QFont>
    {
    public:
        /*! */
        ItemFont(const QString &_group, const QString &_key, QFont &reference, const QFont &defaultValue = QFont());

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;
    };

public:
    /*!
     * Constructor.
     *
     * \a configname name of config file. If no name is given, the default
     * config file as returned by KSharedConfig::openConfig() is used.
     */
    explicit KConfigSkeleton(const QString &configname = QString(), QObject *parent = nullptr);

    /*!
     * Constructor.
     *
     * \a config configuration object to use.
     */
    explicit KConfigSkeleton(KSharedConfig::Ptr config, QObject *parent = nullptr);

    /*!
     * Register an item of type QColor.
     *
     * \a name Name used to identify this setting. Names must be unique.
     *
     * \a reference Pointer to the variable, which is set by read()
     * calls and read by save() calls.
     *
     * \a defaultValue Default value, which is used when the config file
     * does not yet contain the key of this item.
     *
     * \a key Key used in config file. If \a key is a null string, \a name is used as key.
     *
     * Returns the created item
     */
    ItemColor *addItemColor(const QString &name, QColor &reference, const QColor &defaultValue = QColor(128, 128, 128), const QString &key = QString());

    /*!
     * Register an item of type QFont.
     *
     * \a name Name used to identify this setting. Names must be unique.
     *
     * \a reference Pointer to the variable, which is set by read()
     * calls and read by save() calls.
     *
     * \a defaultValue Default value, which is used when the config file
     * does not yet contain the key of this item.
     *
     * \a key Key used in config file. If \a key is a null string, \a name is used as key.
     *
     * Returns the created item
     */
    ItemFont *addItemFont(const QString &name, QFont &reference, const QFont &defaultValue = QFont(), const QString &key = QString());
};

#endif
