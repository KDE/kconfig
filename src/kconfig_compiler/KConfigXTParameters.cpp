/* This file is part of the KDE libraries
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

#include "KConfigXTParameters.h"

// TODO: Remove this.
#undef QT_NO_CAST_FROM_ASCII

#include <QDebug>
#include <QFileInfo>

namespace
{
QTextStream cout(stdout);
QTextStream cerr(stderr);
}

KConfigXTParameters::KConfigXTParameters(const QString &codegenFilename)
{
    if (!codegenFilename.endsWith(QLatin1String(".kcfgc"))) {
        cerr << "Codegen options file must have extension .kcfgc" << endl;
        exit(1);
    }

    baseName = QFileInfo(codegenFilename).fileName();
    baseName = baseName.left(baseName.length() - 6);

    // Configure the compiler with some settings
    QSettings codegenConfig(codegenFilename, QSettings::IniFormat);

    nameSpace = codegenConfig.value(QStringLiteral("NameSpace")).toString();
    className = codegenConfig.value(QStringLiteral("ClassName")).toString();
    if (className.isEmpty()) {
        cerr << "Class name missing" << endl;
        exit(1);
    }
    inherits = codegenConfig.value(QStringLiteral("Inherits")).toString();
    if (inherits.isEmpty()) {
        inherits = QStringLiteral("KConfigSkeleton");
    }
    visibility = codegenConfig.value(QStringLiteral("Visibility")).toString();
    if (!visibility.isEmpty()) {
        visibility += QLatin1Char(' ');
    }
    parentInConstructor = codegenConfig.value(QStringLiteral("ParentInConstructor"), false).toBool();
    forceStringFilename = codegenConfig.value(QStringLiteral("ForceStringFilename"), false).toBool();
    singleton = codegenConfig.value(QStringLiteral("Singleton"), false).toBool();
    staticAccessors = singleton;
    customAddons = codegenConfig.value(QStringLiteral("CustomAdditions"), false).toBool();
    memberVariables = codegenConfig.value(QStringLiteral("MemberVariables")).toString();
    dpointer = (memberVariables == QLatin1String("dpointer"));
    headerIncludes = codegenConfig.value(QStringLiteral("IncludeFiles"), QStringList()).toStringList();
    sourceIncludes = codegenConfig.value(QStringLiteral("SourceIncludeFiles"), QStringList()).toStringList();
    mutators = codegenConfig.value(QStringLiteral("Mutators"), QStringList()).toStringList();
    allMutators = ((mutators.count() == 1) && (mutators.at(0).toLower() == QLatin1String("true")));
    itemAccessors = codegenConfig.value(QStringLiteral("ItemAccessors"), false).toBool();
    setUserTexts = codegenConfig.value(QStringLiteral("SetUserTexts"), false).toBool();
    defaultGetters = codegenConfig.value(QStringLiteral("DefaultValueGetters"), QStringList()).toStringList();
    allDefaultGetters = (defaultGetters.count() == 1) && (defaultGetters.at(0).toLower() == QLatin1String("true"));
    notifiers = codegenConfig.value(QStringLiteral("Notifiers"), QStringList()).toStringList();
    allNotifiers = ((notifiers.count() == 1) && (notifiers.at(0).toLower() == QLatin1String("true")));
    globalEnums = codegenConfig.value(QStringLiteral("GlobalEnums"), false).toBool();
    useEnumTypes = codegenConfig.value(QStringLiteral("UseEnumTypes"), false).toBool();
    const QString trString = codegenConfig.value(QStringLiteral("TranslationSystem")).toString().toLower();
    generateProperties = codegenConfig.value(QStringLiteral("GenerateProperties"), false).toBool();
    if (trString == QLatin1String("kde")) {
        translationSystem = KdeTranslation;
        translationDomain = codegenConfig.value(QStringLiteral("TranslationDomain")).toString();
    } else {
        if (!trString.isEmpty() && trString != QLatin1String("qt")) {
            cerr << "Unknown translation system, falling back to Qt tr()" << endl;
        }
        translationSystem = QtTranslation;
    }
    qCategoryLoggingName = codegenConfig.value(QStringLiteral("CategoryLoggingName"), QString()).toString();
    headerExtension = codegenConfig.value(QStringLiteral("HeaderExtension"), QStringLiteral("h")).toString();
    sourceExtension = codegenConfig.value(QStringLiteral("SourceExtension"), QStringLiteral("cpp")).toString();
}
