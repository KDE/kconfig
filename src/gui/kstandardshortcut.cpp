/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Stefan Taferner <taferner@alpin.or.at>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2001, 2002 Ellis Whitehead <ellis@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kstandardshortcut.h"
#include "kstandardshortcutwatcher.h"

#include "kconfig.h"
#include "kconfigwatcher.h"
#include "ksharedconfig.h"
#include <kconfiggroup.h>

#include <QCoreApplication>
#include <QDebug>
#include <QKeySequence>

namespace KStandardShortcut
{
struct KStandardShortcutInfo {
    // The standard shortcut id. @see StandardShortcut
    StandardShortcut id;

    /*
     * Unique name for the given accel. The name is used to save the user
     * settings. It's not representable. Use description for that.
     * @warning NEVER EVER CHANGE IT OR TRANSLATE IT!
     */
    const char *name;

    //! Localized label for user-visible display, including translation context.
    struct {
        const char *text;
        const char *context;
    } description;

    QList<QKeySequence> defaultShortcuts;

    // A shortcut that is created with @a defaultShortcuts
    QList<QKeySequence> actionShortcuts;

    // If this struct is initialized. If not initialized @cut is not valid
    bool isInitialized;

    // Category of this Shortcut
    Category category;
};

#define CTRL(x) QKeySequence(QKeyCombination(Qt::CTRL | Qt::Key_##x))
#define SHIFT(x) QKeySequence(QKeyCombination(Qt::SHIFT | Qt::Key_##x))
#define CTRLALT(x) QKeySequence(QKeyCombination(Qt::CTRL | Qt::ALT | Qt::Key_##x))
#define CTRLSHIFT(x) QKeySequence(QKeyCombination(Qt::CTRL | Qt::SHIFT | Qt::Key_##x))
#define ALT(x) QKeySequence(QKeyCombination(Qt::ALT | Qt::Key_##x))
#define ALTSHIFT(x) QKeySequence(QKeyCombination(Qt::ALT | Qt::SHIFT | Qt::Key_##x))
#define CTRLMETA(x) QKeySequence(QKeyCombination(Qt::CTRL | Qt::META | Qt::Key_##x))

/* Array of predefined KStandardShortcutInfo objects, which cover all
    the "standard" accelerators. Each enum value from StandardShortcut
    should appear in this table.
*/
// STUFF WILL BREAK IF YOU DON'T READ THIS!!!
// Read the comments of the big enum in kstandardshortcut.h before you change anything!
// clazy:exclude-next-line=non-pod-global-static
static KStandardShortcutInfo g_infoStandardShortcut[] = {
    // Group File,
    {AccelNone, nullptr, {nullptr, nullptr}, QList<QKeySequence>{}, QList<QKeySequence>(), false, Category::InvalidCategory},
    {Open,
     "Open",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Open", "@action"),
     QList<QKeySequence>{QList{CTRL(N)}},
     QList<QKeySequence>(),
     false,
     Category::File},
    {New, "New", QT_TRANSLATE_NOOP3("KStandardShortcut", "New", "@action"), QList<QKeySequence>{CTRL(N) }, QList<QKeySequence>(), false, Category::File},
    {Close,
     "Close",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Close", "@action"),
     QList<QKeySequence>{QList{CTRL(W), CTRL(Escape)}},
     QList<QKeySequence>(),
     false,
     Category::File},

    {Save, "Save", QT_TRANSLATE_NOOP3("KStandardShortcut", "Save", "@action"), QList<QKeySequence>{CTRL(S)}, QList<QKeySequence>(), false, Category::File},
    {Print, "Print", QT_TRANSLATE_NOOP3("KStandardShortcut", "Print", "@action"), QList<QKeySequence>{CTRL(P)}, QList<QKeySequence>(), false, Category::File},
    {Quit,
     "Quit",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Quit", "@action"),
     QList<QKeySequence>{CTRL(Q)},
     QList<QKeySequence>(),
     false,
     Category::Navigation},

    // Group Edit
    {Undo, "Undo", QT_TRANSLATE_NOOP3("KStandardShortcut", "Undo", "@action"), QList<QKeySequence>{CTRL(Z)}, QList<QKeySequence>(), false, Category::Edit},
    {Redo, "Redo", QT_TRANSLATE_NOOP3("KStandardShortcut", "Redo", "@action"), QList<QKeySequence>{CTRLSHIFT(Z)}, QList<QKeySequence>(), false, Category::Edit},
    // Both "Cut" and "Delete" use Shift+Delete, but this is okay; see
    // https://commits.kde.org/kxmlgui/8eabbf6725386e716b7536c71a9181dfe5d959f0
    {Cut,
     "Cut",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Cut", "@action"),
     QList<QKeySequence>{CTRL(X), SHIFT(Delete)},
     QList<QKeySequence>(),
     false,
     Category::Edit},
    {Copy,
     "Copy",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Copy", "@action"),
     QList<QKeySequence>{CTRL(C), CTRL(Insert)},
     QList<QKeySequence>(),
     false,
     Category::Edit},
    {Paste,
     "Paste",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Paste", "@action"),
     QList<QKeySequence>{CTRL(V), SHIFT(Insert)},
     QList<QKeySequence>(),
     false,
     Category::Edit},
    {PasteSelection,
     "Paste Selection",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Paste Selection", "@action"),
     QList<QKeySequence>{CTRLSHIFT(Insert)},
     QList<QKeySequence>(),
     false,
     Category::Edit},

    {SelectAll,
     "SelectAll",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Select All", "@action"),
     QList<QKeySequence>{CTRL(A)},
     QList<QKeySequence>(),
     false,
     Category::Edit},
    {Deselect,
     "Deselect",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Deselect", "@action"),
     QList<QKeySequence>{CTRLSHIFT(A)},
     QList<QKeySequence>(),
     false,
     Category::Edit},
    {DeleteWordBack,
     "DeleteWordBack",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Delete Word Backwards", "@action"),
     QList<QKeySequence>{
#if defined(Q_OS_MACOS)
         ALT(Backspace)},
#else
         CTRL(Backspace)},
#endif
     QList<QKeySequence>(),
     false,
     Category::Edit},
    {DeleteWordForward,
     "DeleteWordForward",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Delete Word Forward", "@action"),
     QList<QKeySequence>{
#if defined(Q_OS_MACOS)
         ALT(Delete)},
#else
         CTRL(Delete)},
#endif
     QList<QKeySequence>(),
     false,
     Category::Edit},

    {Find, "Find", QT_TRANSLATE_NOOP3("KStandardShortcut", "Find", "@action"), QList<QKeySequence>{CTRL(F)}, QList<QKeySequence>(), false, Category::Edit},
    {FindNext,
     "FindNext",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Find Next", "@action"),
     QList<QKeySequence>{
#if defined(Q_OS_MACOS)
         CTRL(G)},
#else
         Qt::Key_F3},
#endif
     QList<QKeySequence>(),
     false,
     Category::Edit},
    {FindPrev,
     "FindPrev",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Find Prev", "@action"),
     QList<QKeySequence>{SHIFT(F3)},
     QList<QKeySequence>(),
     false,
     Category::Edit},
    {Replace,
     "Replace",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Replace", "@action"),
     QList<QKeySequence>{CTRL(R)},
     QList<QKeySequence>(),
     false,
     Category::Edit},

    // Group Navigation
    {Home,
     "Home",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Home", "@action Go to main page"),
     QList<QKeySequence>{ALT(Home), Qt::Key_HomePage},
     QList<QKeySequence>(),
     false,
     Category::Navigation},
    {Begin,
     "Begin",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Begin", "@action Beginning of document"),
     QList<QKeySequence>{
#if defined(Q_OS_MACOS)
         CTRL(Up)},
#else
         CTRL(Home)},
#endif
     QList<QKeySequence>(),
     false,
     Category::Navigation},
    {End,
     "End",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "End", "@action End of document"),
     QList<QKeySequence>{
#if defined(Q_OS_MACOS)
         CTRL(Down)},
#else
         CTRL(End)},
#endif
     QList<QKeySequence>(),
     false,
     Category::Navigation},
    {Prior,
     "Prior",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Prior", "@action"),
     QList<QKeySequence>{Qt::Key_PageUp},
     QList<QKeySequence>(),
     false,
     Category::Navigation},
    {Next,
     "Next",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Next", "@action Opposite to Prior"),
     QList<QKeySequence>{Qt::Key_PageDown},
     QList<QKeySequence>(),
     false,
     Category::Navigation},

    {Up, "Up", QT_TRANSLATE_NOOP3("KStandardShortcut", "Up", "@action"), QList<QKeySequence>{ALT(Up)}, QList<QKeySequence>(), false, Category::Navigation},
    {Back,
     "Back",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Back", "@action"),
     QList<QKeySequence>{ALT(Left), Qt::Key_Back},
     QList<QKeySequence>(),
     false,
     Category::Navigation},
    {Forward,
     "Forward",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Forward", "@action"),
     QList<QKeySequence>{ALT(Right), Qt::Key_Forward},
     QList<QKeySequence>(),
     false,
     Category::Navigation},
    {Reload,
     "Reload",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Reload", "@action"),
     QList<QKeySequence>{Qt::Key_F5, Qt::Key_Refresh},
     QList<QKeySequence>(),
     false,
     Category::Navigation},

    {BeginningOfLine,
     "BeginningOfLine",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Beginning of Line", "@action"),
     QList<QKeySequence>{
#if defined(Q_OS_MACOS)
         CTRL(Left)},
#else
         Qt::Key_Home},
#endif
     QList<QKeySequence>(),
     false,
     Category::Navigation},
    {EndOfLine,
     "EndOfLine",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "End of Line", "@action"),
     QList<QKeySequence>{
#if defined(Q_OS_MACOS)
         CTRL(Right)},
#else
         Qt::Key_End},
#endif
     QList<QKeySequence>(),
     false,
     Category::Navigation},
    {GotoLine,
     "GotoLine",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Go to Line", "@action"),
     QList<QKeySequence>{CTRL(G)},
     QList<QKeySequence>(),
     false,
     Category::Navigation},
    {BackwardWord,
     "BackwardWord",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Backward Word", "@action"),
     QList<QKeySequence>{
#if defined(Q_OS_MACOS)
         ALT(Left)},
#else
         CTRL(Left)},
#endif
     QList<QKeySequence>(),
     false,
     Category::Navigation},
    {ForwardWord,
     "ForwardWord",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Forward Word", "@action"),
     QList<QKeySequence>{
#if defined(Q_OS_MACOS)
         ALT(Right)},
#else
         CTRL(Right)},
#endif
     QList<QKeySequence>(),
     false,
     Category::Navigation},

    {AddBookmark,
     "AddBookmark",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Add Bookmark", "@action"),
     QList<QKeySequence>{CTRL(B)},
     QList<QKeySequence>(),
     false,
     Category::Navigation},
    {ZoomIn,
     "ZoomIn",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Zoom In", "@action"),
     QList<QKeySequence>{CTRL(Plus), CTRL(Equal)},
     QList<QKeySequence>(),
     false,
     Category::View},
    {ZoomOut,
     "ZoomOut",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Zoom Out", "@action"),
     QList<QKeySequence>{CTRL(Minus)},
     QList<QKeySequence>(),
     false,
     Category::View},
    {FullScreen,
     "FullScreen",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Full Screen Mode", "@action"),
     QList<QKeySequence>{
#if defined(Q_OS_MACOS)
         CTRLMETA(F)},
#else
         CTRLSHIFT(F)},
#endif
     QList<QKeySequence>(),
     false,
     Category::View},

    {ShowMenubar,
     "ShowMenubar",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Show Menu Bar", "@action"),
     QList<QKeySequence>{CTRL(M)},
     QList<QKeySequence>(),
     false,
     Category::View},
    {TabNext,
     "Activate Next Tab",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Activate Next Tab", "@action"),
     QList<QKeySequence>{CTRL(PageDown), CTRL(BracketRight)},
     QList<QKeySequence>(),
     false,
     Category::Navigation},
    {TabPrev,
     "Activate Previous Tab",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Activate Previous Tab", "@action"),
     QList<QKeySequence>{CTRL(PageUp), CTRL(BracketLeft)},
     QList<QKeySequence>(),
     false,
     Category::Navigation},

    // Group Help
    {Help, "Help", QT_TRANSLATE_NOOP3("KStandardShortcut", "Help", "@action"), QList<QKeySequence>{Qt::Key_F1}, QList<QKeySequence>(), false, Category::Help},
    {WhatsThis,
     "WhatsThis",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "What's This", "@action"),
     QList<QKeySequence>{SHIFT(F1)},
     QList<QKeySequence>(),
     false,
     Category::Help},

    // Group TextCompletion
    {TextCompletion,
     "TextCompletion",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Text Completion", "@action"),
     QList<QKeySequence>{CTRL(E)},
     QList<QKeySequence>(),
     false,
     Category::Edit},
    {PrevCompletion,
     "PrevCompletion",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Previous Completion Match", "@action"),
     QList<QKeySequence>{CTRL(Up)},
     QList<QKeySequence>(),
     false,
     Category::Edit},
    {NextCompletion,
     "NextCompletion",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Next Completion Match", "@action"),
     QList<QKeySequence>{CTRL(Down)},
     QList<QKeySequence>(),
     false,
     Category::Edit},
    {SubstringCompletion,
     "SubstringCompletion",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Substring Completion", "@action"),
     QList<QKeySequence>{CTRL(T)},
     QList<QKeySequence>(),
     false,
     Category::Edit},

    {RotateUp,
     "RotateUp",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Previous Item in List", "@action"),
     QList<QKeySequence>{Qt::Key_Up},
     QList<QKeySequence>(),
     false,
     Category::Navigation},
    {RotateDown,
     "RotateDown",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Next Item in List", "@action"),
     QList<QKeySequence>{Qt::Key_Down},
     QList<QKeySequence>(),
     false,
     Category::Navigation},

    {OpenRecent,
     "OpenRecent",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Open Recent", "@action"),
     QList<QKeySequence>{},
     QList<QKeySequence>(),
     false,
     Category::File},
    {SaveAs,
     "SaveAs",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Save As", "@action"),
     QList<QKeySequence>{CTRLSHIFT(S)},
     QList<QKeySequence>(),
     false,
     Category::File},
    {Revert, "Revert", QT_TRANSLATE_NOOP3("KStandardShortcut", "Revert", "@action"), QList<QKeySequence>{}, QList<QKeySequence>(), false, Category::Edit},
    {PrintPreview,
     "PrintPreview",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Print Preview", "@action"),
     QList<QKeySequence>{},
     QList<QKeySequence>(),
     false,
     Category::File},
    {Mail, "Mail", QT_TRANSLATE_NOOP3("KStandardShortcut", "Mail", "@action"), QList<QKeySequence>{}, QList<QKeySequence>(), false, Category::Help},
    {Clear, "Clear", QT_TRANSLATE_NOOP3("KStandardShortcut", "Clear", "@action"), QList<QKeySequence>{}, QList<QKeySequence>(), false, Category::Edit},
    {ActualSize,
     "ActualSize",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Zoom to Actual Size", "@action"),
     QList<QKeySequence>{CTRL(0)},
     QList<QKeySequence>(),
     false,
     Category::View},
    {FitToPage,
     "FitToPage",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Fit To Page", "@action"),
     QList<QKeySequence>{},
     QList<QKeySequence>(),
     false,
     Category::View},
    {FitToWidth,
     "FitToWidth",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Fit To Width", "@action"),
     QList<QKeySequence>{},
     QList<QKeySequence>(),
     false,
     Category::View},
    {FitToHeight,
     "FitToHeight",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Fit To Height", "@action"),
     QList<QKeySequence>{},
     QList<QKeySequence>(),
     false,
     Category::View},
    {Zoom, "Zoom", QT_TRANSLATE_NOOP3("KStandardShortcut", "Zoom", "@action"), QList<QKeySequence>{}, QList<QKeySequence>(), false, Category::View},
    {Goto, "Goto", QT_TRANSLATE_NOOP3("KStandardShortcut", "Goto", "@action"), QList<QKeySequence>{}, QList<QKeySequence>(), false, Category::Navigation},
    {GotoPage,
     "GotoPage",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Goto Page", "@action"),
     QList<QKeySequence>{},
     QList<QKeySequence>(),
     false,
     Category::Navigation},
    {DocumentBack,
     "DocumentBack",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Document Back", "@action"),
     QList<QKeySequence>{ALTSHIFT(Left)},
     QList<QKeySequence>(),
     false,
     Category::Navigation},
    {DocumentForward,
     "DocumentForward",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Document Forward", "@action"),
     QList<QKeySequence>{ALTSHIFT(Right)},
     QList<QKeySequence>(),
     false,
     Category::Navigation},
    {EditBookmarks,
     "EditBookmarks",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Edit Bookmarks", "@action"),
     QList<QKeySequence>(),
     QList<QKeySequence>(),
     false,
     Category::Navigation},
    {Spelling, "Spelling", QT_TRANSLATE_NOOP3("KStandardShortcut", "Spelling", "@action"), QList<QKeySequence>{}, QList<QKeySequence>(), false, Category::Edit},
    {ShowToolbar,
     "ShowToolbar",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Show Toolbar", "@action"),
     QList<QKeySequence>{},
     QList<QKeySequence>(),
     false,
     Category::View},
    {ShowStatusbar,
     "ShowStatusbar",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Show Statusbar", "@action"),
     QList<QKeySequence>{},
     QList<QKeySequence>(),
     false,
     Category::View},
    {KeyBindings,
     "KeyBindings",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Key Bindings", "@action"),
     QList<QKeySequence>{CTRLALT(Comma)},
     QList<QKeySequence>(),
     false,
     Category::Settings},
    {Preferences,
     "Preferences",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Configure Application", "@action"),
     QList<QKeySequence>{CTRLSHIFT(Comma)},
     QList<QKeySequence>(),
     false,
     Category::Settings},
    {ConfigureToolbars,
     "ConfigureToolbars",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Configure Toolbars", "@action"),
     QList<QKeySequence>(),
     QList<QKeySequence>(),
     false,
     Category::Settings},
    {ConfigureNotifications,
     "ConfigureNotifications",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Configure Notifications", "@action"),
     QList<QKeySequence>(),
     QList<QKeySequence>(),
     false,
     Category::Settings},
    {ReportBug,
     "ReportBug",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Report Bug", "@action"),
     QList<QKeySequence>{},
     QList<QKeySequence>(),
     false,
     Category::Help},
    {SwitchApplicationLanguage,
     "SwitchApplicationLanguage",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Configure Language…", "@action"),
     QList<QKeySequence>(),
     QList<QKeySequence>(),
     false,
     Category::Settings},
    {AboutApp,
     "AboutApp",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "About Application", "@action"),
     QList<QKeySequence>{},
     QList<QKeySequence>(),
     false,
     Category::Help},
    {AboutKDE,
     "AboutKDE",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "About KDE", "@action"),
     QList<QKeySequence>{},
     QList<QKeySequence>(),
     false,
     Category::Help},
    // Both "Cut" and "Delete" use Shift+Delete, but this is okay; see
    // https://commits.kde.org/kxmlgui/8eabbf6725386e716b7536c71a9181dfe5d959f0
    {DeleteFile,
     "DeleteFile",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Delete", "@action"),
     QList<QKeySequence>{SHIFT(Delete)},
     QList<QKeySequence>(),
     false,
     Category::File},
    {RenameFile,
     "RenameFile",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Rename", "@action"),
     QList<QKeySequence>{Qt::Key_F2},
     QList<QKeySequence>(),
     false,
     Category::File},
    {MoveToTrash,
     "MoveToTrash",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Move to Trash", "@action"),
     QList<QKeySequence>{Qt::Key_Delete},
     QList<QKeySequence>(),
     false,
     Category::File},
    {Donate, "Donate", QT_TRANSLATE_NOOP3("KStandardShortcut", "Donate", "@action"), QList<QKeySequence>{}, QList<QKeySequence>(), false, Category::Help},
    {ShowHideHiddenFiles,
     "ShowHideHiddenFiles",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Show/Hide Hidden Files", "@action"),
     QList<QKeySequence>{CTRL(H), ALT(Period)},
     QList<QKeySequence>(),
     false,
     Category::View},
    {CreateFolder,
     "CreateFolder",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Create Folder", "@action"),
     QList<QKeySequence>{CTRLSHIFT(N)},
     QList<QKeySequence>(),
     false,
     Category::File},
    {OpenMainMenu,
     "OpenMainMenu",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Open Main Menu", "@action referring to the menu bar or a hamburger menu"),
     QList<QKeySequence>{Qt::Key_F10},
     QList<QKeySequence>(),
     false,
     Category::View},
    {OpenContextMenu,
     "OpenContextMenu",
     QT_TRANSLATE_NOOP3("KStandardShortcut", "Open Context Menu", "@action"),
     QList<QKeySequence>{Qt::Key_Menu, SHIFT(F10)},
     QList<QKeySequence>(),
     false,
     Category::View},

    // dummy entry to catch simple off-by-one errors. Insert new entriesS before this line.
    {AccelNone, nullptr, {nullptr, nullptr}, QList<QKeySequence>(), QList<QKeySequence>(), false, Category::InvalidCategory}};

/* Search for the KStandardShortcutInfo object associated with the given @p id.
    Return a dummy entry with no name and an empty shortcut if @p id is invalid.
*/
static KStandardShortcutInfo *guardedStandardShortcutInfo(StandardShortcut id)
{
    if (id >= static_cast<int>(sizeof(g_infoStandardShortcut) / sizeof(KStandardShortcutInfo)) || id < 0) {
        qWarning() << "KStandardShortcut: id not found!";
        return &g_infoStandardShortcut[AccelNone];
    } else {
        return &g_infoStandardShortcut[id];
    }
}

// Sanitize the list for duplicates. For some reason some
// people have kdeglobals entries like
//   Close=Ctrl+W; Ctrl+Esc; Ctrl+W; Ctrl+Esc;
// having the same shortcut more than once in the shortcut
// declaration is clearly bogus so fix it
static void sanitizeShortcutList(QList<QKeySequence> *list)
{
    for (int i = 0; i < list->size(); ++i) {
        const QKeySequence &ks = list->at(i);
        int other = list->indexOf(ks, i + 1);
        while (other != -1) {
            list->removeAt(other);
            other = list->indexOf(ks, other);
        }
    }
}

/* Initialize the accelerator @p id by checking if it is overridden
    in the configuration file (and if it isn't, use the default).
    On X11, if QApplication was initialized with GUI disabled,
    the default will always be used.
*/
void initialize(StandardShortcut id)
{
    KStandardShortcutInfo *info = guardedStandardShortcutInfo(id);

    // All three are needed.
    if (info->id != AccelNone) {
        Q_ASSERT(info->description.text);
        Q_ASSERT(info->description.context);
        Q_ASSERT(info->name);
    }

    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("Shortcuts"));

    if (cg.hasKey(info->name)) {
        QString s = cg.readEntry(info->name);
        if (s != QLatin1String("none")) {
            info->actionShortcuts = QKeySequence::listFromString(s);
            sanitizeShortcutList(&info->actionShortcuts);
        } else {
            info->actionShortcuts = QList<QKeySequence>();
        }
    } else {
        info->actionShortcuts = hardcodedDefaultShortcut(id);
    }

    info->isInitialized = true;
}

void saveShortcut(StandardShortcut id, const QList<QKeySequence> &newShortcut)
{
    KStandardShortcutInfo *info = guardedStandardShortcutInfo(id);
    // If the action has no standard shortcut associated there is nothing to
    // save
    if (info->id == AccelNone) {
        return;
    }

    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("Shortcuts"));

    info->actionShortcuts = newShortcut;
    bool sameAsDefault = (newShortcut == hardcodedDefaultShortcut(id));

    if (sameAsDefault) {
        // If the shortcut is the equal to the hardcoded one we remove it from
        // kdeglobal if necessary and return.
        if (cg.hasKey(info->name)) {
            cg.deleteEntry(info->name, KConfig::Global | KConfig::Persistent | KConfig::Notify);
            cg.sync();
        }

        return;
    }

    // Write the changed shortcut to kdeglobals
    sanitizeShortcutList(&info->actionShortcuts);
    cg.writeEntry(info->name, QKeySequence::listToString(info->actionShortcuts), KConfig::Global | KConfig::Persistent | KConfig::Notify);
    cg.sync();
}

QString name(StandardShortcut id)
{
    return QString::fromLatin1(guardedStandardShortcutInfo(id)->name);
}

QString label(StandardShortcut id)
{
    KStandardShortcutInfo *info = guardedStandardShortcutInfo(id);
    return QCoreApplication::translate("KStandardShortcut", info->description.text, info->description.context);
}

// TODO: Add psWhatsThis entry to KStandardShortcutInfo
QString whatsThis(StandardShortcut /*id*/)
{
    //  KStandardShortcutInfo* info = guardedStandardShortcutInfo( id );
    //  if( info && info->whatsThis )
    //      return i18n(info->whatsThis);
    //  else
    return QString();
}

const QList<QKeySequence> &shortcut(StandardShortcut id)
{
    KStandardShortcutInfo *info = guardedStandardShortcutInfo(id);

    if (!info->isInitialized) {
        initialize(id);
    }

    return info->actionShortcuts;
}

StandardShortcut find(const QKeySequence &seq)
{
    if (!seq.isEmpty()) {
        for (const KStandardShortcutInfo &shortcutInfo : g_infoStandardShortcut) {
            const StandardShortcut id = shortcutInfo.id;
            if (id != AccelNone) {
                if (!shortcutInfo.isInitialized) {
                    initialize(id);
                }
                if (shortcutInfo.actionShortcuts.contains(seq)) {
                    return id;
                }
            }
        }
    }
    return AccelNone;
}

StandardShortcut findByName(const QString &name)
{
    for (const KStandardShortcutInfo &shortcutInfo : g_infoStandardShortcut) {
        if (QLatin1StringView(shortcutInfo.name) == name) {
            return shortcutInfo.id;
        }
    }
    return AccelNone;
}

QList<QKeySequence> hardcodedDefaultShortcut(StandardShortcut id)
{
    QList<QKeySequence> cut;
    KStandardShortcutInfo *info = guardedStandardShortcutInfo(id);
    return info->defaultShortcuts;
}

Category category(StandardShortcut id)
{
    return guardedStandardShortcutInfo(id)->category;
}

const QList<QKeySequence> &open()
{
    return shortcut(Open);
}
const QList<QKeySequence> &openNew()
{
    return shortcut(New);
}
const QList<QKeySequence> &close()
{
    return shortcut(Close);
}
const QList<QKeySequence> &save()
{
    return shortcut(Save);
}
const QList<QKeySequence> &print()
{
    return shortcut(Print);
}
const QList<QKeySequence> &quit()
{
    return shortcut(Quit);
}
const QList<QKeySequence> &cut()
{
    return shortcut(Cut);
}
const QList<QKeySequence> &copy()
{
    return shortcut(Copy);
}
const QList<QKeySequence> &paste()
{
    return shortcut(Paste);
}
const QList<QKeySequence> &pasteSelection()
{
    return shortcut(PasteSelection);
}
const QList<QKeySequence> &deleteWordBack()
{
    return shortcut(DeleteWordBack);
}
const QList<QKeySequence> &deleteWordForward()
{
    return shortcut(DeleteWordForward);
}
const QList<QKeySequence> &undo()
{
    return shortcut(Undo);
}
const QList<QKeySequence> &redo()
{
    return shortcut(Redo);
}
const QList<QKeySequence> &find()
{
    return shortcut(Find);
}
const QList<QKeySequence> &findNext()
{
    return shortcut(FindNext);
}
const QList<QKeySequence> &findPrev()
{
    return shortcut(FindPrev);
}
const QList<QKeySequence> &replace()
{
    return shortcut(Replace);
}
const QList<QKeySequence> &home()
{
    return shortcut(Home);
}
const QList<QKeySequence> &begin()
{
    return shortcut(Begin);
}
const QList<QKeySequence> &end()
{
    return shortcut(End);
}
const QList<QKeySequence> &beginningOfLine()
{
    return shortcut(BeginningOfLine);
}
const QList<QKeySequence> &endOfLine()
{
    return shortcut(EndOfLine);
}
const QList<QKeySequence> &prior()
{
    return shortcut(Prior);
}
const QList<QKeySequence> &next()
{
    return shortcut(Next);
}
const QList<QKeySequence> &backwardWord()
{
    return shortcut(BackwardWord);
}
const QList<QKeySequence> &forwardWord()
{
    return shortcut(ForwardWord);
}
const QList<QKeySequence> &gotoLine()
{
    return shortcut(GotoLine);
}
const QList<QKeySequence> &addBookmark()
{
    return shortcut(AddBookmark);
}
const QList<QKeySequence> &tabNext()
{
    return shortcut(TabNext);
}
const QList<QKeySequence> &tabPrev()
{
    return shortcut(TabPrev);
}
const QList<QKeySequence> &fullScreen()
{
    return shortcut(FullScreen);
}
const QList<QKeySequence> &zoomIn()
{
    return shortcut(ZoomIn);
}
const QList<QKeySequence> &zoomOut()
{
    return shortcut(ZoomOut);
}
const QList<QKeySequence> &help()
{
    return shortcut(Help);
}
const QList<QKeySequence> &completion()
{
    return shortcut(TextCompletion);
}
const QList<QKeySequence> &prevCompletion()
{
    return shortcut(PrevCompletion);
}
const QList<QKeySequence> &nextCompletion()
{
    return shortcut(NextCompletion);
}
const QList<QKeySequence> &rotateUp()
{
    return shortcut(RotateUp);
}
const QList<QKeySequence> &rotateDown()
{
    return shortcut(RotateDown);
}
const QList<QKeySequence> &substringCompletion()
{
    return shortcut(SubstringCompletion);
}
const QList<QKeySequence> &whatsThis()
{
    return shortcut(WhatsThis);
}
const QList<QKeySequence> &reload()
{
    return shortcut(Reload);
}
const QList<QKeySequence> &selectAll()
{
    return shortcut(SelectAll);
}
const QList<QKeySequence> &up()
{
    return shortcut(Up);
}
const QList<QKeySequence> &back()
{
    return shortcut(Back);
}
const QList<QKeySequence> &forward()
{
    return shortcut(Forward);
}
const QList<QKeySequence> &showMenubar()
{
    return shortcut(ShowMenubar);
}
const QList<QKeySequence> &deleteFile()
{
    return shortcut(DeleteFile);
}
const QList<QKeySequence> &renameFile()
{
    return shortcut(RenameFile);
}
const QList<QKeySequence> &createFolder()
{
    return shortcut(CreateFolder);
}
const QList<QKeySequence> &moveToTrash()
{
    return shortcut(MoveToTrash);
}
const QList<QKeySequence> &preferences()
{
    return shortcut(Preferences);
}
const QList<QKeySequence> &showHideHiddenFiles()
{
    return shortcut(ShowHideHiddenFiles);
}
const QList<QKeySequence> &openMainMenu()
{
    return shortcut(OpenMainMenu);
}
const QList<QKeySequence> &openContextMenu()
{
    return shortcut(OpenContextMenu);
}
}
