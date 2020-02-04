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

#ifndef KCONFIGSOURCEGENERATOR_H
#define KCONFIGSOURCEGENERATOR_H

#include "KConfigCodeGeneratorBase.h"
#include "KConfigCommonStructs.h"

#include <QString>
#include <QList>

class KConfigParameters;
class CfgEntry;
class QTextStream;
struct ParseResult;

class KConfigSourceGenerator : public KConfigCodeGeneratorBase {
public:
    KConfigSourceGenerator(
        const QString &inputFile,
        const QString &baseDir,
        const KConfigParameters &parameters,
        ParseResult &parseResult);

    void start() override;

private:
    // Those are fairly self contained functions.
    void createHeaders();
    void createPrivateDPointerImplementation();
    void createSingletonImplementation();
    void createPreamble();
    void createDestructor();
    void createConstructorParameterList();
    void createParentConstructorCall();
    void createInitializerList();
    void createDefaultValueGetterSetter();
    void createNonModifyingSignalsHelper();
    void createSignalFlagsHandler();
    void includeMoc();

    // Constructor related methods
    // the `do` methods have related helper functions that are only related
    // to it. So we can break the function into many smaller ones and create
    // logic inside of the `do` function.
    void doConstructor();
    void createEnums(const CfgEntry *entry);
    void createNormalEntry(const CfgEntry *entry, const QString &key);
    void createIndexedEntry(const CfgEntry *entry, const QString &key);
    void handleCurrentGroupChange(const CfgEntry *entry);

    void doGetterSetterDPointerMode();
    void createGetterDPointerMode(const CfgEntry *entry);
    void createImmutableGetterDPointerMode(const CfgEntry *entry);
    void createSetterDPointerMode(const CfgEntry *entry);
    void createItemGetterDPointerMode(const CfgEntry *entry);

private:
    QString mCurrentGroup;
};

#endif
