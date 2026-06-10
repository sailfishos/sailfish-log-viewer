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

#ifndef LOGGER_LOG_SAVER_H
#define LOGGER_LOG_SAVER_H

#include "LoggerBuffer.h"

#include <QtCore/QObject>

class LoggerLogSaver :
    public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool packing READ isPacking NOTIFY packingChanged)
    Q_PROPERTY(bool saving READ isSaving NOTIFY savingChanged)
    Q_PROPERTY(QString archivePath READ archivePath NOTIFY archivePathChanged)
    Q_PROPERTY(QString archiveFile READ archiveFile CONSTANT)
    Q_PROPERTY(QString archiveType READ archiveType CONSTANT)

public:
    LoggerLogSaver(QString, QString, QObject* aParent = NULL);

    bool isPacking() const;
    bool isSaving() const;
    QString archivePath() const;
    QString archiveFile() const;
    QString archiveType() const;
    QString dirName() const;

public Q_SLOTS:
    void pack();
    void save();
    void addEntry(LoggerEntry);
    void onProcessDied(int, int);

Q_SIGNALS:
    void saveFinished(bool success);
    void packingChanged();
    void savingChanged();
    void archivePathChanged();

private:
    class WriteTask;
    class SaveTask;
    class Private;
    Private* iPrivate;
};

#endif // LOGGER_LOG_SAVER_H
