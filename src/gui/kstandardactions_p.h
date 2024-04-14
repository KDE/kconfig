/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999, 2000 Kurt Granroth <granroth@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

#include <QAction>
#include <QGuiApplication>

#include <KStandardActions>
#include <KStandardShortcut>

#include <string>

namespace KStandardActions
{

/**
 * Helper class for storing raw data in static tables which can be used for QString instance
 * creation at runtime without copying/converting to new memalloc'ed memory, as well as avoiding
 * that way storing the strings directly as QStrings resulting in non-constexpr init code on
 * library loading
 * @internal
 */
struct RawStringData {
    template<std::size_t StringSize>
    constexpr inline RawStringData(const char16_t (&_data)[StringSize])
        : data(_data)
        , size(std::char_traits<char16_t>::length(_data))
    {
    }
    constexpr inline RawStringData(std::nullptr_t)
    {
    }
    constexpr inline RawStringData() = default;

    inline QString toString() const
    {
        if (!data) {
            return QString();
        }

        return Qt::Literals::StringLiterals::operator""_s(data, size);
    }

private:
    const char16_t *const data = nullptr;
    const std::size_t size = 0;
};

/**
 * @internal
 */
struct KStandardActionsInfo {
    KStandardActions::StandardAction id;
    KStandardShortcut::StandardShortcut idAccel;
    const RawStringData psName;
    const char *psLabel;
    const char *psToolTip;
    const RawStringData psIconName;
};
// clang-format off
static const KStandardActionsInfo g_rgActionInfo[] = {
    { New,           KStandardShortcut::New, u"file_new", QT_TR_NOOP("&New"), QT_TR_NOOP("Create new document"), u"document-new" },
    { Open,          KStandardShortcut::Open, u"file_open", QT_TR_NOOP("&Open…"), QT_TR_NOOP("Open an existing document"), u"document-open" },
    { OpenRecent,    KStandardShortcut::AccelNone, u"file_open_recent", QT_TR_NOOP("Open &Recent"), QT_TR_NOOP("Open a document which was recently opened"), u"document-open-recent" },
    { Save,          KStandardShortcut::Save, u"file_save", QT_TR_NOOP("&Save"), QT_TR_NOOP("Save document"), u"document-save" },
    { SaveAs,        KStandardShortcut::SaveAs, u"file_save_as", QT_TR_NOOP("Save &As…"), QT_TR_NOOP("Save document under a new name"), u"document-save-as" },
    { Revert,        KStandardShortcut::Revert, u"file_revert", QT_TR_NOOP("Re&vert"), QT_TR_NOOP("Revert unsaved changes made to document"), u"document-revert" },
    { Close,         KStandardShortcut::Close, u"file_close", QT_TR_NOOP("&Close"), QT_TR_NOOP("Close document"), u"document-close" },
    { Print,         KStandardShortcut::Print, u"file_print", QT_TR_NOOP("&Print…"), QT_TR_NOOP("Print document"), u"document-print" },
    { PrintPreview,  KStandardShortcut::PrintPreview, u"file_print_preview", QT_TR_NOOP("Print Previe&w"), QT_TR_NOOP("Show a print preview of document"), u"document-print-preview" },
    { Mail,          KStandardShortcut::Mail, u"file_mail", QT_TR_NOOP("&Mail…"), QT_TR_NOOP("Send document by mail"), u"mail-send" },
    { Quit,          KStandardShortcut::Quit, u"file_quit", QT_TR_NOOP("&Quit"), QT_TR_NOOP("Quit application"), u"application-exit" },

    { Undo,          KStandardShortcut::Undo, u"edit_undo", QT_TR_NOOP("&Undo"), QT_TR_NOOP("Undo last action"), u"edit-undo" },
    { Redo,          KStandardShortcut::Redo, u"edit_redo", QT_TR_NOOP("Re&do"), QT_TR_NOOP("Redo last undone action"), u"edit-redo" },
    { Cut,           KStandardShortcut::Cut, u"edit_cut", QT_TR_NOOP("Cu&t"), QT_TR_NOOP("Cut selection to clipboard"), u"edit-cut" },
    { Copy,          KStandardShortcut::Copy, u"edit_copy", QT_TR_NOOP("&Copy"), QT_TR_NOOP("Copy selection to clipboard"), u"edit-copy" },
    { Paste,         KStandardShortcut::Paste, u"edit_paste", QT_TR_NOOP("&Paste"), QT_TR_NOOP("Paste clipboard content"), u"edit-paste" },
    { Clear,         KStandardShortcut::Clear, u"edit_clear", QT_TR_NOOP("C&lear"), {}, u"edit-clear" },
    { SelectAll,     KStandardShortcut::SelectAll, u"edit_select_all", QT_TR_NOOP("Select &All"), {}, u"edit-select-all" },
    { Deselect,      KStandardShortcut::Deselect, u"edit_deselect", QT_TR_NOOP("Dese&lect"), {}, u"edit-select-none" },
    { Find,          KStandardShortcut::Find, u"edit_find", QT_TR_NOOP("&Find…"), {}, u"edit-find" },
    { FindNext,      KStandardShortcut::FindNext, u"edit_find_next", QT_TR_NOOP("Find &Next"), {}, u"go-down-search" },
    { FindPrev,      KStandardShortcut::FindPrev, u"edit_find_prev", QT_TR_NOOP("Find Pre&vious"), {}, u"go-up-search" },
    { Replace,       KStandardShortcut::Replace, u"edit_replace", QT_TR_NOOP("&Replace…"), {}, u"edit-find-replace" },

    { ActualSize,    KStandardShortcut::ActualSize, u"view_actual_size", QT_TR_NOOP("Zoom to &Actual Size"), QT_TR_NOOP("View document at its actual size"), u"zoom-original" },
    { FitToPage,     KStandardShortcut::FitToPage, u"view_fit_to_page", QT_TR_NOOP("&Fit to Page"), QT_TR_NOOP("Zoom to fit page in window"), u"zoom-fit-page" },
    { FitToWidth,    KStandardShortcut::FitToWidth, u"view_fit_to_width", QT_TR_NOOP("Fit to Page &Width"), QT_TR_NOOP("Zoom to fit page width in window"), u"zoom-fit-width" },
    { FitToHeight,   KStandardShortcut::FitToHeight, u"view_fit_to_height", QT_TR_NOOP("Fit to Page &Height"), QT_TR_NOOP("Zoom to fit page height in window"), u"zoom-fit-height" },
    { ZoomIn,        KStandardShortcut::ZoomIn, u"view_zoom_in", QT_TR_NOOP("Zoom &In"), {}, u"zoom-in" },
    { ZoomOut,       KStandardShortcut::ZoomOut, u"view_zoom_out", QT_TR_NOOP("Zoom &Out"), {}, u"zoom-out" },
    { Zoom,          KStandardShortcut::Zoom, u"view_zoom", QT_TR_NOOP("&Zoom…"), QT_TR_NOOP("Select zoom level"), u"zoom" },
    { Redisplay,     KStandardShortcut::Reload, u"view_redisplay", QT_TR_NOOP("&Refresh"), QT_TR_NOOP("Refresh document"), u"view-refresh" },

    { Up,            KStandardShortcut::Up, u"go_up", QT_TR_NOOP("&Up"), QT_TR_NOOP("Go up"), u"go-up" },
    // The following three have special i18n() needs for sLabel
    { Back,          KStandardShortcut::Back, u"go_back", {}, {}, u"go-previous" },
    { Forward,       KStandardShortcut::Forward, u"go_forward", {}, {}, u"go-next" },
    { Home,          KStandardShortcut::Home, u"go_home", {}, {}, u"go-home" },
    { Prior,         KStandardShortcut::Prior, u"go_previous", QT_TR_NOOP("&Previous Page"), QT_TR_NOOP("Go to previous page"), u"go-previous-view-page" },
    { Next,          KStandardShortcut::Next, u"go_next", QT_TR_NOOP("&Next Page"), QT_TR_NOOP("Go to next page"), u"go-next-view-page" },
    { Goto,          KStandardShortcut::Goto, u"go_goto", QT_TR_NOOP("&Go To…"), {}, {} },
    { GotoPage,      KStandardShortcut::GotoPage, u"go_goto_page", QT_TR_NOOP("&Go to Page…"), {}, u"go-jump" },
    { GotoLine,      KStandardShortcut::GotoLine, u"go_goto_line", QT_TR_NOOP("&Go to Line…"), {}, {} },
    { FirstPage,     KStandardShortcut::Begin, u"go_first", QT_TR_NOOP("&First Page"), QT_TR_NOOP("Go to first page"), u"go-first-view-page" },
    { LastPage,      KStandardShortcut::End, u"go_last", QT_TR_NOOP("&Last Page"), QT_TR_NOOP("Go to last page"), u"go-last-view-page" },
    { DocumentBack,  KStandardShortcut::DocumentBack, u"go_document_back", QT_TR_NOOP("&Back"), QT_TR_NOOP("Go back in document"), u"go-previous" },
    { DocumentForward, KStandardShortcut::DocumentForward, u"go_document_forward", QT_TR_NOOP("&Forward"), QT_TR_NOOP("Go forward in document"), u"go-next" },

    { AddBookmark,   KStandardShortcut::AddBookmark, u"bookmark_add", QT_TR_NOOP("&Add Bookmark"), {}, u"bookmark-new" },
    { EditBookmarks, KStandardShortcut::EditBookmarks, u"bookmark_edit", QT_TR_NOOP("&Edit Bookmarks…"), {}, u"bookmarks-organize" },

    { Spelling,      KStandardShortcut::Spelling, u"tools_spelling", QT_TR_NOOP("&Spelling…"), QT_TR_NOOP("Check spelling in document"), u"tools-check-spelling" },

    { ShowMenubar,   KStandardShortcut::ShowMenubar, u"options_show_menubar", QT_TR_NOOP("Show &Menubar"), QT_TR_NOOP("Show or hide menubar"), u"show-menu" },
    { ShowToolbar,   KStandardShortcut::ShowToolbar, u"options_show_toolbar", QT_TR_NOOP("Show &Toolbar"), QT_TR_NOOP("Show or hide toolbar"), {} },
    { ShowStatusbar, KStandardShortcut::ShowStatusbar, u"options_show_statusbar", QT_TR_NOOP("Show St&atusbar"), QT_TR_NOOP("Show or hide statusbar"), {} },
    { FullScreen,    KStandardShortcut::FullScreen, u"fullscreen", QT_TR_NOOP("F&ull Screen Mode"), {}, u"view-fullscreen" },
    { KeyBindings,   KStandardShortcut::KeyBindings, u"options_configure_keybinding", QT_TR_NOOP("Configure Keyboard S&hortcuts…"), {}, u"configure-shortcuts" },
    { Preferences,   KStandardShortcut::Preferences, u"options_configure", QT_TR_NOOP("&Configure %1…"), {}, u"configure" },
    { ConfigureToolbars, KStandardShortcut::ConfigureToolbars, u"options_configure_toolbars", QT_TR_NOOP("Configure Tool&bars…"), {}, u"configure-toolbars" },
    { ConfigureNotifications, KStandardShortcut::ConfigureNotifications, u"options_configure_notifications", QT_TR_NOOP("Configure &Notifications…"), {}, u"preferences-desktop-notification" },

    // the idea here is that Contents is used in menus, and Help in dialogs, so both share the same
    // shortcut
    { HelpContents,  KStandardShortcut::Help, u"help_contents", QT_TR_NOOP("%1 &Handbook"), {}, u"help-contents" },
    { WhatsThis,     KStandardShortcut::WhatsThis, u"help_whats_this", QT_TR_NOOP("What's &This?"), {}, u"help-contextual" },
    { ReportBug,     KStandardShortcut::ReportBug, u"help_report_bug", QT_TR_NOOP("&Report Bug…"), {}, u"tools-report-bug" },
    { SwitchApplicationLanguage, KStandardShortcut::SwitchApplicationLanguage, u"switch_application_language", QT_TR_NOOP("Configure &Language…"), {}, u"preferences-desktop-locale" },
    { AboutApp,      KStandardShortcut::AccelNone, u"help_about_app", QT_TR_NOOP("&About %1"), {}, nullptr },
    { AboutKDE,      KStandardShortcut::AccelNone, u"help_about_kde", QT_TR_NOOP("About &KDE"), {}, u"kde" },
    { DeleteFile,    KStandardShortcut::DeleteFile, u"deletefile", QT_TR_NOOP("&Delete"), {}, u"edit-delete" },
    { RenameFile,    KStandardShortcut::RenameFile, u"renamefile", QT_TR_NOOP("&Rename…"), {}, u"edit-rename" },
    { MoveToTrash,   KStandardShortcut::MoveToTrash, u"movetotrash", QT_TR_NOOP("&Move to Trash"), {}, u"trash-empty" },
    { Donate,        KStandardShortcut::Donate, u"help_donate", QT_TR_NOOP("&Donate"), {}, u"help-donate"},
    { HamburgerMenu, KStandardShortcut::OpenMainMenu, u"hamburger_menu", QT_TR_NOOP("Open &Menu"), {}, u"application-menu" },
    { ActionNone,    KStandardShortcut::AccelNone, nullptr, {}, {}, nullptr }
};
// clang-format on

/**
 * @internal
 */
inline const KStandardActionsInfo *infoPtr(StandardAction id)
{
    for (uint i = 0; g_rgActionInfo[i].id != ActionNone; i++) {
        if (g_rgActionInfo[i].id == id) {
            return &g_rgActionInfo[i];
        }
    }

    return nullptr;
}

/**
 * @internal
 */
static inline QStringList internal_stdNames()
{
    QStringList result;

    for (uint i = 0; g_rgActionInfo[i].id != ActionNone; i++)
        if (!QCoreApplication::tr(g_rgActionInfo[i].psLabel).isEmpty()) {
            if (QByteArrayView(g_rgActionInfo[i].psLabel).contains("%1"))
            // Prevents KLocalizedString::toString() from complaining about unsubstituted placeholder.
            {
                result.append(QCoreApplication::tr(g_rgActionInfo[i].psLabel).arg(QString()));
            } else {
                result.append(QCoreApplication::tr(g_rgActionInfo[i].psLabel));
            }
        }

    return result;
}
}
