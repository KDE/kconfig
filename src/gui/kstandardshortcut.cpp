/* This file is part of the KDE libraries
    Copyright (C) 1997 Stefan Taferner (taferner@alpin.or.at)
    Copyright (C) 2000 Nicolas Hadacek (haadcek@kde.org)
    Copyright (C) 2001,2002 Ellis Whitehead (ellis@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kstandardshortcut.h"

#include "kconfig.h"
#include "ksharedconfig.h"
#include <kconfiggroup.h>

#include <QCoreApplication>
#include <QDebug>
#include <QKeySequence>

namespace KStandardShortcut
{

struct KStandardShortcutInfo {
    //! The standard shortcut id. @see StandardShortcut
    StandardShortcut id;

    /**
     * Unique name for the given accel. The name is used to save the user
     * settings. It's not representable. Use description for that.
     * @warning NEVER EVER CHANGE IT OR TRANSLATE IT!
     */
    const char *name;

    //! Localized label for user-visible display
    const char *description;

    //! The keys for this shortcut
    int cutDefault, cutDefault2;

    //! A shortcut that is created with @a cutDefault and @cutDefault2
    QList<QKeySequence> cut;

    //! If this struct is initialized. If not initialized @cut is not valid
    bool isInitialized;
};

#define CTRL(x) Qt::CTRL+Qt::Key_##x
#define SHIFT(x) Qt::SHIFT+Qt::Key_##x
#define CTRLSHIFT(x) Qt::CTRL+Qt::SHIFT+Qt::Key_##x
#define ALT(x) Qt::ALT+Qt::Key_##x
#define ALTSHIFT(x) Qt::ALT+Qt::SHIFT+Qt::Key_##x

/** Array of predefined KStandardShortcutInfo objects, which cover all
    the "standard" accelerators. Each enum value from StandardShortcut
    should appear in this table.
*/
// STUFF WILL BREAK IF YOU DON'T READ THIS!!!
// Read the comments of the big enum in kstandardshortcut.h before you change anything!
static KStandardShortcutInfo g_infoStandardShortcut[] = {
//Group File,
    { AccelNone, 0, 0, 0, 0, QList<QKeySequence>(), false },
    { Open, "Open", (QT_TRANSLATE_NOOP("KStandardShortcut", "Open")), CTRL(O), 0, QList<QKeySequence>(), false },
    { New, "New", QT_TRANSLATE_NOOP("KStandardShortcut", "New"), CTRL(N), 0, QList<QKeySequence>(), false },
    { Close, "Close", QT_TRANSLATE_NOOP("KStandardShortcut", "Close"), CTRL(W), CTRL(Escape), QList<QKeySequence>(), false },
    { Save, "Save", QT_TRANSLATE_NOOP("KStandardShortcut", "Save"), CTRL(S), 0, QList<QKeySequence>(), false },
    { Print, "Print", QT_TRANSLATE_NOOP("KStandardShortcut", "Print"), CTRL(P), 0, QList<QKeySequence>(), false },
    { Quit, "Quit", QT_TRANSLATE_NOOP("KStandardShortcut", "Quit"), CTRL(Q), 0, QList<QKeySequence>(), false },

//Group Edit
    { Undo, "Undo", QT_TRANSLATE_NOOP("KStandardShortcut", "Undo"), CTRL(Z), 0, QList<QKeySequence>(), false },
    { Redo, "Redo", QT_TRANSLATE_NOOP("KStandardShortcut", "Redo"), CTRLSHIFT(Z), 0, QList<QKeySequence>(), false },
    { Cut, "Cut", QT_TRANSLATE_NOOP("KStandardShortcut", "Cut"), CTRL(X), SHIFT(Delete), QList<QKeySequence>(), false },
    { Copy, "Copy", QT_TRANSLATE_NOOP("KStandardShortcut", "Copy"), CTRL(C), CTRL(Insert), QList<QKeySequence>(), false },
    { Paste, "Paste", QT_TRANSLATE_NOOP("KStandardShortcut", "Paste"), CTRL(V), SHIFT(Insert), QList<QKeySequence>(), false },
    { PasteSelection, "Paste Selection", QT_TRANSLATE_NOOP("KStandardShortcut", "Paste Selection"), CTRLSHIFT(Insert), 0, QList<QKeySequence>(), false },

    { SelectAll, "SelectAll", QT_TRANSLATE_NOOP("KStandardShortcut", "Select All"), CTRL(A), 0, QList<QKeySequence>(), false },
    { Deselect, "Deselect", QT_TRANSLATE_NOOP("KStandardShortcut", "Deselect"), CTRLSHIFT(A), 0, QList<QKeySequence>(), false },
    { DeleteWordBack, "DeleteWordBack", QT_TRANSLATE_NOOP("KStandardShortcut", "Delete Word Backwards"), CTRL(Backspace), 0, QList<QKeySequence>(), false },
    { DeleteWordForward, "DeleteWordForward", QT_TRANSLATE_NOOP("KStandardShortcut", "Delete Word Forward"), CTRL(Delete), 0, QList<QKeySequence>(), false },

    { Find, "Find", QT_TRANSLATE_NOOP("KStandardShortcut", "Find"), CTRL(F), 0, QList<QKeySequence>(), false },
    { FindNext, "FindNext", QT_TRANSLATE_NOOP("KStandardShortcut", "Find Next"), Qt::Key_F3, 0, QList<QKeySequence>(), false },
    { FindPrev, "FindPrev", QT_TRANSLATE_NOOP("KStandardShortcut", "Find Prev"), SHIFT(F3), 0, QList<QKeySequence>(), false },
    { Replace, "Replace", QT_TRANSLATE_NOOP("KStandardShortcut", "Replace"), CTRL(R), 0, QList<QKeySequence>(), false },

//Group Navigation
    { Home, "Home", QT_TRANSLATE_NOOP("KStandardShortcut", "Home"), ALT(Home), Qt::Key_HomePage, QList<QKeySequence>(), false },
    { Begin, "Begin", QT_TRANSLATE_NOOP("KStandardShortcut", "Begin"), CTRL(Home), 0, QList<QKeySequence>(), false },
    { End, "End", QT_TRANSLATE_NOOP("KStandardShortcut", "End"), CTRL(End), 0, QList<QKeySequence>(), false },
    { Prior, "Prior", QT_TRANSLATE_NOOP("KStandardShortcut", "Prior"), Qt::Key_PageUp, 0, QList<QKeySequence>(), false },
    { Next, "Next", QT_TRANSLATE_NOOP("KStandardShortcut", "Next"), Qt::Key_PageDown, 0, QList<QKeySequence>(), false },

    { Up, "Up", QT_TRANSLATE_NOOP("KStandardShortcut", "Up"), ALT(Up), 0, QList<QKeySequence>(), false },
    { Back, "Back", QT_TRANSLATE_NOOP("KStandardShortcut", "Back"), ALT(Left), Qt::Key_Back, QList<QKeySequence>(), false },
    { Forward, "Forward", QT_TRANSLATE_NOOP("KStandardShortcut", "Forward"), ALT(Right), Qt::Key_Forward, QList<QKeySequence>(), false },
    { Reload, "Reload", QT_TRANSLATE_NOOP("KStandardShortcut", "Reload"), Qt::Key_F5, Qt::Key_Refresh, QList<QKeySequence>(), false },

    { BeginningOfLine, "BeginningOfLine", QT_TRANSLATE_NOOP("KStandardShortcut", "Beginning of Line"), Qt::Key_Home, 0, QList<QKeySequence>(), false },
    { EndOfLine, "EndOfLine", QT_TRANSLATE_NOOP("KStandardShortcut", "End of Line"), Qt::Key_End, 0, QList<QKeySequence>(), false },
    { GotoLine, "GotoLine", QT_TRANSLATE_NOOP("KStandardShortcut", "Go to Line"), CTRL(G), 0, QList<QKeySequence>(), false },
    { BackwardWord, "BackwardWord", QT_TRANSLATE_NOOP("KStandardShortcut", "Backward Word"), CTRL(Left), 0, QList<QKeySequence>(), false },
    { ForwardWord, "ForwardWord", QT_TRANSLATE_NOOP("KStandardShortcut", "Forward Word"), CTRL(Right), 0, QList<QKeySequence>(), false },

    { AddBookmark, "AddBookmark", QT_TRANSLATE_NOOP("KStandardShortcut", "Add Bookmark"), CTRL(B), 0, QList<QKeySequence>(), false },
    { ZoomIn, "ZoomIn", QT_TRANSLATE_NOOP("KStandardShortcut", "Zoom In"), CTRL(Plus), CTRL(Equal), QList<QKeySequence>(), false },
    { ZoomOut, "ZoomOut", QT_TRANSLATE_NOOP("KStandardShortcut", "Zoom Out"), CTRL(Minus), 0, QList<QKeySequence>(), false },
    { FullScreen, "FullScreen", QT_TRANSLATE_NOOP("KStandardShortcut", "Full Screen Mode"), CTRLSHIFT(F), 0, QList<QKeySequence>(), false },

    { ShowMenubar, "ShowMenubar", QT_TRANSLATE_NOOP("KStandardShortcut", "Show Menu Bar"), CTRL(M), 0, QList<QKeySequence>(), false },
    { TabNext, "Activate Next Tab", QT_TRANSLATE_NOOP("KStandardShortcut", "Activate Next Tab"), CTRL(Period), CTRL(BracketRight), QList<QKeySequence>(), false },
    { TabPrev, "Activate Previous Tab", QT_TRANSLATE_NOOP("KStandardShortcut", "Activate Previous Tab"), CTRL(Comma), CTRL(BracketLeft), QList<QKeySequence>(), false },

    //Group Help
    { Help, "Help", QT_TRANSLATE_NOOP("KStandardShortcut", "Help"), Qt::Key_F1, 0, QList<QKeySequence>(), false },
    { WhatsThis, "WhatsThis", QT_TRANSLATE_NOOP("KStandardShortcut", "What's This"), SHIFT(F1), 0, QList<QKeySequence>(), false },

//Group TextCompletion
    { TextCompletion, "TextCompletion", QT_TRANSLATE_NOOP("KStandardShortcut", "Text Completion"), CTRL(E), 0, QList<QKeySequence>(), false },
    { PrevCompletion, "PrevCompletion", QT_TRANSLATE_NOOP("KStandardShortcut", "Previous Completion Match"), CTRL(Up), 0, QList<QKeySequence>(), false },
    { NextCompletion, "NextCompletion", QT_TRANSLATE_NOOP("KStandardShortcut", "Next Completion Match"), CTRL(Down), 0, QList<QKeySequence>(), false },
    { SubstringCompletion, "SubstringCompletion", QT_TRANSLATE_NOOP("KStandardShortcut", "Substring Completion"), CTRL(T), 0, QList<QKeySequence>(), false },

    { RotateUp, "RotateUp", QT_TRANSLATE_NOOP("KStandardShortcut", "Previous Item in List"), Qt::Key_Up, 0, QList<QKeySequence>(), false },
    { RotateDown, "RotateDown", QT_TRANSLATE_NOOP("KStandardShortcut", "Next Item in List"), Qt::Key_Down, 0, QList<QKeySequence>(), false },

    { OpenRecent, "OpenRecent", QT_TRANSLATE_NOOP("KStandardShortcut", "Open Recent"), 0, 0, QList<QKeySequence>(), false },
    { SaveAs, "SaveAs", QT_TRANSLATE_NOOP("KStandardShortcut", "Save As"), CTRLSHIFT(S), 0, QList<QKeySequence>(), false },
    { Revert, "Revert", QT_TRANSLATE_NOOP("KStandardShortcut", "Revert"), 0, 0, QList<QKeySequence>(), false },
    { PrintPreview, "PrintPreview", QT_TRANSLATE_NOOP("KStandardShortcut", "Print Preview"), 0, 0, QList<QKeySequence>(), false },
    { Mail, "Mail", QT_TRANSLATE_NOOP("KStandardShortcut", "Mail"), 0, 0, QList<QKeySequence>(), false },
    { Clear, "Clear", QT_TRANSLATE_NOOP("KStandardShortcut", "Clear"), 0, 0, QList<QKeySequence>(), false },
    { ActualSize, "ActualSize", QT_TRANSLATE_NOOP("KStandardShortcut", "Actual Size"), 0, 0, QList<QKeySequence>(), false },
    { FitToPage, "FitToPage", QT_TRANSLATE_NOOP("KStandardShortcut", "Fit To Page"), 0, 0, QList<QKeySequence>(), false },
    { FitToWidth, "FitToWidth", QT_TRANSLATE_NOOP("KStandardShortcut", "Fit To Width"), 0, 0, QList<QKeySequence>(), false },
    { FitToHeight, "FitToHeight", QT_TRANSLATE_NOOP("KStandardShortcut", "Fit To Height"), 0, 0, QList<QKeySequence>(), false },
    { Zoom, "Zoom", QT_TRANSLATE_NOOP("KStandardShortcut", "Zoom"), 0, 0, QList<QKeySequence>(), false },
    { Goto, "Goto", QT_TRANSLATE_NOOP("KStandardShortcut", "Goto"), 0, 0, QList<QKeySequence>(), false },
    { GotoPage, "GotoPage", QT_TRANSLATE_NOOP("KStandardShortcut", "Goto Page"), 0, 0, QList<QKeySequence>(), false },
    { DocumentBack, "DocumentBack", QT_TRANSLATE_NOOP("KStandardShortcut", "Document Back"), ALTSHIFT(Left), 0, QList<QKeySequence>(), false },
    { DocumentForward, "DocumentForward", QT_TRANSLATE_NOOP("KStandardShortcut", "Document Forward"), ALTSHIFT(Right), 0, QList<QKeySequence>(), false },
    { EditBookmarks, "EditBookmarks", QT_TRANSLATE_NOOP("KStandardShortcut", "Edit Bookmarks"), 0, 0, QList<QKeySequence>(), false },
    { Spelling, "Spelling", QT_TRANSLATE_NOOP("KStandardShortcut", "Spelling"), 0, 0, QList<QKeySequence>(), false },
    { ShowToolbar, "ShowToolbar", QT_TRANSLATE_NOOP("KStandardShortcut", "Show Toolbar"), 0, 0, QList<QKeySequence>(), false },
    { ShowStatusbar, "ShowStatusbar", QT_TRANSLATE_NOOP("KStandardShortcut", "Show Statusbar"), 0, 0, QList<QKeySequence>(), false },
    { SaveOptions, "SaveOptions", QT_TRANSLATE_NOOP("KStandardShortcut", "Save Options"), 0, 0, QList<QKeySequence>(), false },
    { KeyBindings, "KeyBindings", QT_TRANSLATE_NOOP("KStandardShortcut", "Key Bindings"), 0, 0, QList<QKeySequence>(), false },
    { Preferences, "Preferences", QT_TRANSLATE_NOOP("KStandardShortcut", "Preferences"), 0, 0, QList<QKeySequence>(), false },
    { ConfigureToolbars, "ConfigureToolbars", QT_TRANSLATE_NOOP("KStandardShortcut", "Configure Toolbars"), 0, 0, QList<QKeySequence>(), false },
    { ConfigureNotifications, "ConfigureNotifications", QT_TRANSLATE_NOOP("KStandardShortcut", "Configure Notifications"), 0, 0, QList<QKeySequence>(), false },
    { TipofDay, "TipofDay", QT_TRANSLATE_NOOP("KStandardShortcut", "Tip Of Day"), 0, 0, QList<QKeySequence>(), false },
    { ReportBug, "ReportBug", QT_TRANSLATE_NOOP("KStandardShortcut", "Report Bug"), 0, 0, QList<QKeySequence>(), false },
    { SwitchApplicationLanguage, "SwitchApplicationLanguage", QT_TRANSLATE_NOOP("KStandardShortcut", "Switch Application Language"), 0, 0, QList<QKeySequence>(), false },
    { AboutApp, "AboutApp", QT_TRANSLATE_NOOP("KStandardShortcut", "About Application"), 0, 0, QList<QKeySequence>(), false },
    { AboutKDE, "AboutKDE", QT_TRANSLATE_NOOP("KStandardShortcut", "About KDE"), 0, 0, QList<QKeySequence>(), false },

    //dummy entry to catch simple off-by-one errors. Insert new entries before this line.
    { AccelNone, 0, 0, 0, 0, QList<QKeySequence>(), false }
};

/** Search for the KStandardShortcutInfo object associated with the given @p id.
    Return a dummy entry with no name and an empty shortcut if @p id is invalid.
*/
static KStandardShortcutInfo *guardedStandardShortcutInfo(StandardShortcut id)
{
    if (id >= static_cast<int>(sizeof(g_infoStandardShortcut) / sizeof(KStandardShortcutInfo)) ||
            id < 0) {
        qWarning() << "KStandardShortcut: id not found!";
        return &g_infoStandardShortcut[AccelNone];
    } else {
        return &g_infoStandardShortcut[id];
    }
}

/** Initialize the accelerator @p id by checking if it is overridden
    in the configuration file (and if it isn't, use the default).
    On X11, if QApplication was initialized with GUI disabled,
    the default will always be used.
*/
static void initialize(StandardShortcut id)
{
    KStandardShortcutInfo *info = guardedStandardShortcutInfo(id);

    // Both are needed.
    if (info->id != AccelNone) {
        Q_ASSERT(info->description);
        Q_ASSERT(info->name);
    }

    KConfigGroup cg(KSharedConfig::openConfig(), "Shortcuts");

    if (cg.hasKey(info->name)) {
        QString s = cg.readEntry(info->name);
        if (s != QLatin1String("none")) {
            info->cut = QKeySequence::listFromString(s);
        } else {
            info->cut = QList<QKeySequence>();
        }
    } else {
        info->cut = hardcodedDefaultShortcut(id);
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

    KConfigGroup cg(KSharedConfig::openConfig(), "Shortcuts");

    info->cut = newShortcut;
    bool sameAsDefault = (newShortcut == hardcodedDefaultShortcut(id));

    if (sameAsDefault) {
        // If the shortcut is the equal to the hardcoded one we remove it from
        // kdeglobal if necessary and return.
        if (cg.hasKey(info->name)) {
            cg.deleteEntry(info->name, KConfig::Global | KConfig::Persistent);
        }

        return;
    }

    // Write the changed shortcut to kdeglobals
    cg.writeEntry(info->name, QKeySequence::listToString(info->cut), KConfig::Global | KConfig::Persistent);
}

QString name(StandardShortcut id)
{
    return QString::fromLatin1(guardedStandardShortcutInfo(id)->name);
}

QString label(StandardShortcut id)
{
    KStandardShortcutInfo *info = guardedStandardShortcutInfo(id);
    return QCoreApplication::translate("KStandardShortcut",
                                       info->description);
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

    return info->cut;
}

StandardShortcut find(const QKeySequence &seq)
{
    if (!seq.isEmpty()) {
        for (uint i = 0; i < sizeof(g_infoStandardShortcut) / sizeof(KStandardShortcutInfo); i++) {
            StandardShortcut id = g_infoStandardShortcut[i].id;
            if (id != AccelNone) {
                if (!g_infoStandardShortcut[i].isInitialized) {
                    initialize(id);
                }
                if (g_infoStandardShortcut[i].cut.contains(seq)) {
                    return id;
                }
            }
        }
    }
    return AccelNone;
}

StandardShortcut find(const char *keyName)
{
    for (uint i = 0; i < sizeof(g_infoStandardShortcut) / sizeof(KStandardShortcutInfo); i++)
        if (qstrcmp(g_infoStandardShortcut[i].name, keyName)) {
            return g_infoStandardShortcut[i].id;
        }

    return AccelNone;
}

QList<QKeySequence> hardcodedDefaultShortcut(StandardShortcut id)
{
    QList<QKeySequence> cut;
    KStandardShortcutInfo *info = guardedStandardShortcutInfo(id);

    if (info->cutDefault != 0) {
        cut << info->cutDefault;
    }

    if (info->cutDefault2 != 0) {
        if (cut.isEmpty()) {
            cut << QKeySequence();
        }

        cut << info->cutDefault2;
    }

    return cut;
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

}
