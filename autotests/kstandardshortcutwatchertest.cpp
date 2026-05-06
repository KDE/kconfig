/*
    SPDX-FileCopyrightText: 2022 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kstandardshortcutwatcher.h"
#include "kconfiggroup.h"
#include "ksharedconfig.h"
#include "kstandardshortcut_p.h"

#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

class KStandardShortcutWatcherTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void init();
    void testSignal();
    void testDataUpdated();
    void testRemovedShortcut();
};

Q_DECLARE_METATYPE(KStandardShortcut::StandardShortcut)

const QList<QKeySequence> newShortcut = {Qt::CTRL | Qt::Key_Adiaeresis};

void KStandardShortcutWatcherTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    qRegisterMetaType<KStandardShortcut::StandardShortcut>();
    QVERIFY(KStandardShortcut::hardcodedDefaultShortcut(KStandardShortcut::Open) != newShortcut);
}

void KStandardShortcutWatcherTest::init()
{
    KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("Shortcuts"));
    group.writeEntry("Open", QKeySequence::listToString(KStandardShortcut::hardcodedDefaultShortcut(KStandardShortcut::Open)), KConfig::Global);
    group.sync();
    KStandardShortcut::initialize(KStandardShortcut::Open);
}

void KStandardShortcutWatcherTest::testSignal()
{
#ifdef Q_OS_WIN
    QSKIP("KConfig is built without DBus on Windows");
#endif
    QSignalSpy signalSpy(KStandardShortcut::shortcutWatcher(), &KStandardShortcut::StandardShortcutWatcher::shortcutChanged);
    KStandardShortcut::saveShortcut(KStandardShortcut::Open, newShortcut);
    QTRY_COMPARE(signalSpy.count(), 1);
    const QList<QVariant> arguments = signalSpy.takeFirst();
    QCOMPARE(arguments[0].toInt(), KStandardShortcut::Open);
    QCOMPARE(arguments[1].value<QList<QKeySequence>>(), newShortcut);
}

void KStandardShortcutWatcherTest::testDataUpdated()
{
#ifdef Q_OS_WIN
    QSKIP("KConfig is built without DBus on Windows");
#endif
    QSignalSpy signalSpy(KStandardShortcut::shortcutWatcher(), &KStandardShortcut::StandardShortcutWatcher::shortcutChanged);
    // Writing manually to forego automatic update in saveShortcut()
    KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("Shortcuts"));
    group.writeEntry("Open", QKeySequence::listToString(newShortcut), KConfig::Global | KConfig::Notify);
    group.sync();
    QTRY_COMPARE(signalSpy.count(), 1);
    QCOMPARE(KStandardShortcut::open(), newShortcut);
}

void KStandardShortcutWatcherTest::testRemovedShortcut()
{
#ifdef Q_OS_WIN
    QSKIP("KConfig is built without DBus on Windows");
#endif
    static const auto shandardShortcut = KStandardShortcut::Help;
    using QKeySequenceList = QList<QKeySequence>;
    QSignalSpy signalSpy(KStandardShortcut::shortcutWatcher(), //
                         &KStandardShortcut::StandardShortcutWatcher::shortcutChanged);
    auto lastChanged = signalSpy.crend();
    static const QKeySequenceList emptyShortcut{};
    auto changeAndVerify = [&]() {
        // Later, we need to assume that the old shortcut is not empty.
        const auto oldShortcut = KStandardShortcut::shortcut(shandardShortcut);
        QVERIFY(oldShortcut != emptyShortcut);

        KStandardShortcut::saveShortcut(shandardShortcut, emptyShortcut);
        // QTRY_* macros wait 5 seconds before checking. We're doing the wait
        // separately so that we don't need to get and verify in one line of code.
        QVERIFY(signalSpy.wait(std::chrono::seconds{5}));
        auto it = std::find_if(signalSpy.crbegin(), signalSpy.crend(), [](const QVariantList &list) {
            return list[0].value<KStandardShortcut::StandardShortcut>() == shandardShortcut;
        });
        // Verify that a new change happened.
        QVERIFY(it != signalSpy.crend());
        QVERIFY(it != lastChanged);
        lastChanged = it;
        // Before StandardShortcutWatcher::shortcutChanged is emitted,
        // StandardShortcutWatcher calls KStandardShortcut::initialize.
        // Verify that the signal sent the correct shortcut.
        QCOMPARE((*it)[1].value<QKeySequenceList>(), emptyShortcut);
        // Verify that KStandardShortcut::shortcut returns the same thing.
        QCOMPARE(KStandardShortcut::shortcut(shandardShortcut), emptyShortcut);
    };

    // Later, we need to assume that the default shortcut is being used.
    QCOMPARE(KStandardShortcut::shortcut(shandardShortcut), KStandardShortcut::hardcodedDefaultShortcut(shandardShortcut));
    // Verify change to empty with default being used first.
    changeAndVerify();

    // Verify change to empty with non-default being used first.
    const QKeySequenceList nonDefaultShortcut{QKeyCombination{Qt::KeyboardModifierMask, Qt::Key_Idiaeresis}};
    KStandardShortcut::saveShortcut(shandardShortcut, nonDefaultShortcut);
    QVERIFY(signalSpy.wait(std::chrono::seconds{5}));
    changeAndVerify();
}

QTEST_MAIN(KStandardShortcutWatcherTest)
#include "kstandardshortcutwatchertest.moc"
