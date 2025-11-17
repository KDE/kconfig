/*
    SPDX-FileCopyrightText: 2025 Méven Car <meven@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <KConfig>
#include <KConfigGroup>
#include <QBuffer>
#include <QGuiApplication>

#include <kconfigskeleton.h>

#include <QDebug>

#include "test_kconfigconstructor.h"

int main(int argc, char **argv)
{
    QStandardPaths::setTestModeEnabled(true);
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);

    auto buffer = std::make_shared<QBuffer>();
    buffer->open(QIODevice::ReadWrite | QIODevice::Text);
    {
        KConfig initialConfig(buffer);
        KConfigGroup group = initialConfig.group(QStringLiteral("MyOptions"));
        group.writeEntry(QStringLiteral("MyString"), QStringLiteral("The String"));
    }
    buffer->seek(0);
    std::shared_ptr<KConfig> config = std::make_shared<KConfig>(buffer);

    TestKConfigConstructor t(config);

    bool ok = t.myString() == QLatin1String("The String");

    return ok ? 0 : 1;
}
