/*
    SPDX-FileCopyrightText: 2013 Marco Martin <notmart@gmail.com>
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfigpropertymap.h"

#include <KCoreConfigSkeleton>
#include <QJSValue>
#include <QPointer>

class KConfigPropertyMapPrivate
{
public:
    KConfigPropertyMapPrivate(KConfigPropertyMap *map)
        : q(map)
    {
    }

    enum LoadConfigOption {
        DontEmitValueChanged,
        EmitValueChanged,
    };

    void loadConfig(LoadConfigOption option);
    void writeConfig();
    void writeConfigValue(const QString &key, const QVariant &value);

    KConfigPropertyMap *q;
    QPointer<KCoreConfigSkeleton> config;
    bool updatingConfigValue = false;
    bool notify = false;
};

KConfigPropertyMap::KConfigPropertyMap(KCoreConfigSkeleton *config, QObject *parent)
    : QQmlPropertyMap(this, parent)
    , d(new KConfigPropertyMapPrivate(this))
{
    Q_ASSERT(config);
    d->config = config;

    // Reload the config only if the change signal has *not* been emitted by ourselves updating the config
    connect(config, &KCoreConfigSkeleton::configChanged, this, [this]() {
        if (!d->updatingConfigValue) {
            d->loadConfig(KConfigPropertyMapPrivate::EmitValueChanged);
        }
    });
    connect(this, &KConfigPropertyMap::valueChanged, this, [this](const QString &key, const QVariant &value) {
        d->writeConfigValue(key, value);
    });

    d->loadConfig(KConfigPropertyMapPrivate::DontEmitValueChanged);
}

KConfigPropertyMap::~KConfigPropertyMap() = default;

bool KConfigPropertyMap::isNotify() const
{
    return d->notify;
}

void KConfigPropertyMap::setNotify(bool notify)
{
    d->notify = notify;
}

void KConfigPropertyMap::writeConfig()
{
    d->writeConfig();
}

QVariant KConfigPropertyMap::updateValue(const QString &key, const QVariant &input)
{
    Q_UNUSED(key);
    if (input.userType() == qMetaTypeId<QJSValue>()) {
        return input.value<QJSValue>().toVariant();
    }
    return input;
}

bool KConfigPropertyMap::isImmutable(const QString &key) const
{
    KConfigSkeletonItem *item = d->config->findItem(key);
    if (item) {
        return item->isImmutable();
    }

    return false;
}

void KConfigPropertyMapPrivate::loadConfig(KConfigPropertyMapPrivate::LoadConfigOption option)
{
    if (!config) {
        return;
    }

    const auto &items = config->items();
    for (KConfigSkeletonItem *item : items) {
        q->insert(item->key() + QStringLiteral("Default"), item->getDefault());

        if (auto *enumItem = dynamic_cast<KCoreConfigSkeleton::ItemEnum *>(item)) {
            const auto choices = enumItem->choices();
            QVariantHash enumHash;
            enumHash.reserve(choices.size());
            for (int i = 0; i < choices.size(); ++i) {
                const auto &choice = choices.at(i);
                // The property value is the choice's index, Choice value is only its on-disk representation.
                enumHash.insert(choice.name, i);
            }

            const QString enumKey = item->key() + QLatin1String("Enum");
            auto *enumMap = q->value(enumKey).value<QQmlPropertyMap *>();
            // Recreate the enum map only if the keys have actually changed (unlikely but not impossible).
            if (enumMap) {
                QVariantHash existingEnums;
                existingEnums.reserve(enumMap->count());
                const QStringList existingKeys = enumMap->keys();
                for (const QString &existingKey : existingKeys) {
                    existingEnums.insert(existingKey, enumMap->value(existingKey));
                }

                if (existingEnums != enumHash) {
                    delete enumMap;
                    enumMap = nullptr;
                }
            }

            if (!enumMap) {
                enumMap = QQmlPropertyMap::create(q);
                enumMap->insert(enumHash);
                enumMap->freeze();
                q->insert(enumKey, QVariant::fromValue(enumMap));
            }
        }

        q->insert(item->key(), item->property());
        if (option == EmitValueChanged) {
            Q_EMIT q->valueChanged(item->key(), item->property());
        }
    }
}

void KConfigPropertyMapPrivate::writeConfig()
{
    if (!config) {
        return;
    }

    const auto lstItems = config->items();
    for (KConfigSkeletonItem *item : lstItems) {
        item->setWriteFlags(notify ? KConfigBase::Notify : KConfigBase::Normal);
        item->setProperty(q->value(item->key()));
    }
    // Internally sync the config. This way we ensure the config file is written, even if the process crashed
    config->save();
}

void KConfigPropertyMapPrivate::writeConfigValue(const QString &key, const QVariant &value)
{
    if (KConfigSkeletonItem *item = config->findItem(key)) {
        updatingConfigValue = true;
        item->setWriteFlags(notify ? KConfigBase::Notify : KConfigBase::Normal);
        item->setProperty(value);
        updatingConfigValue = false;
    }
}

#include "moc_kconfigpropertymap.cpp"
