/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kwindowstatesaver.h"
#include "ksharedconfig.h"
#include "kwindowconfig.h"

#include <QTimer>
#include <QWindow>

class KWindowStateSaverPrivate
{
public:
    QWindow *window = nullptr;
    KConfigGroup configGroup;
    std::function<QWindow *()> windowHandleCallback;
    int timerId = 0;

    void init(KWindowStateSaver *q);
    void initWidget(QObject *widget, KWindowStateSaver *q);
};

void KWindowStateSaverPrivate::init(KWindowStateSaver *q)
{
    if (!window) {
        return;
    }

    KWindowConfig::restoreWindowSize(window, configGroup);
    KWindowConfig::restoreWindowPosition(window, configGroup);

    const auto saveSize = [q, this]() {
        QTimer::singleShot(50, [q, this]() {
            KWindowConfig::saveWindowSize(window, configGroup);
            if (!timerId) {
                timerId = q->startTimer(std::chrono::seconds(30));
            }
        });
    };
    const auto savePosition = [q, this]() {
        KWindowConfig::saveWindowPosition(window, configGroup);
        if (!timerId) {
            timerId = q->startTimer(std::chrono::seconds(30));
        }
    };

    QObject::connect(window, &QWindow::widthChanged, q, saveSize);
    QObject::connect(window, &QWindow::heightChanged, q, saveSize);
    QObject::connect(window, &QWindow::xChanged, q, savePosition);
    QObject::connect(window, &QWindow::yChanged, q, savePosition);
}

void KWindowStateSaverPrivate::initWidget(QObject *widget, KWindowStateSaver *q)
{
    if (!window && windowHandleCallback) {
        window = windowHandleCallback();
    }
    if (window) {
        init(q);
    } else {
        widget->installEventFilter(q);
    }
}

KWindowStateSaver::KWindowStateSaver(QWindow *window, const KConfigGroup &configGroup)
    : QObject(window)
    , d(new KWindowStateSaverPrivate)
{
    Q_ASSERT(window);
    d->window = window;
    d->configGroup = configGroup;
    d->init(this);
}

KWindowStateSaver::KWindowStateSaver(QWindow *window, const QString &configGroupName)
    : QObject(window)
    , d(new KWindowStateSaverPrivate)
{
    Q_ASSERT(window);
    d->window = window;
    d->configGroup = KConfigGroup(KSharedConfig::openStateConfig(), configGroupName);
    d->init(this);
}

KWindowStateSaver::~KWindowStateSaver()
{
    delete d;
}

void KWindowStateSaver::timerEvent(QTimerEvent *event)
{
    killTimer(event->timerId());
    d->configGroup.sync();
    d->timerId = 0;
}

bool KWindowStateSaver::eventFilter(QObject *watched, QEvent *event)
{
    // QEvent::PlatformSurface would give us a valid window, but if there are
    // initial resizings (explicitly or via layout constraints) those would then
    // already overwrite our restored values. So wait until all that is done
    // and only restore afterwards.
    if (event->type() == QEvent::ShowToParent && !d->window) {
        watched->removeEventFilter(this);
        d->window = d->windowHandleCallback();
        d->init(this);
    }

    return QObject::eventFilter(watched, event);
}

void KWindowStateSaver::initWidget(QObject *widget, const std::function<QWindow *()> &windowHandleCallback, const KConfigGroup &configGroup)
{
    d = new KWindowStateSaverPrivate;
    d->windowHandleCallback = windowHandleCallback;
    d->configGroup = configGroup;
    d->initWidget(widget, this);
}

void KWindowStateSaver::initWidget(QObject *widget, const std::function<QWindow *()> &windowHandleCallback, const QString &configGroupName)
{
    d = new KWindowStateSaverPrivate;
    d->windowHandleCallback = windowHandleCallback;
    d->configGroup = KConfigGroup(KSharedConfig::openStateConfig(), configGroupName);
    d->initWidget(widget, this);
}

#include "moc_kwindowstatesaver.cpp"
