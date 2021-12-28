// SPDX-FileCopyrightText: 2021 Carson Black
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QPointer>
#include <QTimer>
#include <QWindow>

#include <KConfig>
#include <KSharedConfig>
#include <KWindowConfig>

#include "kwindowconfig.h"
#include "windowstate_p.h"

struct WindowStateSaver::Private
{
    QPointer<QWindow> window;
    QString name;
    QString configName;
    QScopedPointer<QTimer> timer;
};

WindowStateSaver::WindowStateSaver(QObject* parent) : QObject(parent), d(new Private)
{
    d->timer.reset(new QTimer);
    d->timer->setInterval(100);
    connect(d->timer.get(), &QTimer::timeout, this, [=]() {
        save();
    });
}

WindowStateSaver::~WindowStateSaver()
{
    save();
}

QWindow* WindowStateSaver::window() const
{
    return d->window;
}

void WindowStateSaver::setWindow(QWindow* window)
{
    if (d->window == window) {
        return;
    }

    if (d->window) {
        disconnect(d->window, nullptr, this, nullptr);
    }

    if (window) {
        auto deferredSave = [this] {
            d->timer->start();
        };
        connect(window, &QWindow::widthChanged, this, deferredSave);
        connect(window, &QWindow::heightChanged, this, deferredSave);
        connect(window, &QWindow::xChanged, this, deferredSave);
        connect(window, &QWindow::yChanged, this, deferredSave);
    }

    d->window = window;
    Q_EMIT windowChanged();
}

void WindowStateSaver::componentComplete()
{
    connect(this, &WindowStateSaver::windowChanged, this, &WindowStateSaver::restore);
    connect(this, &WindowStateSaver::configNameChanged, this, &WindowStateSaver::save);
    connect(this, &WindowStateSaver::windowNameChanged, this, &WindowStateSaver::save);

    restore();
}

QString WindowStateSaver::windowName() const
{
    return d->name;
}

void WindowStateSaver::setWindowName(const QString& name)
{
    if (d->name == name) {
        return;
    }

    d->name = name;
    Q_EMIT windowNameChanged();
}

QString WindowStateSaver::configName() const
{
    return d->configName;
}

void WindowStateSaver::setConfigName(const QString& name)
{
    if (d->configName == name) {
        return;
    }

    d->configName = name;
    Q_EMIT configNameChanged();
}

void WindowStateSaver::save()
{
    if (!d->window || d->name.isEmpty()) {
        return;
    }

    KSharedConfigPtr cfg(KSharedConfig::openConfig(d->configName, KConfig::SimpleConfig, QStandardPaths::AppDataLocation));

    auto group = cfg->group("KConfig QML").group("Window States").group(d->name);

    KWindowConfig::saveWindowPosition(d->window, group);
    KWindowConfig::saveWindowSize(d->window, group);
}

void WindowStateSaver::restore()
{
    if (!d->window || d->name.isEmpty()) {
        return;
    }

    KSharedConfigPtr cfg(KSharedConfig::openConfig(d->configName));

    const auto group = cfg->group("KConfig QML").group("Window States").group(d->name);

    KWindowConfig::restoreWindowPosition(d->window, group);
    KWindowConfig::restoreWindowSize(d->window, group);
}
