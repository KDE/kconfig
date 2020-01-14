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

#ifndef KCONFIGHEADERGENERATOR_H
#define KCONFIGHEADERGENERATOR_H

#include "KConfigCodeGeneratorBase.h"
#include "KConfigCommonStructs.h"

#include <QString>
#include <QList>

class KConfigXTParameters;
class CfgEntry;
class QTextStream;
struct ParseResult;

class KConfigHeaderGenerator : public KConfigCodeGeneratorBase {
public:
    KConfigHeaderGenerator(
        const QString& inputFile,
        const QString& baseDir,
        const KConfigXTParameters &parameters, 
        ParseResult &parseResult);

        void start() override;

private:
    void startHeaderGuards();
    void endHeaderGuards();

    void implementEnums();
    void implementChoiceEnums(const CfgEntry *entry, const CfgEntry::Choices &choices);
    void implementValueEnums(const CfgEntry *entry, const QStringList &values);

    void doClassDefinition();
    void createHeaders();
    void createDPointer();
    void createNonDPointerHelpers();

    void createConstructor();
    void createDestructor();
    void createForwardDeclarations();
    void createSingleton();
    void createSignals();

    void createSetters(const CfgEntry *entry);
    void createItemAcessors(const CfgEntry *entry, const QString& returnType);
    void createGetters(const CfgEntry *entry, const QString& returnType);
    void createProperties(const CfgEntry *entry, const QString& returnType);
    void createDefaultValueMember(const CfgEntry *entry);
};

#endif
