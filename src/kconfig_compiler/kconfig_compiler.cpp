/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
    SPDX-FileCopyrightText: 2006 Michaël Larouche <michael.larouche@kdemail.net>
    SPDX-FileCopyrightText: 2008 Allen Winter <winter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// Compiling this file with this flag is just crazy
#undef QT_NO_CAST_FROM_ASCII

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QTextStream>
#include <QDomAttr>
#include <QRegularExpression>
#include <QStringList>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <ostream>
#include <iostream>
#include <stdlib.h>

#include "../../kconfig_version.h"
#include "KConfigParameters.h"
#include "KConfigCommonStructs.h"
#include "KConfigHeaderGenerator.h"
#include "KConfigSourceGenerator.h"
#include "KConfigXmlParser.h"

QString varName(const QString &n, const KConfigParameters &cfg)
{
    QString result;
    if (!cfg.dpointer) {
        result = QChar::fromLatin1('m') + n;
        result[1] = result[1].toUpper();
    } else {
        result = n;
        result[0] = result[0].toLower();
    }
    return result;
}

QString varPath(const QString &n, const KConfigParameters &cfg)
{
    QString result;
    if (cfg.dpointer) {
        result = "d->" + varName(n, cfg);
    } else {
        result = varName(n, cfg);
    }
    return result;
}

QString enumName(const QString &n)
{
    QString result = QLatin1String("Enum") + n;
    result[4] = result[4].toUpper();
    return result;
}

QString enumName(const QString &n, const CfgEntry::Choices &c)
{
    QString result = c.name();
    if (result.isEmpty()) {
        result = QLatin1String("Enum") + n;
        result[4] = result[4].toUpper();
    }
    return result;
}

QString enumType(const CfgEntry *e, bool globalEnums)
{
    QString result = e->choices.name();
    if (result.isEmpty()) {
        result = QLatin1String("Enum") + e->name;
        if (!globalEnums) {
            result += QLatin1String("::type");
        }
        result[4] = result[4].toUpper();
    }
    return result;
}

QString enumTypeQualifier(const QString &n, const CfgEntry::Choices &c)
{
    QString result = c.name();
    if (result.isEmpty()) {
        result = QLatin1String("Enum") + n + QLatin1String("::");
        result[4] = result[4].toUpper();
    } else if (c.external()) {
        result = c.externalQualifier();
    } else {
        result.clear();
    }
    return result;
}

QString setFunction(const QString &n, const QString &className)
{
    QString result = QLatin1String("set") + n;
    result[3] = result[3].toUpper();

    if (!className.isEmpty()) {
        result = className + QLatin1String("::") + result;
    }
    return result;
}

QString changeSignalName(const QString &n)
{
    return n+QStringLiteral("Changed");
}

QString getDefaultFunction(const QString &n, const QString &className)
{
    QString result = QLatin1String("default") +  n + QLatin1String("Value");
    result[7] = result[7].toUpper();

    if (!className.isEmpty()) {
        result = className + QLatin1String("::") + result;
    }
    return result;
}

QString getFunction(const QString &n, const QString &className)
{
    QString result = n;
    result[0] = result[0].toLower();

    if (!className.isEmpty()) {
        result = className + QLatin1String("::") + result;
    }
    return result;
}

QString immutableFunction(const QString &n, const QString &className)
{
    QString result = QLatin1String("is") + n;
    result[2] = result[2].toUpper();
    result += "Immutable";

    if (!className.isEmpty()) {
        result = className + QLatin1String("::") + result;
    }
    return result;
}

void addQuotes(QString &s)
{
    if (!s.startsWith(QLatin1Char('"'))) {
        s.prepend(QLatin1Char('"'));
    }
    if (!s.endsWith(QLatin1Char('"'))) {
        s.append(QLatin1Char('"'));
    }
}

static QString quoteString(const QString &s)
{
    QString r = s;
    r.replace(QLatin1Char('\\'), QLatin1String("\\\\"));
    r.replace(QLatin1Char('\"'), QLatin1String("\\\""));
    r.remove(QLatin1Char('\r'));
    r.replace(QLatin1Char('\n'), QLatin1String("\\n\"\n\""));
    return QLatin1Char('\"') + r + QLatin1Char('\"');
}

QString literalString(const QString &s)
{
    bool isAscii = true;
    for (int i = s.length(); i--;)
        if (s[i].unicode() > 127) {
            isAscii = false;
        }

    if (isAscii) {
        return QLatin1String("QStringLiteral( ") + quoteString(s) + QLatin1String(" )");
    } else {
        return QLatin1String("QString::fromUtf8( ") + quoteString(s) + QLatin1String(" )");
    }
}


QString signalEnumName(const QString &signalName)
{
    QString result;
    result = QLatin1String("signal") + signalName;
    result[6] = result[6].toUpper();

    return result;
}


bool isUnsigned(const QString &type)
{
    if (type == QLatin1String("UInt")) {
        return true;
    }
    if (type == QLatin1String("ULongLong")) {
        return true;
    }
    return false;
}

/**
  Return parameter declaration for given type.
*/
QString param(const QString &t)
{
    const QString type = t.toLower();
    if (type == QLatin1String("string")) {
        return QStringLiteral("const QString &");
    } else if (type == QLatin1String("stringlist")) {
        return QStringLiteral("const QStringList &");
    } else if (type == QLatin1String("font")) {
        return QStringLiteral("const QFont &");
    } else if (type == QLatin1String("rect")) {
        return QStringLiteral("const QRect &");
    } else if (type == QLatin1String("size")) {
        return QStringLiteral("const QSize &");
    } else if (type == QLatin1String("color")) {
        return QStringLiteral("const QColor &");
    } else if (type == QLatin1String("point")) {
        return QStringLiteral("const QPoint &");
    } else if (type == QLatin1String("int")) {
        return QStringLiteral("int");
    } else if (type == QLatin1String("uint")) {
        return QStringLiteral("uint");
    } else if (type == QLatin1String("bool")) {
        return QStringLiteral("bool");
    } else if (type == QLatin1String("double")) {
        return QStringLiteral("double");
    } else if (type == QLatin1String("datetime")) {
        return QStringLiteral("const QDateTime &");
    } else if (type == QLatin1String("longlong")) {
        return QStringLiteral("qint64");
    } else if (type == QLatin1String("ulonglong")) {
        return QStringLiteral("quint64");
    } else if (type == QLatin1String("intlist")) {
        return QStringLiteral("const QList<int> &");
    } else if (type == QLatin1String("enum")) {
        return QStringLiteral("int");
    } else if (type == QLatin1String("path")) {
        return QStringLiteral("const QString &");
    } else if (type == QLatin1String("pathlist")) {
        return QStringLiteral("const QStringList &");
    } else if (type == QLatin1String("password")) {
        return QStringLiteral("const QString &");
    } else if (type == QLatin1String("url")) {
        return QStringLiteral("const QUrl &");
    } else if (type == QLatin1String("urllist")) {
        return QStringLiteral("const QList<QUrl> &");
    } else {
        std::cerr << "kconfig_compiler_kf5 does not support type \"" << qPrintable(type) << "\"" << std::endl;
        return QStringLiteral("QString"); //For now, but an assert would be better
    }
}

/**
  Actual C++ storage type for given type.
*/
QString cppType(const QString &t)
{
    const QString type = t.toLower();
    if (type == QLatin1String("string")) {
        return QStringLiteral("QString");
    } else if (type == QLatin1String("stringlist")) {
        return QStringLiteral("QStringList");
    } else if (type == QLatin1String("font")) {
        return QStringLiteral("QFont");
    } else if (type == QLatin1String("rect")) {
        return QStringLiteral("QRect");
    } else if (type == QLatin1String("size")) {
        return QStringLiteral("QSize");
    } else if (type == QLatin1String("color")) {
        return QStringLiteral("QColor");
    } else if (type == QLatin1String("point")) {
        return QStringLiteral("QPoint");
    } else if (type == QLatin1String("int")) {
        return QStringLiteral("int");
    } else if (type == QLatin1String("uint")) {
        return QStringLiteral("uint");
    } else if (type == QLatin1String("bool")) {
        return QStringLiteral("bool");
    } else if (type == QLatin1String("double")) {
        return QStringLiteral("double");
    } else if (type == QLatin1String("datetime")) {
        return QStringLiteral("QDateTime");
    } else if (type == QLatin1String("longlong")) {
        return QStringLiteral("qint64");
    } else if (type == QLatin1String("ulonglong")) {
        return QStringLiteral("quint64");
    } else if (type == QLatin1String("intlist")) {
        return QStringLiteral("QList<int>");
    } else if (type == QLatin1String("enum")) {
        return QStringLiteral("int");
    } else if (type == QLatin1String("path")) {
        return QStringLiteral("QString");
    } else if (type == QLatin1String("pathlist")) {
        return QStringLiteral("QStringList");
    } else if (type == QLatin1String("password")) {
        return QStringLiteral("QString");
    } else if (type == QLatin1String("url")) {
        return QStringLiteral("QUrl");
    } else if (type == QLatin1String("urllist")) {
        return QStringLiteral("QList<QUrl>");
    } else {
        std::cerr << "kconfig_compiler_kf5 does not support type \"" << qPrintable(type) << "\"" << std::endl;
        return QStringLiteral("QString"); //For now, but an assert would be better
    }
}

QString defaultValue(const QString &t)
{
    const QString type = t.toLower();
    if (type == QLatin1String("string")) {
        return QStringLiteral("\"\"");    // Use empty string, not null string!
    } else if (type == QLatin1String("stringlist")) {
        return QStringLiteral("QStringList()");
    } else if (type == QLatin1String("font")) {
        return QStringLiteral("QFont()");
    } else if (type == QLatin1String("rect")) {
        return QStringLiteral("QRect()");
    } else if (type == QLatin1String("size")) {
        return QStringLiteral("QSize()");
    } else if (type == QLatin1String("color")) {
        return QStringLiteral("QColor(128, 128, 128)");
    } else if (type == QLatin1String("point")) {
        return QStringLiteral("QPoint()");
    } else if (type == QLatin1String("int")) {
        return QStringLiteral("0");
    } else if (type == QLatin1String("uint")) {
        return QStringLiteral("0");
    } else if (type == QLatin1String("bool")) {
        return QStringLiteral("false");
    } else if (type == QLatin1String("double")) {
        return QStringLiteral("0.0");
    } else if (type == QLatin1String("datetime")) {
        return QStringLiteral("QDateTime()");
    } else if (type == QLatin1String("longlong")) {
        return QStringLiteral("0");
    } else if (type == QLatin1String("ulonglong")) {
        return QStringLiteral("0");
    } else if (type == QLatin1String("intlist")) {
        return QStringLiteral("QList<int>()");
    } else if (type == QLatin1String("enum")) {
        return QStringLiteral("0");
    } else if (type == QLatin1String("path")) {
        return QStringLiteral("\"\"");    // Use empty string, not null string!
    } else if (type == QLatin1String("pathlist")) {
        return QStringLiteral("QStringList()");
    } else if (type == QLatin1String("password")) {
        return QStringLiteral("\"\"");    // Use empty string, not null string!
    } else if (type == QLatin1String("url")) {
        return QStringLiteral("QUrl()");
    } else if (type == QLatin1String("urllist")) {
        return QStringLiteral("QList<QUrl>()");
    } else {
        std::cerr << "Error, kconfig_compiler_kf5 does not support the \"" << qPrintable(type) << "\" type!" << std::endl;
        return QStringLiteral("QString"); //For now, but an assert would be better
    }
}

QString itemType(const QString &type)
{
    QString t;

    t = type;
    t.replace(0, 1, t.left(1).toUpper());

    return t;
}

QString itemDeclaration(const CfgEntry *e, const KConfigParameters &cfg)
{
    const QString type = cfg.inherits + "::Item" + itemType(e->type);

    QString fCap = e->name;
    fCap[0] = fCap[0].toUpper();
    const QString argSuffix = (!e->param.isEmpty()) ? (QStringLiteral("[%1]").arg(e->paramMax + 1)) : QString();
    QString result;

    if (!cfg.itemAccessors && !cfg.dpointer) {
        result += "  " + (!e->signalList.isEmpty() ? QStringLiteral("KConfigCompilerSignallingItem") : type) +
            "  *item" + fCap + argSuffix + ";\n";
    }

    if (!e->signalList.isEmpty()) {
        result += "  " + type + "  *" + innerItemVar(e, cfg) + argSuffix + ";\n";
    }

    return result;
}

// returns the name of an item variable
// use itemPath to know the full path
// like using d-> in case of dpointer
QString itemVar(const CfgEntry *e, const KConfigParameters &cfg)
{
    QString result;
    if (cfg.itemAccessors) {
        if (!cfg.dpointer) {
            result = 'm' + e->name + "Item";
            result[1] = result[1].toUpper();
        } else {
            result = e->name + "Item";
            result[0] = result[0].toLower();
        }
    } else {
        result = "item" + e->name;
        result[4] = result[4].toUpper();
    }
    return result;
}

// returns the name of the local inner item if there is one
// (before wrapping with KConfigCompilerSignallingItem)
// Otherwise return itemVar()
QString innerItemVar(const CfgEntry *e, const KConfigParameters &cfg)
{
    if (e->signalList.isEmpty()) {
        return itemPath(e, cfg);
    } else {
        QString result = "innerItem" + e->name;
        result[9] = result[9].toUpper();
        return result;
    }
}

QString itemPath(const CfgEntry *e, const KConfigParameters &cfg)
{
    QString result;
    if (cfg.dpointer) {
        result = "d->" + itemVar(e, cfg);
    } else {
        result = itemVar(e, cfg);
    }
    return result;
}

QString newInnerItem(const CfgEntry *entry, const QString &key, const QString &defaultValue,
                const KConfigParameters &cfg, const QString &param) {
    QString t = "new "+ cfg.inherits + "::Item" + itemType(entry->type) + "( currentGroup(), "
            + key + ", " + varPath( entry->name, cfg ) + param;

    if (entry->type == QLatin1String("Enum")) {
        t += ", values" + entry->name;
    }
    if (!defaultValue.isEmpty()) {
        t += QLatin1String(", ") + defaultValue;
    }
    t += QLatin1String(" );");

    return t;
}

QString newItem(const CfgEntry *entry, const QString &key, const QString &defaultValue,
                const KConfigParameters &cfg, const QString &param) {

    QList<Signal> sigs = entry->signalList;
    QString t;
    if (!sigs.isEmpty()) {
        t += QLatin1String("new KConfigCompilerSignallingItem(") + innerItemVar(entry, cfg) + param;
        t += QLatin1String(", this, notifyFunction, ");
        //append the signal flags
        for (int i = 0; i < sigs.size(); ++i) {
            if (i != 0)
                t += QLatin1String(" | ");
            t += signalEnumName(sigs[i].name);
        }
        t += QLatin1String(");");
    } else {
        t += newInnerItem(entry, key, defaultValue, cfg, param);
    }
    return t;
}

QString paramString(const QString &s, const CfgEntry *e, int i)
{
    QString result = s;
    QString needle = "$(" + e->param + ')';
    if (result.contains(needle)) {
        QString tmp;
        if (e->paramType == QLatin1String("Enum")) {
            tmp = e->paramValues.at(i);
        } else {
            tmp = QString::number(i);
        }

        result.replace(needle, tmp);
    }
    return result;
}

QString paramString(const QString &group, const QList<Param> &parameters)
{
    QString paramString = group;
    QString arguments;
    int i = 1;
    for (QList<Param>::ConstIterator it = parameters.constBegin();
            it != parameters.constEnd(); ++it) {
        if (paramString.contains("$(" + (*it).name + ')')) {
            const QString tmp = QStringLiteral("%%1").arg(i++);
            paramString.replace("$(" + (*it).name + ')', tmp);
            arguments += ".arg( mParam" + (*it).name + " )";
        }
    }
    if (arguments.isEmpty()) {
        return "QStringLiteral( \"" + group + "\" )";
    }

    return "QStringLiteral( \"" + paramString + "\" )" + arguments;
}

QString translatedString(const KConfigParameters &cfg, const QString &string, const QString &context, const QString &param, const QString &paramValue)
{
    QString result;

    switch (cfg.translationSystem) {
    case KConfigParameters::QtTranslation:
        if (!context.isEmpty()) {
            result += "/*: " + context + " */ QCoreApplication::translate(\"";
        } else {
            result += QLatin1String("QCoreApplication::translate(\"");
        }
        result += cfg.className + "\", ";
        break;

    case KConfigParameters::KdeTranslation:
        if (!cfg.translationDomain.isEmpty() && !context.isEmpty()) {
            result += "i18ndc(" + quoteString(cfg.translationDomain) + ", " + quoteString(context) + ", ";
        } else if (!cfg.translationDomain.isEmpty()) {
            result += "i18nd(" + quoteString(cfg.translationDomain) + ", ";
        } else if (!context.isEmpty()) {
            result += "i18nc(" + quoteString(context) + ", ";
        } else {
            result += QLatin1String("i18n(");
        }
        break;
    }

    if (!param.isEmpty()) {
        QString resolvedString = string;
        resolvedString.replace("$(" + param + ')', paramValue);
        result += quoteString(resolvedString);
    } else {
        result += quoteString(string);
    }

    result += ')';

    return result;
}

/* int i is the value of the parameter */
QString userTextsFunctions(const CfgEntry *e, const KConfigParameters &cfg, QString itemVarStr, const QString &i)
{
    QString txt;
    if (itemVarStr.isNull()) {
        itemVarStr = itemPath(e, cfg);
    }
    if (!e->label.isEmpty()) {
        txt += "  " + itemVarStr + "->setLabel( ";
        txt += translatedString(cfg, e->label, e->labelContext, e->param, i);
        txt += QLatin1String(" );\n");
    }
    if (!e->toolTip.isEmpty()) {
        txt += "  " + itemVarStr + "->setToolTip( ";
        txt += translatedString(cfg, e->toolTip, e->toolTipContext, e->param, i);
        txt += QLatin1String(" );\n");
    }
    if (!e->whatsThis.isEmpty()) {
        txt += "  " + itemVarStr + "->setWhatsThis( ";
        txt += translatedString(cfg, e->whatsThis, e->whatsThisContext, e->param, i);
        txt += QLatin1String(" );\n");
    }
    return txt;
}


// returns the member mutator implementation
// which should go in the h file if inline
// or the cpp file if not inline
//TODO: Fix add Debug Method, it should also take the debug string.
void addDebugMethod(QTextStream &out, const KConfigParameters &cfg, const QString &n)
{
    if (cfg.qCategoryLoggingName.isEmpty()) {
       out << "  qDebug() << \"" << setFunction(n);
    } else {
       out << "  qCDebug(" << cfg.qCategoryLoggingName << ") << \"" << setFunction(n);
    }
}


// returns the member get default implementation
// which should go in the h file if inline
// or the cpp file if not inline
QString memberGetDefaultBody(const CfgEntry *e)
{
    QString result = e->code;
    QTextStream out(&result, QIODevice::WriteOnly);
    out << '\n';

    if (!e->param.isEmpty()) {
        out << "  switch (i) {\n";
        for (int i = 0; i <= e->paramMax; ++i) {
            if (!e->paramDefaultValues[i].isEmpty()) {
                out << "  case " << i << ": return " << e->paramDefaultValues[i] << ";\n";
            }
        }
        QString defaultValue = e->defaultValue;

        out << "  default:\n";
        out << "    return " << defaultValue.replace("$(" + e->param + ')', QLatin1String("i")) << ";\n";
        out << "  }\n";
    } else {
        out << "  return " << e->defaultValue << ';';
    }

    return result;
}

// returns the item accesor implementation
// which should go in the h file if inline
// or the cpp file if not inline
QString itemAccessorBody(const CfgEntry *e, const KConfigParameters &cfg)
{
    QString result;
    QTextStream out(&result, QIODevice::WriteOnly);

    out << "return " << itemPath(e, cfg);
    if (!e->param.isEmpty()) {
        out << "[i]";
    }
    out << ";\n";

    return result;
}

//indents text adding X spaces per line
QString indent(QString text, int spaces)
{
    QString result;
    QTextStream out(&result, QIODevice::WriteOnly);
    QTextStream in(&text, QIODevice::ReadOnly);
    QString currLine;
    while (!in.atEnd()) {
        currLine = in.readLine();
        if (!currLine.isEmpty())
            for (int i = 0; i < spaces; i++) {
                out << " ";
            }
        out << currLine << '\n';
    }
    return result;
}

bool hasErrors(KConfigXmlParser &parser, const ParseResult& parseResult, const KConfigParameters &cfg)
{
    Q_UNUSED(parser)

    if (cfg.className.isEmpty()) {
        std::cerr << "Class name missing" << std::endl;
        return true;
    }

    if (cfg.singleton && !parseResult.parameters.isEmpty()) {
        std::cerr << "Singleton class can not have parameters" << std::endl;
        return true;
    }

    if (!parseResult.cfgFileName.isEmpty() && parseResult.cfgFileNameArg) {
        std::cerr << "Having both a fixed filename and a filename as argument is not possible." << std::endl;
        return true;
    }

    /* TODO: For some reason some configuration files prefer to have *no* entries
    * at all in it, and the generated code is mostly bogus as KConfigXT will not
    * handle save / load / properties, etc, nothing.
    *
    * The first of those files that I came across are qmakebuilderconfig.kcfg from the KDevelop
    * project.
    * I think we should remove the possibility of creating configuration classes from configuration
    * files that don't really have configuration in it. but I'm changing this right now to allow
    * kdevelop files to pass.
    *
    * Remove for KDE 6
    * (to make things more interesting, it failed in a code that's never used within KDevelop... )
    */
    if (parseResult.entries.isEmpty()) {
        std::cerr << "No entries." << std::endl;
        return false;
    }

    return false;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kconfig_compiler"));
    app.setApplicationVersion(QStringLiteral(KCONFIG_VERSION_STRING));

    QString inputFilename, codegenFilename;

    QCommandLineOption targetDirectoryOption(QStringList { QStringLiteral("d"), QStringLiteral("directory") },
            QCoreApplication::translate("main", "Directory to generate files in [.]"),
            QCoreApplication::translate("main", "directory"), QStringLiteral("."));

    QCommandLineOption licenseOption (
        QStringList { QStringLiteral("l"), QStringLiteral("license") },
        QCoreApplication::translate("main", "Display software license."));

    QCommandLineParser parser;

    parser.addPositionalArgument(QStringLiteral("file.kcfg"), QStringLiteral("Input kcfg XML file"));
    parser.addPositionalArgument(QStringLiteral("file.kcfgc"), QStringLiteral("Code generation options file"));

    parser.addOption(targetDirectoryOption);
    parser.addOption (licenseOption);

    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    if (parser.isSet(licenseOption)) {
        std::cout << "Copyright 2003 Cornelius Schumacher, Waldo Bastian, Zack Rusin," << std::endl;
        std::cout << "    Reinhold Kainhofer, Duncan Mac-Vicar P., Harald Fernengel" << std::endl;
        std::cout << "This program comes with ABSOLUTELY NO WARRANTY." << std::endl;
        std::cout << "You may redistribute copies of this program" << std::endl;
        std::cout << "under the terms of the GNU Library Public License." << std::endl;
        std::cout << "For more information about these matters, see the file named COPYING." << std::endl;
        return 0;
    }

    const QStringList args = parser.positionalArguments();
    if (args.count() < 2) {
        std::cerr << "Too few arguments." << std::endl;
	    return 1;
    }

    if (args.count() > 2) {
        std::cerr << "Too many arguments." << std::endl;
        return 1;
    }
    inputFilename = args.at(0);
    codegenFilename = args.at(1);

    // TODO: Transform baseDir into a helper.
    QString baseDir = parser.value(targetDirectoryOption);

#ifdef Q_OS_WIN
    if (!baseDir.endsWith('/') && !baseDir.endsWith('\\'))
#else
    if (!baseDir.endsWith('/'))
#endif
        baseDir.append("/");

    KConfigParameters cfg(codegenFilename);

    KConfigXmlParser xmlParser(cfg, inputFilename);
    
    // The Xml Parser aborts in the case of an error, so if we get
    // to parseResult, we have a working Xml file.
    xmlParser.start();

    ParseResult parseResult = xmlParser.getParseResult();

    if (hasErrors(xmlParser, parseResult, cfg)) {
        return 1;
    }

    // TODO: Move this to somewhere saner.
    for (const auto &signal : qAsConst(parseResult.signalList)) {
        parseResult.hasNonModifySignals |= !signal.modify;
    }

    // remove '.kcfg' from the name.
    const QString baseName = inputFilename.mid(0, inputFilename.size()-5);
    KConfigHeaderGenerator headerGenerator(baseName, baseDir, cfg, parseResult);
    headerGenerator.start();
    headerGenerator.save();

    KConfigSourceGenerator sourceGenerator(baseName, baseDir, cfg, parseResult);
    sourceGenerator.start();
    sourceGenerator.save();

    qDeleteAll(parseResult.entries);
}
