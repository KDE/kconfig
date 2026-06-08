/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KCONFIG_AUTOTESTS_HELPER_H
#define KCONFIG_AUTOTESTS_HELPER_H

#include <QFile>
#include <QLocale>

#include <optional>

/**
 * @brief Helper to store the current default QLocale and resetting to it in the dtor.
 **/
class DefaultLocale
{
public:
    DefaultLocale()
        : m_defaultLocale()
    {
    }
    ~DefaultLocale()
    {
        QLocale::setDefault(m_defaultLocale);
    }

private:
    QLocale m_defaultLocale;
};

/*!
 * RAII-style helper to set an environment variable for a test which is automatically reset after the test.
 */
class EnvironmentVariableOverride
{
public:
    EnvironmentVariableOverride(const char *varName, QByteArrayView value)
        : mVarName{varName}
    {
        if (mVarName.isEmpty()) {
            return;
        }
        if (qEnvironmentVariableIsSet(mVarName.constData())) {
            mOldValue = qgetenv(mVarName.constData());
        }
        qputenv(mVarName.constData(), value);
    }

    ~EnvironmentVariableOverride()
    {
        if (mVarName.isEmpty()) {
            return;
        }
        if (mOldValue.has_value()) {
            qputenv(mVarName.constData(), mOldValue.value());
        } else {
            qunsetenv(mVarName.constData());
        }
    }

private:
    QByteArray mVarName;
    std::optional<QByteArray> mOldValue;
};

static QList<QByteArray> readLinesFrom(const QString &path)
{
    QFile file(path);
    const bool opened = file.open(QIODevice::ReadOnly | QIODevice::Text);
    QList<QByteArray> lines;
    if (!opened) {
        qWarning() << "Failed to open" << path;
        return lines;
    }
    QByteArray line;
    do {
        line = file.readLine();
        if (!line.isEmpty()) {
            lines.append(line);
        }
    } while (!line.isEmpty());
    return lines;
}

#endif
