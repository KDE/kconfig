/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only

*/

#include <QCoreApplication>
#include <QLocale>
#include <QThreadStorage>
#include <QTimer>
#include <QtGlobal>

using ParseCache = QThreadStorage<int>;
Q_GLOBAL_STATIC(ParseCache, sGlobalParse)

class Tester
{
public:
    ~Tester()
    {
        QLocale().name();
        sGlobalParse->localData();
    }
};

Q_GLOBAL_STATIC(Tester, globalTestObject)

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    Tester *t = globalTestObject();

    QTimer::singleShot(0, qApp, SLOT(quit()));
    return app.exec();
}
