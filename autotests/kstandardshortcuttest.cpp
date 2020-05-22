/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QtTest>
#include "kstandardshortcuttest.h"

QTEST_MAIN(KStandardShortcutTest)  // GUI needed by KAccel

#include <kstandardshortcut.h>

void KStandardShortcutTest::testShortcutDefault()
{
    QCOMPARE(QKeySequence::listToString(KStandardShortcut::hardcodedDefaultShortcut(KStandardShortcut::FullScreen)), QLatin1String("Ctrl+Shift+F"));
    QCOMPARE(QKeySequence::listToString(KStandardShortcut::hardcodedDefaultShortcut(KStandardShortcut::BeginningOfLine)), QLatin1String("Home"));
    QCOMPARE(QKeySequence::listToString(KStandardShortcut::hardcodedDefaultShortcut(KStandardShortcut::EndOfLine)), QLatin1String("End"));
    QCOMPARE(QKeySequence::listToString(KStandardShortcut::hardcodedDefaultShortcut(KStandardShortcut::Home)), QLatin1String("Alt+Home; Home Page"));
}

void KStandardShortcutTest::testName()
{
    QCOMPARE(KStandardShortcut::name(KStandardShortcut::BeginningOfLine), QLatin1String("BeginningOfLine"));
    QCOMPARE(KStandardShortcut::name(KStandardShortcut::EndOfLine), QLatin1String("EndOfLine"));
    QCOMPARE(KStandardShortcut::name(KStandardShortcut::Home), QLatin1String("Home"));
}

void KStandardShortcutTest::testLabel()
{
    // Tests run in English, right?
    QCOMPARE(KStandardShortcut::label(KStandardShortcut::FindNext), QLatin1String("Find Next"));
    QCOMPARE(KStandardShortcut::label(KStandardShortcut::Home), QLatin1String("Home"));
}

void KStandardShortcutTest::testShortcut()
{
    QCOMPARE(QKeySequence::listToString(KStandardShortcut::shortcut(KStandardShortcut::ZoomIn)), QKeySequence::listToString(KStandardShortcut::zoomIn()));
}

void KStandardShortcutTest::testFindStdAccel()
{
    QCOMPARE(KStandardShortcut::find(QString("Ctrl+F")), KStandardShortcut::Find);
    QCOMPARE(KStandardShortcut::find(QString("Ctrl+Shift+Alt+G")), KStandardShortcut::AccelNone);
}

void KStandardShortcutTest::testFindByName()
{
    for (int i = KStandardShortcut::AccelNone + 1; i < KStandardShortcut::StandardShortcutCount; ++i) {
        const auto id = static_cast<KStandardShortcut::StandardShortcut>(i);
        QCOMPARE(id, KStandardShortcut::findByName(KStandardShortcut::name(id)));
    }
}
