/* This file is part of the KDE libraries
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

#ifndef KCFGXMLPARSER_H
#define KCFGXMLPARSER_H

#include <QDomDocument>
#include <QString>
#include <QRegularExpression>

#include "KConfigCommonStructs.h"
#include "KConfigXTParameters.h"

/* This parses the contents of a Xml file into a ParseResult Structure,
 * It also fails hard:
 * If start() succeeds, you can use the result,
 * if start() fails, the program aborts with an error message so there's
 * no possibility of generating incorrect code information.
 */
class KCFGXmlParser {
public:
    KCFGXmlParser(const KConfigXTParameters &cfg, const QString& inputFileName);

    // Start the parser and reads the contents of the inputFileName into the ParseResult Structure
    void start();

    // Get the result of the parse
    ParseResult getParseResult() const;

private:
    // creates a `somethingChanged` signal for every property
    void createChangedSignal(CfgEntry &readEntry);

    void validateNameAndKey(CfgEntry &readEntry, const QDomElement &element);

    // TODO: Use std::optional and CfgEntry (without heap allocation) for this function
    // *or* fail hard if the parse fails.
    CfgEntry *parseEntry(const QString &group, const QDomElement &element);

    // Steps
    void readIncludeTag(const QDomElement &element);
    void readGroupTag(const QDomElement &element);
    void readKcfgfileTag(const QDomElement &element);
    void readSignalTag(const QDomElement &element);

    // Those are the Entries in the Xml, that represent a parameter within the <group> </group> tag.
    void readParameterFromEntry(CfgEntry &entry, const QDomElement &element);
    bool hasDefaultCode(CfgEntry &entry, const QDomElement &element);
    void readChoicesFromEntry(CfgEntry &entry, const QDomElement &element);
    void readGroupElements(CfgEntry &entry, const QDomElement &element);
    void readParamDefaultValues(CfgEntry &entry, const QDomElement &element);

private:
    ParseResult mParseResult;
    KConfigXTParameters cfg;
    QString mInputFileName;
    QStringList mAllNames;
    QRegularExpression mValidNameRegexp;
};

#endif
