/*
   This file is part of the KDE libraries
   Copyright (c) 2015 Christoph Cullmann <cullmann@kde.org>

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

#include <QtTest>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

class FallbackConfigResourcesTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testResourceFallbackFile();
};

void FallbackConfigResourcesTest::initTestCase()
{
    QStandardPaths::enableTestMode(true);
}

void FallbackConfigResourcesTest::testResourceFallbackFile()
{
    KSharedConfig::Ptr sharedConfig = KSharedConfig::openConfig(QStringLiteral("kconfigtesting"), KConfig::NoGlobals);
    QVERIFY(sharedConfig);

    KConfigGroup cfg(sharedConfig, QStringLiteral("MainSection"));
    QCOMPARE(cfg.readEntry(QStringLiteral("TestEntry"), QStringLiteral("UnexpectedData")), QStringLiteral("ExpectedData"));
}

QTEST_MAIN(FallbackConfigResourcesTest)

#include "fallbackconfigresourcestest.moc"
