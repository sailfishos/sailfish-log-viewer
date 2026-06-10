/*
 * Copyright (C) 2016-2026 Slava Monich <slava@monich.com>
 * Copyright (C) 2016-2022 Jolla Ltd.
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

#ifndef LOGGER_MAIN_H
#define LOGGER_MAIN_H

#include <QtCore/QObject>
#include <QtCore/QStringList>

class QQuickView;
class QGuiApplication;

class LoggerMain :
    public QObject
{
    Q_OBJECT

public:
    LoggerMain(int* aArgc, char** aArgv, const char* aService,
        QStringList aRpmPackages, QString aAppPrefix, QString aAppSuffix,
        QString aQmlPath);
    virtual ~LoggerMain();

    int run();

    static bool saveOutput(const char* const aArgv[], QString);
    static bool saveOutput(const char*, const char* const aArgv[], QString);
    static bool saveOutput(const char*, const char*, const char*, QString);
    static bool saveOutput(const char*, const char*, const char*, const char*,
        const char*, const char*, const char*, QString);

protected:
    // These are invoked by run()
    virtual QString translationPath();
    virtual void saveFilesAtStartup(QString);
    virtual void setupView(QQuickView*);

protected:
    const QString iService;

private:
    const QStringList iRpmPackages;
    const QString iFullAppName;
    const QString iAppSuffix;
    const QString iQmlPath;
    QGuiApplication* iApp;
    struct dbus_log_client* iClient;
};

#endif // LOGGER_MAIN_H
