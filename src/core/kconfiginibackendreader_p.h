// SPDX-License-Identifier: LGPL-2.0-or-later
// SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
// SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
// SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>
// SPDX-FileCopyrightText: 2025 Harald Sitter <sitter@kde.org>

#pragma once

#include <QDir>
#include <QFile>
#include <QLockFile>
#include <QSaveFile>

#ifdef Q_OS_ANDROID
#include <QStandardPaths>
#endif

#include <chrono>
#include <qtpreprocessorsupport.h>

#ifndef Q_OS_WIN
#include <unistd.h> // getuid
#endif
#include <sys/types.h> // uid_t

#include "kconfig_core_log_settings.h"

using namespace Qt::StringLiterals;

// Wraps a QLockFile to allow no-op locking when locking is not supported (e.g. QIODevice based backends).
class AbstractLockFile
{
public:
    AbstractLockFile() = default;
    virtual ~AbstractLockFile() = default;
    Q_DISABLE_COPY_MOVE(AbstractLockFile)

    [[nodiscard]] virtual bool tryLock(std::chrono::milliseconds timeout = std::chrono::milliseconds::zero()) = 0;
    [[nodiscard]] virtual bool isLocked() const = 0;
    [[nodiscard]] virtual QString fileName() const = 0;
    virtual void unlock() = 0;
    [[nodiscard]] virtual QLockFile::LockError error() const = 0;
};

class RealLockFile : public AbstractLockFile
{
public:
    RealLockFile(std::unique_ptr<QLockFile> &&lockFile)
        : m_lockFile(std::move(lockFile))
    {
        m_lockFile->setStaleLockTime(std::chrono::seconds{20});
    }

    [[nodiscard]] bool tryLock(std::chrono::milliseconds timeout) override
    {
        return m_lockFile->tryLock(timeout);
    }

    void unlock() override
    {
        m_lockFile->unlock();
    }

    [[nodiscard]] bool isLocked() const override
    {
        return m_lockFile->isLocked();
    }
    QString fileName() const override
    {
        return m_lockFile->fileName();
    }

    [[nodiscard]] QLockFile::LockError error() const override
    {
        return m_lockFile->error();
    }

private:
    std::unique_ptr<QLockFile> m_lockFile;
};

class FakeLockFile : public AbstractLockFile
{
public:
    [[nodiscard]] bool tryLock(std::chrono::milliseconds timeout) override
    {
        Q_UNUSED(timeout)
        return m_locked = true;
    }

    void unlock() override
    {
        m_locked = false;
    }

    [[nodiscard]] bool isLocked() const override
    {
        return m_locked;
    }

    [[nodiscard]] QString fileName() const override
    {
        return {};
    }

    [[nodiscard]] QLockFile::LockError error() const override
    {
        return QLockFile::LockError::NoError;
    }

private:
    bool m_locked = false;
};

class KConfigIniBackendAbstractDevice
{
public:
    struct OpenResult {
        bool shouldHaveDevice;
        std::shared_ptr<QIODevice> device;
    };

    KConfigIniBackendAbstractDevice() = default;
    virtual ~KConfigIniBackendAbstractDevice() = default;
    Q_DISABLE_COPY_MOVE(KConfigIniBackendAbstractDevice)

    // The local file path for path based devices or a pseudo name for all others.
    [[nodiscard]] virtual QString id() const = 0;
    [[nodiscard]] virtual bool isDeviceReadable() const = 0;
    [[nodiscard]] virtual bool canWriteToDevice() const = 0;
    [[nodiscard]] virtual bool writeToDevice(const std::function<void(QIODevice &)> &write) = 0;
    [[nodiscard]] virtual OpenResult open() = 0;
    [[nodiscard]] virtual std::unique_ptr<AbstractLockFile> lockFile() = 0;
    virtual void createEnclosingEntity() = 0;
};

class KConfigIniBackendNullDevice : public KConfigIniBackendAbstractDevice
{
public:
    [[nodiscard]] QString id() const override
    {
        return u"((NullDevice))"_s;
    }

    [[nodiscard]] bool isDeviceReadable() const override
    {
        return false;
    }

    [[nodiscard]] bool canWriteToDevice() const override
    {
        return false;
    }

    [[nodiscard]] bool writeToDevice(const std::function<void(QIODevice &)> &write) override
    {
        Q_UNUSED(write)
        return false;
    }

    [[nodiscard]] OpenResult open() override
    {
        return {.shouldHaveDevice = false, .device = nullptr};
    }

    [[nodiscard]] std::unique_ptr<AbstractLockFile> lockFile() override
    {
        return std::make_unique<FakeLockFile>();
    }

    void createEnclosingEntity() override
    {
    }
};

class KConfigIniBackendPathDevice : public KConfigIniBackendAbstractDevice
{
public:
    explicit KConfigIniBackendPathDevice(const QFileInfo &info)
        : m_fileInfo(info)
    {
        Q_ASSERT_X(m_fileInfo.isAbsolute(), "KConfigIniBackendPathDevice", "File path must be absolute");
        Q_ASSERT_X(!m_fileInfo.filePath().isEmpty(), "KConfigIniBackendPathDevice", "File path must not be empty");
    }

    [[nodiscard]] QString id() const override
    {
        if (m_fileInfo.exists()) {
            return m_fileInfo.canonicalFilePath();
        }

        // The canonical filepath is empty when the file does not exist. Try to approximate it.
        if (const QString dirPath = m_fileInfo.dir().canonicalPath(); !dirPath.isEmpty()) {
            const QString filePath = dirPath + QLatin1Char('/') + m_fileInfo.fileName();
            return filePath;
        }

        // If that doesn't work either then we have no recourse but to return the possibly non-canonical path.
        return m_fileInfo.filePath();
    }

    [[nodiscard]] bool isDeviceReadable() const override
    {
        return m_fileInfo.exists();
    }

    [[nodiscard]] bool canWriteToDevice() const override
    {
        if (m_fileInfo.exists()) {
            return m_fileInfo.isWritable();
        }

        // If the file does not exist, check if the deepest existing dir is writable
        QFileInfo dir(m_fileInfo.absolutePath());
        while (!dir.exists()) {
            QString parent = dir.absolutePath(); // Go up. Can't use cdUp() on non-existing dirs.
            if (parent == dir.filePath()) {
                // no parent
                return false;
            }
            dir.setFile(parent);
        }
        return dir.isDir() && dir.isWritable();
    }

    [[nodiscard]] bool writeToDevice(const std::function<void(QIODevice &)> &write) override
    {
        // After writing our file properties may have changed. Ensure we have the latest data in cache.
        auto refreshFileInfo = qScopeGuard([this]() {
            m_fileInfo.refresh();
        });

        // check if file exists
        QFile::Permissions fileMode = id().startsWith(u"/etc/xdg/"_s) ? QFile::ReadUser | QFile::WriteUser | QFile::ReadGroup | QFile::ReadOther //
                                                                      : QFile::ReadUser | QFile::WriteUser;

        bool createNew = true;

        if (m_fileInfo.exists()) {
#ifdef Q_OS_WIN
            // TODO: getuid does not exist on windows, use GetSecurityInfo and GetTokenInformation instead
            createNew = false;
#else
            if (m_fileInfo.ownerId() == ::getuid()) {
                // Preserve file mode if file exists and is owned by user.
                fileMode = m_fileInfo.permissions();
            } else {
                // File is not owned by user:
                // Don't create new file but write to existing file instead.
                createNew = false;
            }
#endif
        }

        if (createNew) {
            QSaveFile file(m_fileInfo.filePath());
            if (!file.open(QIODevice::WriteOnly)) {
#ifdef Q_OS_ANDROID
                // HACK: when we are dealing with content:// URIs, QSaveFile has to rely on DirectWrite.
                // Otherwise this method returns a false and we're done.
                file.setDirectWriteFallback(true);
                if (!file.open(QIODevice::WriteOnly)) {
                    qWarning(KCONFIG_CORE_LOG) << "Couldn't create a new file:" << m_fileInfo.filePath() << ". Error:" << file.errorString();
                    return false;
                }
#else
                qWarning(KCONFIG_CORE_LOG) << "Couldn't create a new file:" << m_fileInfo.filePath() << ". Error:" << file.errorString();
                return false;
#endif
            }

            file.setTextModeEnabled(true); // to get eol translation
            write(file);

            if (!file.size() && (fileMode == (QFile::ReadUser | QFile::WriteUser))) {
                // File is empty and doesn't have special permissions: delete it.
                file.cancelWriting();

                if (m_fileInfo.exists()) {
                    // also remove the old file in case it existed. this can happen
                    // when we delete all the entries in an existing config file.
                    // if we don't do this, then deletions and revertToDefault's
                    // will mysteriously fail
                    QFile::remove(m_fileInfo.filePath());
                }
            } else {
                // Normal case: Close the file
                if (file.commit()) {
                    QFile::setPermissions(m_fileInfo.filePath(), fileMode);
                    return true;
                }
                // Couldn't write. Disk full?
                qCWarning(KCONFIG_CORE_LOG) << "Couldn't write" << m_fileInfo.filePath() << ". Disk full?";
                return false;
            }
        } else {
            QFile f(m_fileInfo.filePath());

            // Open existing file. *DON'T* create it if it suddenly does not exist!
            if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::ExistingOnly)) {
                return false;
            }

            f.setTextModeEnabled(true);
            write(f);
        }

        return true;
    }

    [[nodiscard]] OpenResult open() override
    {
        auto file = std::make_unique<QFile>(m_fileInfo.filePath());
        if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
            return {.shouldHaveDevice = file->exists(), .device = nullptr};
        }
        return {.shouldHaveDevice = true, .device = std::move(file)};
    }

    void createEnclosingEntity() override
    {
        const QString file = m_fileInfo.filePath();
        if (file.isEmpty()) {
            return; // nothing to do
        }

        // Create the containing dir, maybe it wasn't there
        if (!QDir().mkpath(QFileInfo(file).absolutePath())) {
            qWarning("KConfigIniBackend: Could not create enclosing directory for %s", qPrintable(file));
        }
    }

    [[nodiscard]] std::unique_ptr<AbstractLockFile> lockFile() override
    {
#ifdef Q_OS_ANDROID
        // handle content Uris properly
        if (m_fileInfo.filePath().startsWith(QLatin1String("content://"))) {
            // we can't create file at an arbitrary location, so use internal storage to create one

            // NOTE: filename can be the same, but because this lock is short lived we may never have a collision
            return std::make_unique<RealLockFile>(std::make_unique<QLockFile>(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
                                                                              + QLatin1String("/") + m_fileInfo.fileName() + QLatin1String(".lock")));
        }
        return std::make_unique<RealLockFile>(std::make_unique<QLockFile>(m_fileInfo.filePath() + QLatin1String(".lock")));
#else
        return std::make_unique<RealLockFile>(std::make_unique<QLockFile>(m_fileInfo.filePath() + QLatin1String(".lock")));
#endif
    }

private:
    QFileInfo m_fileInfo;
};

class KConfigIniBackendQIODevice : public KConfigIniBackendAbstractDevice
{
public:
    KConfigIniBackendQIODevice(const std::shared_ptr<QIODevice> &device)
        : m_device(device)
    {
    }

    [[nodiscard]] QString id() const override
    {
        return u"((QIODevice))"_s;
    }

    [[nodiscard]] bool isDeviceReadable() const override
    {
        return m_device->isOpen() && m_device->isReadable();
    }

    [[nodiscard]] bool canWriteToDevice() const override
    {
        return m_device->isOpen() && m_device->isWritable();
    }

    [[nodiscard]] bool writeToDevice(const std::function<void(QIODevice &)> &write) override
    {
        m_device->setTextModeEnabled(true);
        write(*m_device);
        return true;
    }

    [[nodiscard]] OpenResult open() override
    {
        return {.shouldHaveDevice = true, .device = m_device};
    }

    [[nodiscard]] std::unique_ptr<AbstractLockFile> lockFile() override
    {
        return std::make_unique<FakeLockFile>();
    }

    void createEnclosingEntity() override
    {
        // nothing to do
    }

private:
    std::shared_ptr<QIODevice> m_device;
};
