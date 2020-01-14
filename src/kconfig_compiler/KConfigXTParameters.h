/*
    This file is part of KDE.
    Copyright (C) 2020 Tomaz Cananbrava (tcanabrava@kde.org)
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 Waldo Bastian <bastian@kde.org>
    Copyright (c) 2003 Zack Rusin <zack@kde.org>
    Copyright (c) 2006 Michaël Larouche <michael.larouche@kdemail.net>
    Copyright (c) 2008 Allen Winter <winter@kde.org>
    Copyright (C) 2020 Tomaz Cananbrava (tcanabrava@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KCOFIGXTPARAMETERS_H
#define KCOFIGXTPARAMETERS_H

#include <QSettings>
#include <QString>
#include <QStringList>

/**
   Configuration Compiler Configuration
*/
class KConfigXTParameters
{
public:
    KConfigXTParameters(const QString &codegenFilename);

public:
    enum TranslationSystem {
        QtTranslation,
        KdeTranslation
    };

    // These are read from the .kcfgc configuration file
    QString nameSpace;     // The namespace for the class to be generated
    QString className;     // The class name to be generated
    QString inherits;      // The class the generated class inherits (if empty, from KConfigSkeleton)
    QString visibility;
    bool parentInConstructor; // The class has the optional parent parameter in its constructor
    bool forceStringFilename;
    bool singleton;        // The class will be a singleton
    bool staticAccessors;  // provide or not static accessors
    bool customAddons;
    QString memberVariables;
    QStringList headerIncludes;
    QStringList sourceIncludes;
    QStringList mutators;
    QStringList defaultGetters;
    QStringList notifiers;
    QString qCategoryLoggingName;
    QString headerExtension;
    QString sourceExtension;
    bool allMutators;
    bool setUserTexts;
    bool allDefaultGetters;
    bool dpointer;
    bool globalEnums;
    bool useEnumTypes;
    bool itemAccessors;
    bool allNotifiers;
    TranslationSystem translationSystem;
    QString translationDomain;
    bool generateProperties;
    QString baseName;
};

#endif
