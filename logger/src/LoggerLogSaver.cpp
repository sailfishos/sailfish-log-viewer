/*
 * Copyright (C) 2016-2026 Slava Monich <slava@monich.com>
 * Copyright (C) 2016-2021 Jolla Ltd.
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer
 *     in the documentation and/or other materials provided with the
 *     distribution.
 *
 *  3. Neither the names of the copyright holders nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * any official policies, either expressed or implied.
 */

#include "LoggerLogSaver.h"
#include "LoggerCategory.h"
#include "HarbourDebug.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QMutex>
#include <QtCore/QStandardPaths>
#include <QtCore/QTemporaryDir>
#include <QtCore/QThreadPool>

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

// ==========================================================================
// LoggerLogSaver::Private declaration
// ==========================================================================

class LoggerLogSaver::Private :
    public QObject
{
    Q_OBJECT

public:
    Private(const QString&, const QString&, LoggerLogSaver*);
    ~Private();

    LoggerLogSaver* parentObject() const;
    void addEntry(const LoggerEntry&);
    void doWrite();
    void pack();
    bool save();

public Q_SLOTS:
    void onSaveTaskDone(bool);

public:
    bool iWriteTaskRunning;
    QMutex iMutex;
    LoggerBuffer iBuffer;
    QThreadPool* iThreadPool;
    QTemporaryDir iTempDir;
    const QString iArchiveType;
    const QString iArchiveName;
    const QString iArchiveFile;
    const QString iRootDir;
    QFile iLogFile;
    const QString iArchivePath;
    guint iArchiveSize;
    SaveTask* iSaveTask;
    int iPid;
};

// ==========================================================================
// LoggerLogSaver::WriteTask
// ==========================================================================

class LoggerLogSaver::WriteTask :
    public QRunnable
{
public:
    WriteTask(Private*);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    Private* iOwner;
};

LoggerLogSaver::WriteTask::WriteTask(
    Private* aOwner) :
    iOwner(aOwner)
{
    setAutoDelete(true);
}

void
LoggerLogSaver::WriteTask::run()
{
    iOwner->doWrite();
}

// ==========================================================================
// LoggerLogSaver::SaveTask
// ==========================================================================

class LoggerLogSaver::SaveTask :
    public QObject,
    public QRunnable
{
    Q_OBJECT

public:
    SaveTask(QString aSrc, QString aDest, QObject* aParent = Q_NULLPTR) :
        QObject(aParent), iSrc(aSrc), iDest(aDest) {
        setAutoDelete(false);
    }

protected:
    void run() Q_DECL_OVERRIDE;

Q_SIGNALS:
    void done(bool aSuccess);

private:
    QString iSrc;
    QString iDest;
};

void
LoggerLogSaver::SaveTask::run()
{
    QFile::remove(iDest);

    bool ok = QFile::copy(iSrc, iDest);
    if (ok) {
        HDEBUG("Copied" << qPrintable(iSrc) << "->" << qPrintable(iDest));
    } else {
        HDEBUG("Failed to copy" << qPrintable(iSrc) << "->" << qPrintable(iDest));
    }
    Q_EMIT done(ok);
}

// ==========================================================================
// LoggerLogSaver::Private implementation
// ==========================================================================

LoggerLogSaver::Private::Private(
    const QString& aName,
    const QString& aTmpDir,
    LoggerLogSaver* aParent) :
    QObject(aParent),
    iWriteTaskRunning(false),
    iBuffer(-1),
    iThreadPool(new QThreadPool(this)),
    iTempDir(aTmpDir + QDir::separator() + aName + "_XXXXXX"),
    iArchiveType("application/x-gzip"),
    iArchiveName(aName + QDateTime::currentDateTime().toString("_yyyy-MM-dd_hhmmss")),
    iArchiveFile(iArchiveName + ".tar.gz"),
    iRootDir(iTempDir.path() + QDir::separator() + iArchiveName),
    iLogFile(iRootDir + QDir::separator() + aName + ".log"),
    iArchivePath(iTempDir.path() + QDir::separator() + iArchiveFile),
    iArchiveSize(0),
    iSaveTask(Q_NULLPTR),
    iPid(0)
{
    iThreadPool->setMaxThreadCount(1);
    iTempDir.setAutoRemove(true);
    HDEBUG("Temporary directory" << iTempDir.path());
    if (!QDir(iRootDir).mkpath(iRootDir)) {
        HDEBUG("Failed to create " << qPrintable(iRootDir));
    }
    if (!iLogFile.open(QFile::Text | QFile::ReadWrite)) {
        HDEBUG("Failed to open " << qPrintable(iLogFile.fileName()));
    }
}

LoggerLogSaver::Private::~Private()
{
    iThreadPool->waitForDone();
    HASSERT(!iWriteTaskRunning);
    QFile::remove(iArchivePath);
}

inline
LoggerLogSaver*
LoggerLogSaver::Private::parentObject() const
{
    return qobject_cast<LoggerLogSaver*>(parent());
}

// Invoked on the UI thread
void
LoggerLogSaver::Private::addEntry(
    const LoggerEntry& aEntry)
{
    iMutex.lock();
    iBuffer.put(aEntry);
    if (!iWriteTaskRunning) {
        iWriteTaskRunning = true;
        iThreadPool->start(new WriteTask(this));
    }
    iMutex.unlock();
}

// Invoked on the worker thread
void
LoggerLogSaver::Private::doWrite()
{
    iMutex.lock();
    HASSERT(iWriteTaskRunning);
    while (!iBuffer.isEmpty()) {
        LoggerEntry entry(iBuffer.get());
        iMutex.unlock();
        // Actually save it
        LoggerEntry::Type type(entry.type());
        if (type == LoggerEntry::TypeLog) {
            LoggerCategory category(entry.category());
            QString categoryName(category.name());
            QString timestamp(entry.time().toString("yyyy-MM-dd hh:mm:ss.zzz "));
            QString line;
            if (category.isValid() &&
                !category.name().isEmpty() &&
                !category.hidden()) {
                line = timestamp + categoryName + ": " + entry.text();
            } else {
                line = timestamp + entry.text();
            }
            iLogFile.write(line.toUtf8());
            iLogFile.write("\n");
        } else if (type == LoggerEntry::TypeSkip) {
            iLogFile.write(QString("... skipped %1 entries\n").
                arg(entry.skipCount()).toUtf8());
        }
        iMutex.lock();
    }
    iWriteTaskRunning = false;
    iMutex.unlock();
    iLogFile.flush();
}

void
LoggerLogSaver::Private::pack()
{
    QFile::remove(iArchivePath);
    HDEBUG("Creating" << qPrintable(iArchivePath));
    if (iPid > 0) kill(iPid, SIGKILL);
    if (!(iPid = fork())) {
        // Child
        execlp("tar", "tar", "-czf", qPrintable(iArchivePath), "-C",
            qPrintable(iTempDir.path()), qPrintable(iArchiveName), Q_NULLPTR);
        abort();
    }
}

bool
LoggerLogSaver::Private::save()
{
    HDEBUG(qPrintable(iArchivePath));
    if (!iArchivePath.isEmpty() && !iSaveTask) {
        QString fileName = QFileInfo(iArchivePath).fileName();
        QString destPath = QDir(QStandardPaths::writableLocation
            (QStandardPaths::DocumentsLocation)).absoluteFilePath(fileName);
        iSaveTask = new SaveTask(iArchivePath, destPath, this);
        connect(iSaveTask, SIGNAL(done(bool)), SLOT(onSaveTaskDone(bool)),
            Qt::QueuedConnection);
        HDEBUG(">" << qPrintable(destPath));
        iThreadPool->start(iSaveTask);
        return true;
    } else {
        return false;
    }
}

void
LoggerLogSaver::Private::onSaveTaskDone(
    bool aSuccess)
{
    HDEBUG((aSuccess ? "OK" : "ERROR"));
    if (iSaveTask) {
        LoggerLogSaver* saver = parentObject();

        delete iSaveTask;
        iSaveTask = Q_NULLPTR;
        Q_EMIT saver->saveFinished(aSuccess);
        Q_EMIT saver->savingChanged();
    }
}

// ==========================================================================
// LoggerLogSaver
// ==========================================================================

LoggerLogSaver::LoggerLogSaver(
    QString aName,
    QString aTmpDir,
    QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(aName, aTmpDir, this))
{}

QString
LoggerLogSaver::dirName() const
{
    return iPrivate->iRootDir;
}


bool
LoggerLogSaver::isPacking() const
{
    return iPrivate->iPid > 0;
}

bool
LoggerLogSaver::isSaving() const
{
    return iPrivate->iSaveTask != Q_NULLPTR;
}

QString
LoggerLogSaver::archivePath() const
{
    return iPrivate->iArchivePath;
}

QString
LoggerLogSaver::archiveFile() const
{
    return iPrivate->iArchiveFile;
}

QString
LoggerLogSaver::archiveType() const
{
    return iPrivate->iArchiveType;
}

qint64
LoggerLogSaver::archiveSize() const
{
    return iPrivate->iArchiveSize;
}

QUrl
LoggerLogSaver::archiveUrl() const
{
    return QUrl::fromLocalFile(iPrivate->iArchivePath);
}

void
LoggerLogSaver::pack()
{
    const bool wasPacking = isPacking();

    iPrivate->pack();
    if (wasPacking != isPacking()) {
        Q_EMIT packingChanged();
    }
    Q_EMIT archivePathChanged();
}

void
LoggerLogSaver::onProcessDied(
    int aPid,
    int aStatus)
{
    if (iPrivate->iPid > 0 && iPrivate->iPid == aPid) {
        const QByteArray tarball(iPrivate->iArchivePath.toLocal8Bit());
        const qint64 size = QFileInfo(iPrivate->iArchivePath).size();

        HDEBUG("Tar done, pid" << aPid << "status" <<
            aStatus << "size" << size);
        if (chown(tarball.constData(), getuid(), getgid()) < 0) {
            HWARN("Failed to chown" << tarball.constData() << ":"
                << strerror(errno));
        }
        iPrivate->iPid = -1;
        if (iPrivate->iArchiveSize != size) {
            iPrivate->iArchiveSize = size;
            Q_EMIT archiveSizeChanged();
        }
        Q_EMIT packingChanged();
    }
}

void
LoggerLogSaver::save()
{
    if (iPrivate->save()) {
        Q_EMIT savingChanged();
    }
}

void
LoggerLogSaver::addEntry(
    LoggerEntry aEntry)
{
    iPrivate->addEntry(aEntry);
}

#include "LoggerLogSaver.moc"
