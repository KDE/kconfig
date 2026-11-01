/*
 * SPDX-FileCopyrightText: 2026 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <QTest>

#include <KSharedConfig>

#include <memory>

class KConfigLoader;
class KConfigPropertyMap;

class ConfigPropertyMapTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void init();
    void cleanup();

private Q_SLOTS:
    void boolDefaultValue();
    void colorDefaultValue();
    void dateTimeDefaultValue();
    void enumDefaultValue();
    void enumDefaultValueString();
    void enumMap();
    void enumMapQmlAccess();
    void enumMapIsFrozen();
    void enumMapAfterConfigChange();
    void fontDefaultValue();
    void intDefaultValue();
    void passwordDefaultValue();
    void pathDefaultValue();
    void stringDefaultValue();
    void stringListDefaultValue();
    void uintDefaultValue();
    void urlDefaultValue();
    void doubleDefaultValue();
    void intListEmptyDefaultValue();
    void intListDefaultValue();
    void longLongDefaultValue();
    void pointDefaultValue();
    void pointFDefaultValue();
    void rectDefaultValue();
    void rectFDefaultValue();
    void sizeDefaultValue();
    void sizeFDefaultValue();
    void ulongLongDefaultValue();
    void timeDefaultValue();

    void valueChangedUpdatesConfig();
    void writeConfig();
    void configChangedReloadsValues();
    void isImmutable();
    void qmlValueConversion();

private:
    void initPropertyMap();
    void resetPropertyMap();

    KSharedConfigPtr m_config;
    std::unique_ptr<KConfigLoader> m_loader;
    std::unique_ptr<KConfigPropertyMap> m_propertyMap;
};
