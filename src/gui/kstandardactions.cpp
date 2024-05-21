/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999, 2000 Kurt Granroth <granroth@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kstandardactions.h"
#include "kstandardactions_p.h"
#include "moc_kstandardactions_p.cpp"

#include <KStandardShortcutWatcher>
#include <QGuiApplication>

namespace KStandardActions
{

QList<StandardAction> actionIds()
{
    QList<StandardAction> result;

    for (uint i = 0; g_rgActionInfo[i].id != ActionNone; i++) {
        result.append(g_rgActionInfo[i].id);
    }

    return result;
}

KStandardShortcut::StandardShortcut shortcutForActionId(StandardAction id)
{
    const KStandardActionsInfo *pInfo = infoPtr(id);
    return (pInfo) ? pInfo->idAccel : KStandardShortcut::AccelNone;
}

QAction *_kgui_createInternal(StandardAction id, QObject *parent)
{
    QAction *pAction = new QAction(parent);
    configureAction(id, pAction);
    return pAction;
}

void configureAction(StandardAction id, QAction *action)
{
    const auto *pInfo = KStandardActions::infoPtr(static_cast<KStandardActions::StandardAction>(id));

    // qCDebug(KCONFIG_WIDGETS_LOG) << "KStandardAction::create( " << id  << "=" << (pInfo ? pInfo->psName.toString() : QString()) << ", " << parent << " )"; //
    // ellis

    if (pInfo) {
        QString sLabel;
        QString iconName = pInfo->psIconName.toString();

        switch (id) {
        case Back:
            sLabel = QCoreApplication::translate("go back", "&Back");
            if (QGuiApplication::isRightToLeft()) {
                iconName = QStringLiteral("go-next");
            }
            break;

        case Forward:
            sLabel = QCoreApplication::translate("go forward", "&Forward");
            if (QGuiApplication::isRightToLeft()) {
                iconName = QStringLiteral("go-previous");
            }
            break;

        case Home:
            sLabel = QCoreApplication::translate("home page", "&Home");
            break;
        case Preferences:
        case AboutApp:
        case HelpContents: {
            QString appDisplayName = QGuiApplication::applicationDisplayName();
            if (appDisplayName.isEmpty()) {
                appDisplayName = QCoreApplication::applicationName();
            }
            sLabel = QCoreApplication::tr(pInfo->psLabel).arg(appDisplayName);
        } break;
        default:
            sLabel = QCoreApplication::tr(pInfo->psLabel);
        }

        if (QGuiApplication::isRightToLeft()) {
            switch (id) {
            case Prior:
                iconName = QStringLiteral("go-next-view-page");
                break;
            case Next:
                iconName = QStringLiteral("go-previous-view-page");
                break;
            case FirstPage:
                iconName = QStringLiteral("go-last-view-page");
                break;
            case LastPage:
                iconName = QStringLiteral("go-first-view-page");
                break;
            case DocumentBack:
                iconName = QStringLiteral("go-next");
                break;
            case DocumentForward:
                iconName = QStringLiteral("go-previous");
                break;
            default:
                break;
            }
        }

        if (id == Donate) {
            const QString currencyCode = QLocale().currencySymbol(QLocale::CurrencyIsoCode).toLower();
            if (!currencyCode.isEmpty()) {
                iconName = QStringLiteral("help-donate-%1").arg(currencyCode);
            }
        }

        QIcon icon = iconName.isEmpty() ? QIcon() : QIcon::fromTheme(iconName);

        switch (id) {
        case ShowMenubar: {
            action->setWhatsThis(QCoreApplication::translate("@info:whatsthis",
                                                             "Show Menubar<p>"
                                                             "Shows the menubar again after it has been hidden</p>"));
            action->setCheckable(true);
            action->setChecked(true);
            break;
        }
        case ShowToolbar:
            action->setCheckable(true);
            action->setChecked(true);
            break;
        case ShowStatusbar:
            action->setWhatsThis(
                QCoreApplication::translate("@info:whatsthis",
                                            "Show Statusbar<p>"
                                            "Shows the statusbar, which is the bar at the bottom of the window used for status information.</p>"));
            action->setCheckable(true);
            action->setChecked(true);
            break;
        case FullScreen:
            action->setCheckable(true);
            break;
        // Same as default, but with the app icon
        case AboutApp: {
            icon = qGuiApp->windowIcon();
            break;
        }
        default:
            break;
        }

        // Set the text before setting the MenuRole, as on OS X setText will do some heuristic role guessing.
        // This ensures user menu items get the intended role out of the list below.
        action->setText(sLabel);

        switch (id) {
        case Quit:
            action->setMenuRole(QAction::QuitRole);
            break;

        case Preferences:
            action->setMenuRole(QAction::PreferencesRole);
            break;

        case AboutApp:
            action->setMenuRole(QAction::AboutRole);
            break;

        default:
            action->setMenuRole(QAction::NoRole);
            break;
        }

        if (!QCoreApplication::tr(pInfo->psToolTip).isEmpty()) {
            action->setToolTip(QCoreApplication::tr(pInfo->psToolTip));
        }
        action->setIcon(icon);

        QList<QKeySequence> cut = KStandardShortcut::shortcut(pInfo->idAccel);
        if (!cut.isEmpty()) {
            // emulate KActionCollection::setDefaultShortcuts to allow the use of "configure shortcuts"
            action->setShortcuts(cut);
            action->setProperty("defaultShortcuts", QVariant::fromValue(cut));
        }
        action->connect(KStandardShortcut::shortcutWatcher(),
                        &KStandardShortcut::StandardShortcutWatcher::shortcutChanged,
                        action,
                        [action, shortcut = pInfo->idAccel](KStandardShortcut::StandardShortcut id, const QList<QKeySequence> &newShortcut) {
                            if (id != shortcut) {
                                return;
                            }
                            action->setShortcuts(newShortcut);
                            action->setProperty("defaultShortcuts", QVariant::fromValue(newShortcut));
                        });

        action->setObjectName(pInfo->psName.toString());
    }

    if (action && action->parent() && action->parent()->inherits("KActionCollection")) {
        QMetaObject::invokeMethod(action->parent(), "addAction", Q_ARG(QString, action->objectName()), Q_ARG(QAction *, action));
    }
}

QString name(StandardAction id)
{
    const KStandardActionsInfo *pInfo = infoPtr(id);
    return (pInfo) ? pInfo->psName.toString() : QString();
}
}
