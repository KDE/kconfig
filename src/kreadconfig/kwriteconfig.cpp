/*  Write KConfig() entries - for use in shell scripts.

    Copyright (c) 2001 Red Hat, Inc.
    Copyright (c) 2001 Luís Pedro Coelho <luis_pedro@netcabo.pt>

    Programmed by Luís Pedro Coelho <luis_pedro@netcabo.pt>
    based on kreadconfig by Bernhard Rosenkraenzer <bero@redhat.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <KConfig>
#include <KConfigGroup>
#include <stdio.h>
#include <QCoreApplication>
#include <QCommandLineParser>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringLiteral("file"), QCoreApplication::translate("main", "Use <file> instead of global config"), QStringLiteral("file")));
    parser.addOption(QCommandLineOption(QStringLiteral("group"), QCoreApplication::translate("main", "Group to look in. Use repeatedly for nested groups."), QStringLiteral("group"), QStringLiteral("KDE")));
    parser.addOption(QCommandLineOption(QStringLiteral("key"), QCoreApplication::translate("main", "Key to look for"), QStringLiteral("key")));
    parser.addOption(QCommandLineOption(QStringLiteral("type"), QCoreApplication::translate("main", "Type of variable. Use \"bool\" for a boolean, otherwise it is treated as a string"), QStringLiteral("type")));
    parser.addPositionalArgument(QStringLiteral("value"), QCoreApplication::translate("main",  "The value to write. Mandatory, on a shell use '' for empty" ));

    parser.process(app);

    const QStringList groups=parser.values(QStringLiteral("group"));
    QString key=parser.value(QStringLiteral("key"));
    QString file=parser.value(QStringLiteral("file"));
    QString type=parser.value(QStringLiteral("type")).toLower();


    if (parser.positionalArguments().isEmpty()) {
        parser.showHelp(1);
    }
    QString value = parser.positionalArguments().at(0);

    KConfig *konfig;
    if (file.isEmpty())
        konfig = new KConfig(QStringLiteral( "kdeglobals"), KConfig::NoGlobals );
    else
        konfig = new KConfig( file, KConfig::NoGlobals );

    KConfigGroup cfgGroup = konfig->group(QString());
    for (const QString &grp : groups)
        cfgGroup = cfgGroup.group(grp);
    if ( konfig->accessMode() != KConfig::ReadWrite || cfgGroup.isEntryImmutable( key ) ) return 2;

    if(type==QStringLiteral("bool")) {
        // For symmetry with kreadconfig we accept a wider range of values as true than Qt
        bool boolvalue=(value==QStringLiteral("true") || value==QStringLiteral("on") || value==QStringLiteral("yes") || value==QStringLiteral("1"));
        cfgGroup.writeEntry( key, boolvalue );
    } else if (type==QStringLiteral("path")) {
        cfgGroup.writePathEntry( key, value );
    } else {
        cfgGroup.writeEntry( key, value );
    }
    konfig->sync();
    delete konfig;
    return 0;
}

