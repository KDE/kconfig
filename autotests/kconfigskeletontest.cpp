/* This file is part of the KDE libraries
    Copyright (C) 2006 Olivier Goffart  <ogoffart at kde.org>

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

#include "kconfigskeletontest.h"

#include <kconfig.h>
#include <QFont>
#include <QtTest/QtTestGui>

QTEST_MAIN(KConfigSkeletonTest)

#define DEFAULT_SETTING1 false
#define DEFAULT_SETTING2 QColor(1,2,3)
#define DEFAULT_SETTING3 QFont("helvetica",12)
#define DEFAULT_SETTING4 QString("Hello World")

#define WRITE_SETTING1 true
#define WRITE_SETTING2 QColor(3,2,1)
#define WRITE_SETTING3 QFont("helvetica",14)
#define WRITE_SETTING4 QString("KDE")

void KConfigSkeletonTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void KConfigSkeletonTest::init()
{
    QFile::remove(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/kconfigskeletontestrc");
    s = new KConfigSkeleton("kconfigskeletontestrc");
    s->setCurrentGroup("MyGroup");
    itemBool = s->addItemBool("MySetting1", mMyBool, DEFAULT_SETTING1);
    s->addItemColor("MySetting2", mMyColor, DEFAULT_SETTING2);

    s->setCurrentGroup("MyOtherGroup");
    s->addItemFont("MySetting3", mMyFont, DEFAULT_SETTING3);
    s->addItemString("MySetting4", mMyString, DEFAULT_SETTING4);

    QCOMPARE(mMyBool, DEFAULT_SETTING1);
    QCOMPARE(mMyColor, DEFAULT_SETTING2);
    QCOMPARE(mMyFont, DEFAULT_SETTING3);
    QCOMPARE(mMyString, DEFAULT_SETTING4);
}

void KConfigSkeletonTest::cleanup()
{
    delete s;
}

void KConfigSkeletonTest::testSimple()
{
    mMyBool = WRITE_SETTING1;
    mMyColor = WRITE_SETTING2;
    mMyFont = WRITE_SETTING3;
    mMyString = WRITE_SETTING4;

    s->save();

    mMyBool = false;
    mMyColor = QColor();
    mMyString.clear();
    mMyFont = QFont();

    s->read();

    QCOMPARE(mMyBool, WRITE_SETTING1);
    QCOMPARE(mMyColor, WRITE_SETTING2);
    QCOMPARE(mMyFont, WRITE_SETTING3);
    QCOMPARE(mMyString, WRITE_SETTING4);
}

void KConfigSkeletonTest::testRemoveItem()
{
    QVERIFY(s->findItem("MySetting1"));
    s->removeItem("MySetting1");
    QVERIFY(!s->findItem("MySetting1"));
}

void KConfigSkeletonTest::testClear()
{
    QVERIFY(s->findItem("MySetting2"));
    QVERIFY(s->findItem("MySetting3"));
    QVERIFY(s->findItem("MySetting4"));

    s->clearItems();

    QVERIFY(!s->findItem("MySetting2"));
    QVERIFY(!s->findItem("MySetting3"));
    QVERIFY(!s->findItem("MySetting4"));
}

void KConfigSkeletonTest::testDefaults()
{
    mMyBool = WRITE_SETTING1;
    mMyColor = WRITE_SETTING2;
    mMyFont = WRITE_SETTING3;
    mMyString = WRITE_SETTING4;

    s->save();

    s->setDefaults();

    QCOMPARE(mMyBool, DEFAULT_SETTING1);
    QCOMPARE(mMyColor, DEFAULT_SETTING2);
    QCOMPARE(mMyFont, DEFAULT_SETTING3);
    QCOMPARE(mMyString, DEFAULT_SETTING4);

    s->save();
}

void KConfigSkeletonTest::testKConfigDirty()
{
    itemBool->setValue(true);
    itemBool->writeConfig(s->sharedConfig().data());
    QVERIFY(s->sharedConfig()->isDirty());
    s->save();
    QVERIFY(!s->sharedConfig()->isDirty());

    itemBool->setValue(false);
    itemBool->writeConfig(s->sharedConfig().data());
    QVERIFY(s->sharedConfig()->isDirty());
    s->save();
    QVERIFY(!s->sharedConfig()->isDirty());
}

void KConfigSkeletonTest::testSaveRead()
{
    itemBool->setValue(true);
    s->save();

    itemBool->setValue(false);
    s->save();

    mMyBool = true;

    s->read();
    QCOMPARE(mMyBool, false);
}
