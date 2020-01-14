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

#include "KConfigSourceGenerator.h"
#include "KConfigCommonStructs.h"


KConfigSourceGenerator::KConfigSourceGenerator(
    const QString& inputFile,
    const QString& baseDir,
    const KConfigXTParameters &cfg,
    ParseResult &parseResult)
    : KConfigCodeGeneratorBase(inputFile, baseDir, baseDir + cfg.baseName + QLatin1Char('.') + cfg.sourceExtension, cfg, parseResult)
{
}

void KConfigSourceGenerator::start() 
{
    KConfigCodeGeneratorBase::start();
    stream << endl;
    createHeaders();

    if (!cfg.nameSpace.isEmpty()) {
        stream << "using namespace " << cfg.nameSpace << ";";
        stream << endl << endl;
    }

    createPrivateDPointerImplementation();
    createSingletonImplementation();
    createPreamble();
    doConstructor();
    doGetterSetterDPointerMode();
    createDefaultValueGetterSetter();
    createDestructor();
    createNonModifyingSignalsHelper();
    createSignalFlagsHandler();
    includeMoc();
}

void KConfigSourceGenerator::createHeaders()
{
    QString headerName = cfg.baseName + QLatin1Char('.') + cfg.headerExtension;

    // TODO: Make addQuotes return a string instead of replacing it inplace.
    addQuotes(headerName);

    addHeaders({ headerName });
    stream << endl;

    addHeaders(cfg.sourceIncludes);
    if (cfg.setUserTexts && cfg.translationSystem == KConfigXTParameters::KdeTranslation) {
        addHeaders({QStringLiteral("klocalizedstring.h")});
        stream << endl;
    }

    // Header required by singleton implementation
    if (cfg.singleton) {
        addHeaders({QStringLiteral("qglobal.h"), QStringLiteral("QFile")});

        // HACK: Add single line to fix test.
        if (cfg.singleton && parseResult.cfgFileNameArg) {
            stream << endl;
        }
    }

    if (cfg.singleton && parseResult.cfgFileNameArg) {
        addHeaders({QStringLiteral("QDebug")});
    }

    if (cfg.singleton) {
        stream << endl;
    }
}

void KConfigSourceGenerator::createPrivateDPointerImplementation()
{
    // private class implementation
    if (!cfg.dpointer) {
        return;
    }

    QString group;
    beginNamespaces();
    stream << "class " << cfg.className << "Private" << endl;
    stream << "{" << endl;
    stream << "  public:" << endl;

    // Create Members
    for (auto *entry : qAsConst(parseResult.entries)) {
        if (entry->group != group) {
            group = entry->group;
            stream << endl;
            stream << "    // " << group << endl;
        }
        stream << "    " << cppType(entry->type) << " " << varName(entry->name, cfg);
        if (!entry->param.isEmpty()) {
            stream << QStringLiteral("[%1]").arg(entry->paramMax + 1);
        }
        stream << ";" << endl;
    }
    stream << endl << "    // items" << endl;

    // Create Items.
    for (auto *entry : qAsConst(parseResult.entries)) {
        const QString declType = entry->signalList.isEmpty()
                ? QString(cfg.inherits + QStringLiteral("::Item") + itemType(entry->type))
                : QStringLiteral("KConfigCompilerSignallingItem");

        stream << "    " << declType << " *" << itemVar( entry, cfg );
        if (!entry->param.isEmpty()) {
            stream << QStringLiteral("[%1]").arg(entry->paramMax + 1);
        }
        stream << ";" << endl;
    }

    if (parseResult.hasNonModifySignals) {
        stream << "    uint " << varName(QStringLiteral("settingsChanged"), cfg) << ";" << endl;
    }

    stream << "};" << endl << endl;
    endNamespaces();
}

void KConfigSourceGenerator::createSingletonImplementation()
{
    // Singleton implementation
    if (!cfg.singleton) {
        return;
    }

    beginNamespaces();
    stream << "class " << cfg.className << "Helper" << endl;
    stream << '{' << endl;
    stream << "  public:" << endl;
    stream << "    " << cfg.className << "Helper() : q(nullptr) {}" << endl;
    stream << "    ~" << cfg.className << "Helper() { delete q; }" << endl;
    stream << "    " << cfg.className << "Helper(const " << cfg.className << "Helper&) = delete;" << endl;
    stream << "    " << cfg.className << "Helper& operator=(const " << cfg.className << "Helper&) = delete;" << endl;
    stream << "    " << cfg.className << " *q;" << endl;
    stream << "};" << endl;
    endNamespaces();

    stream << "Q_GLOBAL_STATIC(" << cfg.className << "Helper, s_global" << cfg.className << ")" << endl;

    stream << cfg.className << " *" << cfg.className << "::self()" << endl;
    stream << "{" << endl;
    if (parseResult.cfgFileNameArg) {
        stream << "  if (!s_global" << cfg.className << "()->q)" << endl;
        stream << "     qFatal(\"you need to call " << cfg.className << "::instance before using\");" << endl;
    } else {
        stream << "  if (!s_global" << cfg.className << "()->q) {" << endl;
        stream << "    new " << cfg.className << ';' << endl;
        stream << "    s_global" << cfg.className << "()->q->read();" << endl;
        stream << "  }" << endl << endl;
    }
    stream << "  return s_global" << cfg.className << "()->q;" << endl;
    stream << "}" << endl << endl;

    if (parseResult.cfgFileNameArg) {
        auto instance = [this] (const QString &type, const QString &arg, bool isString) {
            stream << "void " << cfg.className << "::instance(" << type << " " << arg << ")" << endl;
            stream << "{" << endl;
            stream << "  if (s_global" << cfg.className << "()->q) {" << endl;
            stream << "     qDebug() << \"" << cfg.className << "::instance called after the first use - ignoring\";" << endl;
            stream << "     return;" << endl;
            stream << "  }" << endl;
            stream << "  new " << cfg.className << "(";
            if (isString) {
                stream << "KSharedConfig::openConfig(" << arg << ")";
            } else {
                stream << "std::move(" << arg << ")";
            }
            stream << ");" << endl;
            stream << "  s_global" << cfg.className << "()->q->read();" << endl;
            stream << "}" << endl << endl;
        };
        instance(QStringLiteral("const QString&"), QStringLiteral("cfgfilename"), true);
        instance(QStringLiteral("KSharedConfig::Ptr"), QStringLiteral("config"), false);
    }
}

void KConfigSourceGenerator::createPreamble()
{
    QString cppPreamble;
    for (const auto entry : qAsConst(parseResult.entries)) {
        if (entry->paramValues.isEmpty()) {
            continue;
        }
        
        cppPreamble += QStringLiteral("const char* const ") + cfg.className + QStringLiteral("::") + enumName(entry->param);
        cppPreamble += cfg.globalEnums 
            ? QStringLiteral("ToString[] = { \"") + entry->paramValues.join(QStringLiteral("\", \"")) + QStringLiteral("\" };\n")
            : QStringLiteral("::enumToString[] = { \"") + entry->paramValues.join(QStringLiteral("\", \"")) + QStringLiteral("\" };\n");
    }

    if (!cppPreamble.isEmpty()) {
        stream << cppPreamble << endl;
    }
}

void KConfigSourceGenerator::createConstructorParameterList()
{
    if (parseResult.cfgFileNameArg) {
        if (!cfg.forceStringFilename) {
            stream << " KSharedConfig::Ptr config";
        } else {
            stream << " const QString& config";
        }
        stream << (parseResult.parameters.isEmpty() ? "" : ",");
    }

    for (QList<Param>::ConstIterator it = parseResult.parameters.constBegin();
            it != parseResult.parameters.constEnd(); ++it) {
        if (it != parseResult.parameters.constBegin()) {
            stream << ",";
        }
        stream << " " << param((*it).type) << " " << (*it).name;
    }

    if (cfg.parentInConstructor) {
        if (parseResult.cfgFileNameArg || !parseResult.parameters.isEmpty()) {
            stream << ",";
        }
        stream << " QObject *parent";
    }

}

void KConfigSourceGenerator::createParentConstructorCall()
{
    stream << cfg.inherits << "(";
    if (!parseResult.cfgFileName.isEmpty()) {
        stream << " QStringLiteral( \"" << parseResult.cfgFileName << "\" ";
    }
    if (parseResult.cfgFileNameArg) {
        if (! cfg.forceStringFilename) {
            stream << " std::move( config ) ";
        } else {
            stream << " config ";
        }
    }
    if (!parseResult.cfgFileName.isEmpty()) {
        stream << ") ";
    }
    stream << ")" << endl;
}

void KConfigSourceGenerator::createInitializerList()
{
    for (const auto &parameter : qAsConst(parseResult.parameters)) {
        stream << "  , mParam" << parameter.name << "(" << parameter.name << ")" << endl;
    }

    if (parseResult.hasNonModifySignals && !cfg.dpointer) {
        stream << "  , " << varName(QStringLiteral("settingsChanged"), cfg) << "(0)" << endl;
    }
}

void KConfigSourceGenerator::createEnums(const CfgEntry *entry)
{
    if (entry->type != QLatin1String("Enum")) {
        return;
    }
    stream << "  QList<" << cfg.inherits << "::ItemEnum::Choice> values" << entry->name << ";" << endl;
    
    for (const auto &choice : qAsConst(entry->choices.choices)) {
        stream << "  {" << endl;
        stream << "    " << cfg.inherits << "::ItemEnum::Choice choice;" << endl;
        stream << "    choice.name = QStringLiteral(\"" << choice.name << "\");" << endl;
        if (cfg.setUserTexts) {
            if (!choice.label.isEmpty()) {
                stream << "    choice.label = "
                    << translatedString(cfg, choice.label, choice.context)
                    << ";" << endl;
            }
            if (!choice.toolTip.isEmpty()) {
                stream << "    choice.toolTip = "
                    << translatedString(cfg, choice.toolTip, choice.context)
                    << ";" << endl;
            }
            if (!choice.whatsThis.isEmpty()) {
                stream << "    choice.whatsThis = "
                    << translatedString(cfg, choice.whatsThis, choice.context)
                    << ";" << endl;
            }
        }
        stream << "    values" << entry->name << ".append( choice );" << endl;
        stream << "  }" << endl;
    }
}

void KConfigSourceGenerator::createNormalEntry(const CfgEntry *entry, const QString& key)
{
   stream << "  " << itemPath(entry, cfg) << " = "
        << newItem(entry, key, entry->defaultValue, cfg) << endl;

    if (!entry->min.isEmpty()) {
        stream << "  " << itemPath(entry, cfg) << "->setMinValue(" << entry->min << ");" << endl;
    }
    if (!entry->max.isEmpty()) {
        stream << "  " << itemPath(entry, cfg) << "->setMaxValue(" << entry->max << ");" << endl;
    }

    if (cfg.setUserTexts) {
        stream << userTextsFunctions(entry, cfg);
    }

    if (cfg.allNotifiers || cfg.notifiers.contains(entry->name)) {
        stream << "  " << itemPath(entry, cfg) << "->setWriteFlags(KConfigBase::Notify);" << endl;
    }

    stream << "  addItem( " << itemPath(entry, cfg);
    QString quotedName = entry->name;
    addQuotes(quotedName);
    if (quotedName != key) {
        stream << ", QStringLiteral( \"" << entry->name << "\" )";
    }
    stream << " );" << endl;
}

void KConfigSourceGenerator::createIndexedEntry(const CfgEntry *entry, const QString& key)
{
    for (int i = 0; i <= entry->paramMax; i++) {
        QString itemVarStr(itemPath(entry, cfg) + QStringLiteral("[%1]").arg(i));

        QString defaultStr = !entry->paramDefaultValues[i].isEmpty() ? entry->paramDefaultValues[i]
                           : !entry->defaultValue.isEmpty() ? paramString(entry->defaultValue, entry, i)
                           : defaultValue(entry->type);
        
        stream << "  " << itemVarStr << " = "
            << newItem(entry, paramString(key, entry, i), defaultStr, cfg, QStringLiteral("[%1]").arg(i)) << endl;

        if (cfg.setUserTexts) {
            stream << userTextsFunctions(entry, cfg, itemVarStr, entry->paramName);
        }

        // Make mutators for enum parameters work by adding them with $(..) replaced by the
        // param name. The check for isImmutable in the set* functions doesn't have the param
        // name available, just the corresponding enum value (int), so we need to store the
        // param names in a separate static list!.
        const bool isEnum = entry->paramType == QLatin1String("Enum");
        const QString arg = isEnum ? entry->paramValues[i] : QString::number(i);

        QString paramName = entry->paramName;

        stream << "  addItem( " << itemVarStr << ", QStringLiteral( \"";
        stream << paramName.replace(QStringLiteral("$(") + entry->param + QLatin1Char(')'), QLatin1String("%1")).arg( arg );
        stream << "\" ) );" << endl;
    }
}

void KConfigSourceGenerator::handleCurrentGroupChange(const CfgEntry *entry)
{
    if (entry->group == mCurrentGroup) {
        return;
    }

    // HACK: This fixes one spacing line in the diff. Remove this in the future and adapt the testcases.
    static bool first = true;
    if (!entry->group.isEmpty()) {
        if (!first) {
            stream << endl;
        }
        first = false;
    }

    mCurrentGroup = entry->group;
    stream << "  setCurrentGroup( " << paramString(mCurrentGroup, parseResult.parameters) << " );";
    stream << endl << endl;
}

void KConfigSourceGenerator::doConstructor()
{
    // Constructor
    stream << cfg.className << "::" << cfg.className << "(";
    createConstructorParameterList();
    stream << " )" << endl;
    stream << "  : ";
    createParentConstructorCall();
    createInitializerList();

    stream << "{" << endl;

    if (cfg.parentInConstructor) {
        stream << "  setParent(parent);" << endl;
    }

    if (cfg.dpointer) {
        stream << "  d = new " << cfg.className << "Private;" << endl;
        if (parseResult.hasNonModifySignals) {
            stream << "  " << varPath(QStringLiteral("settingsChanged"), cfg) << " = 0;" << endl;
        }
    }

    // Needed in case the singleton class is used as baseclass for
    // another singleton.
    if (cfg.singleton) {
        stream << "  Q_ASSERT(!s_global" << cfg.className << "()->q);" << endl;
        stream << "  s_global" << cfg.className << "()->q = this;" << endl;
    }

    if (!parseResult.signalList.isEmpty()) {
        // this cast to base-class pointer-to-member is valid C++
        // https://stackoverflow.com/questions/4272909/is-it-safe-to-upcast-a-method-pointer-and-use-it-with-base-class-pointer/
        stream << "  KConfigCompilerSignallingItem::NotifyFunction notifyFunction ="
            << " static_cast<KConfigCompilerSignallingItem::NotifyFunction>(&"
            << cfg.className << "::itemChanged);" << endl;

        stream << endl;
    }

    for (auto *entry : qAsConst(parseResult.entries)) {
        handleCurrentGroupChange(entry);

        const QString key = paramString(entry->key, parseResult.parameters);
        if (!entry->code.isEmpty()) {
            stream << entry->code << endl;
        }
        createEnums(entry);

        if (!cfg.dpointer) {
            stream << itemDeclaration(entry, cfg);
        }

        if (entry->param.isEmpty()) {
            createNormalEntry(entry, key);
        } else {
            createIndexedEntry(entry, key);
        }
    }

    stream << "}" << endl << endl;
}

void KConfigSourceGenerator::createGetterDPointerMode(const CfgEntry *entry)
{
    // Accessor
    if (cfg.useEnumTypes && entry->type == QLatin1String("Enum")) {
        stream << enumType(entry, cfg.globalEnums);
    } else {
        stream << cppType(entry->type);
    }

    stream << " " << getFunction(entry->name, cfg.className) << "(";
    if (!entry->param.isEmpty()) {
        stream << " " << cppType(entry->paramType) << " i ";
    }
    stream << ")" << Const << endl;

    // function body inline only if not using dpointer
    // for BC mode
    startScope();
    // HACK: Fix memberAccessorBody
    stream << "  " << memberAccessorBody(entry, cfg.globalEnums);
    endScope();
    stream << endl;
}

void KConfigSourceGenerator::createSetterDPointerMode(const CfgEntry *entry)
{
    // Manipulator
    if (!(cfg.allMutators || cfg.mutators.contains(entry->name))) {
        return;
    }

    stream << "void " << setFunction(entry->name, cfg.className) << "( ";
    if (!entry->param.isEmpty()) {
        stream << cppType(entry->paramType) << " i, ";
    }

    if (cfg.useEnumTypes && entry->type == QLatin1String("Enum")) {
        stream << enumType(entry, cfg.globalEnums);
    } else {
        stream << param(entry->type);
    }
    stream << " v )" << endl;

    // function body inline only if not using dpointer
    // for BC mode
    startScope();
    memberMutatorBody(entry);
    endScope();
    stream << endl;
}

void KConfigSourceGenerator::createItemGetterDPointerMode(const CfgEntry *entry)
{
    // Item accessor
    if (!cfg.itemAccessors) {
        return;
    }
    stream << endl;
    stream << cfg.inherits << "::Item" << itemType(entry->type) << " *"
        << getFunction(entry->name, cfg.className) << "Item(";
    if (!entry->param.isEmpty()) {
        stream << " " << cppType(entry->paramType) << " i ";
    }
    stream << ")" << endl;
    startScope();
    stream << "  " << itemAccessorBody(entry, cfg);
    endScope();
}

void KConfigSourceGenerator::doGetterSetterDPointerMode()
{
    if (!cfg.dpointer) {
        return;
    }

    // setters and getters go in Cpp if in dpointer mode
    for (auto *entry : qAsConst(parseResult.entries)) {
        createSetterDPointerMode(entry);
        createGetterDPointerMode(entry);
        createItemGetterDPointerMode(entry);
        stream << endl;
    }
}

void KConfigSourceGenerator::createDefaultValueGetterSetter()
{
    // default value getters always go in Cpp
    for (auto *entry : qAsConst(parseResult.entries)) {
        QString n = entry->name;
        QString t = entry->type;

        // Default value Accessor, as "helper" function
        if ((cfg.allDefaultGetters || cfg.defaultGetters.contains(n)) && !entry->defaultValue.isEmpty()) {
            stream << cppType(t) << " " << getDefaultFunction(n, cfg.className) << "_helper(";
            if (!entry->param.isEmpty()) {
                stream << " " << cppType(entry->paramType) << " i ";
            }
            stream << ")" << Const << endl;
            startScope();
            stream << memberGetDefaultBody(entry) << endl;
            endScope();
            stream << endl;
        }
    }
}

void KConfigSourceGenerator::createDestructor()
{
    stream << cfg.className << "::~" << cfg.className << "()" << endl;
    startScope();
    if (cfg.dpointer) {
        stream << "  delete d;" << endl;
    }
    if (cfg.singleton) {
        stream << "  s_global" << cfg.className << "()->q = nullptr;" << endl;
    }
    endScope();
    stream << endl;
}

void KConfigSourceGenerator::createNonModifyingSignalsHelper()
{
    if (!parseResult.hasNonModifySignals) {
        return;
    }
    stream << "bool " << cfg.className << "::" << "usrSave()" << endl;
    startScope();
    stream << "  const bool res = " << cfg.inherits << "::usrSave();" << endl;
    stream << "  if (!res) return false;" << endl << endl;
    for (const Signal &signal : qAsConst(parseResult.signalList)) {
        if (signal.modify) {
            continue;
        }

        stream << "  if ( " << varPath(QStringLiteral("settingsChanged"), cfg) << " & " << signalEnumName(signal.name) << " )" << endl;
        stream << "    Q_EMIT " << signal.name << "(";
        QList<Param>::ConstIterator it, itEnd = signal.arguments.constEnd();
        for (it = signal.arguments.constBegin(); it != itEnd;) {
            Param argument = *it;
            bool cast = false;
            if (cfg.useEnumTypes && argument.type == QLatin1String("Enum")) {
                for (int i = 0, end = parseResult.entries.count(); i < end; ++i) {
                    if (parseResult.entries.at(i)->name == argument.name) {
                        stream << "static_cast<" << enumType(parseResult.entries.at(i), cfg.globalEnums) << ">(";
                        cast = true;
                        break;
                    }
                }
            }
            stream << varPath(argument.name, cfg);
            if (cast) {
                stream << ")";
            }
            if (++it != itEnd) {
                stream << ", ";
            }
        }

        stream << ");" << endl;
    }

    stream << "  " << varPath(QStringLiteral("settingsChanged"), cfg) << " = 0;" << endl;
    stream << "  return true;" << endl;
    endScope();
}

void KConfigSourceGenerator::createSignalFlagsHandler() 
{
    if (parseResult.signalList.isEmpty()) {
        return;
    }

    stream << endl;
    stream << "void " << cfg.className << "::" << "itemChanged(quint64 flags) {" << endl;
    if (parseResult.hasNonModifySignals)
        stream << "  " << varPath(QStringLiteral("settingsChanged"), cfg) << " |= flags;" << endl;

    if (!parseResult.signalList.isEmpty())
        stream << endl;

    for (const Signal &signal : qAsConst(parseResult.signalList)) {
        if (signal.modify) {
            stream << "  if ( flags & " << signalEnumName(signal.name) << " ) {" << endl;
            stream << "    Q_EMIT " << signal.name << "();" << endl;
            stream << "  }" << endl;
        }
    }

    stream << "}" << endl;
}

void KConfigSourceGenerator::includeMoc() {
    const QString mocFileName = cfg.baseName + QStringLiteral(".moc");

    if (parseResult.signalList.count() || cfg.generateProperties) {
        // Add includemoc if they are signals defined.
        stream << endl;
        stream << "#include \"" << mocFileName << "\"" << endl;
        stream << endl;
    }
}
