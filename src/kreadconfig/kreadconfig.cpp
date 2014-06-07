/*  Read KConfig() entries - for use in shell scripts.

    Copyright (c) 2001 Red Hat, Inc.

    Programmed by Bernhard Rosenkraenzer <bero@redhat.com>

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

/*
 * If --type is specified as bool, the return value is 0 if the value
 * is set, 1 if it isn't set. There is no output.
 *
 * If --type is specified as num, the return value matches the value
 * of the key. There is no output.
 *
 * If --type is not set, the value of the key is simply printed to stdout.
 *
 * Usage examples:
 *	if kreadconfig5 --group KDE --key macStyle --type bool; then
 *		echo "We're using Mac-Style menus."
 *	else
 *		echo "We're using normal menus."
 *	fi
 *
 *	TRASH=`kreadconfig5 --group Paths --key Trash`
 *	if test -n "$TRASH"; then
 *		mv someFile "$TRASH"
 *	else
 *		rm someFile
 *	fi
 */

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QCommandLineParser>
#include <stdio.h>

int main(int argc, char **argv)
{
	QCoreApplication app(argc, argv);

	QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QStringLiteral("file"), QCoreApplication::translate("main", "Use <file> instead of global config"), QStringLiteral("file")));
    parser.addOption(QCommandLineOption(QStringLiteral("group"), QCoreApplication::translate("main", "Group to look in. Use repeatedly for nested groups."), QStringLiteral("group"), QStringLiteral("KDE")));
    parser.addOption(QCommandLineOption(QStringLiteral("key"), QCoreApplication::translate("main", "Key to look for"), QStringLiteral("key")));
    parser.addOption(QCommandLineOption(QStringLiteral("default"), QCoreApplication::translate("main", "Default value"), QStringLiteral("value")));
    parser.addOption(QCommandLineOption(QStringLiteral("type"), QCoreApplication::translate("main", "Type of variable"), QStringLiteral("type")));

	parser.process(app);

    QStringList groups=parser.values(QStringLiteral("group"));
    QString key=parser.value(QStringLiteral("key"));
    QString file=parser.value(QStringLiteral("file"));
    QString dflt=parser.value(QStringLiteral("default"));
    QString type=parser.value(QStringLiteral("type")).toLower();

    if (key.isNull() || !parser.positionalArguments().isEmpty()) {
		parser.showHelp(1);
	}

	KSharedConfig::openConfig();

	KConfig *konfig;
	bool configMustDeleted = false;
	if (file.isEmpty())
	   konfig = KSharedConfig::openConfig().data();
	else
	{
		konfig = new KConfig( file, KConfig::NoGlobals );
		configMustDeleted=true;
	}
	KConfigGroup cfgGroup = konfig->group(QString());
	foreach (const QString &grp, groups)
		cfgGroup = cfgGroup.group(grp);
    if(type==QStringLiteral("bool")) {
		dflt=dflt.toLower();
        bool def=(dflt==QStringLiteral("true") || dflt==QStringLiteral("on") || dflt==QStringLiteral("yes") || dflt==QStringLiteral("1"));
		bool retValue = !cfgGroup.readEntry(key, def);
		if ( configMustDeleted )
			delete konfig;
		return retValue;
    } else if((type==QStringLiteral("num")) || (type==QStringLiteral("int"))) {
		int retValue = cfgGroup.readEntry(key, dflt.toInt());
		if ( configMustDeleted )
			delete konfig;
		return retValue;
    } else if (type==QStringLiteral("path")){
		fprintf(stdout, "%s\n", cfgGroup.readPathEntry(key, dflt).toLocal8Bit().data());
		if ( configMustDeleted )
			delete konfig;
		return 0;
	} else {
		/* Assume it's a string... */
		fprintf(stdout, "%s\n", cfgGroup.readEntry(key, dflt).toLocal8Bit().data());
		if ( configMustDeleted )
			delete konfig;
		return 0;
	}
}

