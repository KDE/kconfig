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

#include "KCFGXmlParser.h"
#include "KConfigXTParameters.h"

#include <QDomAttr>
#include <QDomElement>
#include <QDomNode>
#include <QFile>
#include <QList>
#include <QStringList>
#include <QTextStream>

namespace
{
QTextStream cout(stdout);
QTextStream cerr(stderr);
}

//TODO: Move preprocessDefault to Header / CPP implementation.
// it makes no sense for a parser to process those values and generate code.

static void preProcessDefault(QString &defaultValue, const QString &name,
                              const QString &type,
                              const CfgEntry::Choices &choices,
                              QString &code, const KConfigXTParameters &cfg)
{
    if (type == QLatin1String("String") && !defaultValue.isEmpty()) {
        defaultValue = literalString(defaultValue);

    } else if (type == QLatin1String("Path") && !defaultValue.isEmpty()) {
        defaultValue = literalString(defaultValue);
    } else if (type == QLatin1String("Url") && !defaultValue.isEmpty()) {
        // Use fromUserInput in order to support absolute paths and absolute urls, like KDE4's KUrl(QString) did.
        defaultValue = QLatin1String("QUrl::fromUserInput( ") + literalString(defaultValue) + QLatin1Char(')');
    } else if ((type == QLatin1String("UrlList") || type == QLatin1String("StringList") || type == QLatin1String("PathList")) && !defaultValue.isEmpty()) {
        QTextStream cpp(&code, QIODevice::WriteOnly | QIODevice::Append);
        if (!code.isEmpty()) {
            cpp << endl;
        }

        if (type == QLatin1String("UrlList")) {
            cpp << "  QList<QUrl> default" << name << ";" << endl;
        } else {
            cpp << "  QStringList default" << name << ";" << endl;
        }
        const QStringList defaults = defaultValue.split(QLatin1Char(','));
        QStringList::ConstIterator it;
        for (it = defaults.constBegin(); it != defaults.constEnd(); ++it) {
            cpp << "  default" << name << ".append( ";
            if (type == QLatin1String("UrlList")) {
                cpp << "QUrl::fromUserInput(";
            }
            cpp << "QString::fromUtf8( \"" << *it << "\" ) ";
            if (type == QLatin1String("UrlList")) {
                cpp << ") ";
            }
            cpp << ");" << endl;
        }
        defaultValue = QLatin1String("default") + name;

    } else if (type == QLatin1String("Color") && !defaultValue.isEmpty()) {
        const QRegularExpression colorRe(QRegularExpression::anchoredPattern(
                                         QStringLiteral("\\d+,\\s*\\d+,\\s*\\d+(,\\s*\\d+)?")));

        if (colorRe.match(defaultValue).hasMatch()) {
            defaultValue = QLatin1String("QColor( ") + defaultValue + QLatin1String(" )");
        } else {
            defaultValue = QLatin1String("QColor( \"") + defaultValue + QLatin1String("\" )");
        }

    } else if (type == QLatin1String("Enum")) {
        QList<CfgEntry::Choice>::ConstIterator it;
        for (it = choices.choices.constBegin(); it != choices.choices.constEnd(); ++it) {
            if ((*it).name == defaultValue) {
                if (cfg.globalEnums && choices.name().isEmpty()) {
                    defaultValue.prepend(choices.prefix);
                } else {
                    defaultValue.prepend(enumTypeQualifier(name, choices) + choices.prefix);
                }
                break;
            }
        }

    } else if (type == QLatin1String("IntList")) {
        QTextStream cpp(&code, QIODevice::WriteOnly | QIODevice::Append);
        if (!code.isEmpty()) {
            cpp << endl;
        }

        cpp << "  QList<int> default" << name << ";" << endl;
        if (!defaultValue.isEmpty()) {
            const QStringList defaults = defaultValue.split(QLatin1Char(','));
            QStringList::ConstIterator it;
            for (it = defaults.constBegin(); it != defaults.constEnd(); ++it) {
                cpp << "  default" << name << ".append( " << *it << " );"
                    << endl;
            }
        }
        defaultValue = QLatin1String("default") + name;
    }
}

static QString dumpNode(const QDomNode &node)
{
    QString msg;
    QTextStream s(&msg, QIODevice::WriteOnly);
    node.save(s, 0);

    msg = msg.simplified();
    if (msg.length() > 40) {
        return msg.left(37) + QLatin1String("...");
    }
    return msg;
}

void KCFGXmlParser::readParameterFromEntry(CfgEntry &readEntry, const QDomElement &e)
{
    readEntry.param = e.attribute(QStringLiteral("name"));
    readEntry.paramType = e.attribute(QStringLiteral("type"));
    
    if (readEntry.param.isEmpty()) {
        cerr << "Parameter must have a name: " << dumpNode(e) << endl;
        exit (1);
    }
    
    if (readEntry.paramType.isEmpty()) {
        cerr << "Parameter must have a type: " << dumpNode(e) << endl;
        exit(1);
    }

    if ((readEntry.paramType == QLatin1String("Int")) || (readEntry.paramType == QLatin1String("UInt"))) {
        bool ok;
        readEntry.paramMax = e.attribute(QStringLiteral("max")).toInt(&ok);
        if (!ok) {
            cerr << "Integer parameter must have a maximum (e.g. max=\"0\"): " << dumpNode(e) << endl;
            exit(1);
        }
    } else if (readEntry.paramType == QLatin1String("Enum")) {
        for (QDomElement e2 = e.firstChildElement(); !e2.isNull(); e2 = e2.nextSiblingElement()) {
            if (e2.tagName() == QLatin1String("values")) {
                for (QDomElement e3 = e2.firstChildElement(); !e3.isNull(); e3 = e3.nextSiblingElement()) {
                    if (e3.tagName() == QLatin1String("value")) {
                        readEntry.paramValues.append(e3.text());
                    }
                }
                break;
            }
        }
        if (readEntry.paramValues.isEmpty()) {
            cerr << "No values specified for parameter '" << readEntry.param << "'." << endl;
            exit(1);
        }
        readEntry.paramMax = readEntry.paramValues.count() - 1;
    } else {
        cerr << "Parameter '" << readEntry.param << "' has type " << readEntry.paramType 
            << " but must be of type int, uint or Enum." << endl;
        exit(1);
    }
}

bool KCFGXmlParser::hasDefaultCode(CfgEntry &readEntry, const QDomElement &element)
{
    for (QDomElement e = element.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        if (e.attribute(QStringLiteral("param")).isEmpty()) {
            if (e.attribute(QStringLiteral("code")) == QLatin1String("true")) {
                return true;
            }
        }
    }
    return false;
}


void KCFGXmlParser::readChoicesFromEntry(CfgEntry &readEntry, const QDomElement &e)
{
    QList<CfgEntry::Choice> chlist;
    for (QDomElement e2 = e.firstChildElement(); !e2.isNull(); e2 = e2.nextSiblingElement()) {
        if (e2.tagName() != QLatin1String("choice")) {
            continue;
        }
        CfgEntry::Choice choice;
        choice.name = e2.attribute(QStringLiteral("name"));
        if (choice.name.isEmpty()) {
            cerr << "Tag <choice> requires attribute 'name'." << endl;
        }
        for (QDomElement e3 = e2.firstChildElement(); !e3.isNull(); e3 = e3.nextSiblingElement()) {
            if (e3.tagName() == QLatin1String("label")) {
                choice.label = e3.text();
                choice.context = e3.attribute(QStringLiteral("context"));
            }
            if (e3.tagName() == QLatin1String("tooltip")) {
                choice.toolTip = e3.text();
                choice.context = e3.attribute(QStringLiteral("context"));
            }
            if (e3.tagName() == QLatin1String("whatsthis")) {
                choice.whatsThis = e3.text();
                choice.context = e3.attribute(QStringLiteral("context"));
            }
        }
        chlist.append(choice);
    }
    
    QString name = e.attribute(QStringLiteral("name"));
    QString prefix = e.attribute(QStringLiteral("prefix"));

    readEntry.choices = CfgEntry::Choices(chlist, name, prefix);
}

void KCFGXmlParser::readGroupElements(CfgEntry &readEntry, const QDomElement &element)
{
    for (QDomElement e = element.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        QString tag = e.tagName();
        if (tag == QLatin1String("label")) {
            readEntry.label = e.text();
            readEntry.labelContext = e.attribute(QStringLiteral("context"));
        } else if (tag == QLatin1String("tooltip")) {
            readEntry.toolTip = e.text();
            readEntry.toolTipContext = e.attribute(QStringLiteral("context"));
        } else if (tag == QLatin1String("whatsthis")) {
            readEntry.whatsThis = e.text();
            readEntry.whatsThisContext = e.attribute(QStringLiteral("context"));
        } else if (tag == QLatin1String("min")) {
            readEntry.min = e.text();
        } else if (tag == QLatin1String("max")) {
            readEntry.max = e.text();
        } else if (tag == QLatin1String("code")) {
            readEntry.code = e.text();
        } else if (tag == QLatin1String("parameter")) {
            readParameterFromEntry(readEntry, e);
        } else if (tag == QLatin1String("default")) {
            if (e.attribute(QStringLiteral("param")).isEmpty()) {
                readEntry.defaultValue = e.text();
            }
        } else if (tag == QLatin1String("choices")) {
            readChoicesFromEntry(readEntry, e);
        } else if (tag == QLatin1String("emit")) {
            Signal signal;
            signal.name = e.attribute(QStringLiteral("signal"));
            readEntry.signalList.append(signal);
        }
    }
}

void KCFGXmlParser::createChangedSignal(CfgEntry &readEntry)
{
    if (cfg.generateProperties && (cfg.allMutators || cfg.mutators.contains(readEntry.name))) {
        Signal s;
        s.name = changeSignalName(readEntry.name);
        s.modify = true;
        readEntry.signalList.append(s);
    }
}

void KCFGXmlParser::validateNameAndKey(CfgEntry &readEntry, const QDomElement &element) 
{
    bool nameIsEmpty = readEntry.name.isEmpty();
    if (nameIsEmpty && readEntry.key.isEmpty()) {
        cerr << "Entry must have a name or a key: " << dumpNode(element) << endl;
        exit (1);
    }

    if (readEntry.key.isEmpty()) {
        readEntry.key = readEntry.name;
    }

    if (nameIsEmpty) {
        readEntry.name = readEntry.key;
        readEntry.name.remove(QLatin1Char(' '));
    } else if (readEntry.name.contains(QLatin1Char(' '))) {
        cout << "Entry '" << readEntry.name << "' contains spaces! <name> elements can not contain spaces!" << endl;
        readEntry.name.remove(QLatin1Char(' '));
    }

    if (readEntry.name.contains(QStringLiteral("$("))) {
        if (readEntry.param.isEmpty()) {
            cerr << "Name may not be parameterized: " << readEntry.name << endl;
            exit (1);
        }
    } else {
        if (!readEntry.param.isEmpty()) {
            cerr << "Name must contain '$(" << readEntry.param << ")': " << readEntry.name << endl;
            exit (1);
        }
    }
}

void KCFGXmlParser::readParamDefaultValues(CfgEntry &readEntry, const QDomElement &element)
{
    if (readEntry.param.isEmpty()) {
        return;
    }
    // Adjust name
    readEntry.paramName = readEntry.name;

    readEntry.name.remove(QStringLiteral("$(") + readEntry.param + QLatin1Char(')'));
    // Lookup defaults for indexed entries
    for (int i = 0; i <= readEntry.paramMax; i++) {
        readEntry.paramDefaultValues.append(QString());
    }

    for (QDomElement e = element.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        QString tag = e.tagName();
        if (tag != QLatin1String("default")) {
            continue;
        }
        QString index = e.attribute(QStringLiteral("param"));
        if (index.isEmpty()) {
            continue;
        }

        bool ok;
        int i = index.toInt(&ok);
        if (!ok) {
            i = readEntry.paramValues.indexOf(index);
            if (i == -1) {
                cerr << "Index '" << index << "' for default value is unknown." << endl;
                exit (1);
            }
        }

        if ((i < 0) || (i > readEntry.paramMax)) {
            cerr << "Index '" << i << "' for default value is out of range [0, " << readEntry.paramMax << "]." << endl;
            exit (1);
        }

        QString tmpDefaultValue = e.text();

        if (e.attribute(QStringLiteral("code")) != QLatin1String("true")) {
            preProcessDefault(tmpDefaultValue, readEntry.name, readEntry.type, readEntry.choices, readEntry.code, cfg);
        }

        readEntry.paramDefaultValues[i] = tmpDefaultValue;
    }
}

CfgEntry *KCFGXmlParser::parseEntry(const QString &group, const QDomElement &element)
{
    CfgEntry readEntry;
    readEntry.type = element.attribute(QStringLiteral("type"));
    readEntry.name = element.attribute(QStringLiteral("name"));
    readEntry.key = element.attribute(QStringLiteral("key"));
    readEntry.hidden = element.attribute(QStringLiteral("hidden")) == QLatin1String("true");;
    readEntry.group = group;

    const bool nameIsEmpty = readEntry.name.isEmpty();

    readGroupElements(readEntry, element);

    createChangedSignal(readEntry);
    validateNameAndKey(readEntry, element);

    if (readEntry.label.isEmpty()) {
        readEntry.label = readEntry.key;
    }

    if (readEntry.type.isEmpty()) {
        readEntry.type = QStringLiteral("String");    // XXX : implicit type might be bad
    }

    readParamDefaultValues(readEntry, element);

    if (!mValidNameRegexp.match(readEntry.name).hasMatch()) {
        if (nameIsEmpty)
            cerr << "The key '" << readEntry.key << "' can not be used as name for the entry because "
                 "it is not a valid name. You need to specify a valid name for this entry." << endl;
        else {
            cerr << "The name '" << readEntry.name << "' is not a valid name for an entry." << endl;
        }
        exit (1);
    }

    if (mAllNames.contains(readEntry.name)) {
        if (nameIsEmpty)
            cerr << "The key '" << readEntry.key << "' can not be used as name for the entry because "
                 "it does not result in a unique name. You need to specify a unique name for this entry." << endl;
        else {
            cerr << "The name '" << readEntry.name << "' is not unique." << endl;
        }
        exit (1);
    }

    mAllNames.append(readEntry.name);

    if (!hasDefaultCode(readEntry, element)) {
        // TODO: Move all the options to CfgEntry.
        preProcessDefault(readEntry.defaultValue, readEntry.name, readEntry.type, readEntry.choices, readEntry.code, cfg);
    }

    // TODO: Try to Just return the CfgEntry we populated instead of 
    // creating another one to fill the code.
    CfgEntry *result = new CfgEntry();
    result->group = readEntry.group;
    result->type = readEntry.type;
    result->key = readEntry.key;
    result->name = readEntry.name;
    result->labelContext = readEntry.labelContext;
    result->label = readEntry.label;
    result->toolTipContext = readEntry.toolTipContext;
    result->toolTip = readEntry.toolTip;
    result->whatsThisContext = readEntry.whatsThisContext;
    result->whatsThis = readEntry.whatsThis;
    result->code = readEntry.code;
    result->defaultValue = readEntry.defaultValue;
    result->choices = readEntry.choices;
    result->signalList = readEntry.signalList;
    result->hidden = readEntry.hidden;

    if (!readEntry.param.isEmpty()) {
        result->param = readEntry.param;
        result->paramName = readEntry.paramName;
        result->paramType = readEntry.paramType;
        result->paramValues = readEntry.paramValues;
        result->paramDefaultValues = readEntry.paramDefaultValues;
        result->paramMax = readEntry.paramMax;
    }
    result->min = readEntry.min;
    result->max = readEntry.max;

    return result;
}

// TODO: Change the name of the config variable.
KCFGXmlParser::KCFGXmlParser(const KConfigXTParameters &cfg, const QString& inputFileName)
    : cfg(cfg), mInputFileName(inputFileName)
{
    mValidNameRegexp.setPattern(QRegularExpression::anchoredPattern(QStringLiteral("[a-zA-Z_][a-zA-Z0-9_]*")));
}
 
void KCFGXmlParser::start()
{
    QFile input(mInputFileName);
    QDomDocument doc;
    QString errorMsg;
    int errorRow;
    int errorCol;
    if (!doc.setContent(&input, &errorMsg, &errorRow, &errorCol)) {
        cerr << "Unable to load document." << endl;
        cerr << "Parse error in " << mInputFileName << ", line " << errorRow << ", col " << errorCol << ": " << errorMsg << endl;
        exit (1);
    }
    
    QDomElement cfgElement = doc.documentElement();
    if (cfgElement.isNull()) {
        cerr << "No document in kcfg file" << endl;
        exit (1);
    }

    for (QDomElement element = cfgElement.firstChildElement(); !element.isNull(); element = element.nextSiblingElement()) {
        QString tag = element.tagName();

        if (tag == QLatin1String("include")) {
            readIncludeTag(element);
        } else if (tag == QLatin1String("kcfgfile")) {
            readKcfgfileTag(element);
        } else if (tag == QLatin1String("group")) {
            readGroupTag(element);
        } else if (tag == QLatin1String("signal")) {
            readSignalTag(element);
        }
    }
}

ParseResult KCFGXmlParser::getParseResult() const
{
    return mParseResult;
}

void KCFGXmlParser::readIncludeTag(const QDomElement &e)
{
    QString includeFile = e.text();
    if (!includeFile.isEmpty()) {
        mParseResult.includes.append(includeFile);
    }
}

void KCFGXmlParser::readGroupTag(const QDomElement &e)
{
    QString group = e.attribute(QStringLiteral("name"));
    if (group.isEmpty()) {
        cerr << "Group without name" << endl;
        exit (1);
    }

    for (QDomElement e2 = e.firstChildElement(); !e2.isNull(); e2 = e2.nextSiblingElement()) {
        if (e2.tagName() != QLatin1String("entry")) {
            continue;
        }
        CfgEntry *entry = parseEntry(group, e2);
        if (entry) {
            mParseResult.entries.append(entry);
        } else {
            cerr << "Can not parse entry." << endl;
            exit (1);
        }
    }
}

void KCFGXmlParser::readKcfgfileTag(const QDomElement &e)
{
    mParseResult.cfgFileName = e.attribute(QStringLiteral("name"));
    mParseResult.cfgFileNameArg = e.attribute(QStringLiteral("arg")).toLower() == QLatin1String("true");
    for (QDomElement e2 = e.firstChildElement(); !e2.isNull(); e2 = e2.nextSiblingElement()) {
        if (e2.tagName() == QLatin1String("parameter")) {
            Param p;
            p.name = e2.attribute(QStringLiteral("name"));
            p.type = e2.attribute(QStringLiteral("type"));
            if (p.type.isEmpty()) {
                p.type = QStringLiteral("String");
            }
            mParseResult.parameters.append(p);
        }
    }
}

void KCFGXmlParser::readSignalTag(const QDomElement &e)
{
    QString signalName = e.attribute(QStringLiteral("name"));
    if (signalName.isEmpty()) {
        cerr << "Signal without name." << endl;
        exit (1);
    }
    Signal theSignal;
    theSignal.name = signalName;

    for (QDomElement e2 = e.firstChildElement(); !e2.isNull(); e2 = e2.nextSiblingElement()) {
        if (e2.tagName() == QLatin1String("argument")) {
            Param argument;
            argument.type = e2.attribute(QStringLiteral("type"));
            if (argument.type.isEmpty()) {
                cerr << "Signal argument without type." << endl;
                exit (1);
            }
            argument.name= e2.text();
            theSignal.arguments.append(argument);
        } else if (e2.tagName() == QLatin1String("label")) {
            theSignal.label = e2.text();
        }
    }
    
    mParseResult.signalList.append(theSignal);
}
