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

#include "LoggerMain.h"
#include "LoggerHints.h"
#include "LoggerLogModel.h"
#include "LoggerLogSaver.h"
#include "LoggerSettings.h"
#include "LoggerCategoryModel.h"
#include "LoggerCategoryFilterModel.h"

#include "HarbourDebug.h"
#include "HarbourProcessState.h"
#include "HarbourSystemInfo.h"
#include "HarbourSigChildHandler.h"
#include "HarbourTransferMethodsModel.h"

#include <sailfishapp.h>
#include <QtCore/QFile>
#include <QtQuick>
#include <QtQml>

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/fsuid.h>

#undef signals
#include "dbuslog_client.h"

LoggerMain::LoggerMain(
    int* aArgc,
    char** aArgv,
    const char* aService,
    QStringList aRpmPackages,
    QString aAppPrefix,
    QString aAppSuffix,
    QString aQmlPath) :
    iService(aService),
    iRpmPackages(aRpmPackages),
    iFullAppName(aAppPrefix + QString("-") + aAppSuffix),
    iAppSuffix(aAppSuffix),
    iQmlPath(aQmlPath),
    iApp(SailfishApp::application(*aArgc, aArgv)),
    iClient(dbus_log_client_new(G_BUS_TYPE_SYSTEM, aService, "/",
        DBUSLOG_CLIENT_FLAG_AUTOSTART))
{
    QByteArray plugin((QString("harbour.logger.") + aAppSuffix).toLatin1());
    qmlRegisterType<LoggerHints>(plugin.constData(), 1, 0, "LoggerHints");
    HarbourTransferMethodInfo2::registerTypes();
}

LoggerMain::~LoggerMain()
{
    dbus_log_client_unref(iClient);
    delete iApp;
}

/* static */
bool
LoggerMain::saveOutput(
    const char* aExe,
    const char* const aArgv[],
    QString aOut)
{
    int fd = open(qPrintable(aOut), O_WRONLY | O_CREAT, 0644);

    if (fd >= 0) {
        pid_t pid = fork();

        if (!pid) {
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            execvp(aExe, (char**)aArgv);
            abort();
        }
        close(fd);
        HDEBUG(pid << qPrintable(aOut));
        return true;
    } else {
        HWARN(qPrintable(aOut) << strerror(errno));
        return false;
    }
}

/* static */
bool
LoggerMain::saveOutput(
    const char* const aArgv[],
    QString aOut)
{
    return saveOutput(aArgv[0], aArgv, aOut);
}

/* static */
bool
LoggerMain::saveOutput(
    const char* aExe,
    const char* aArg1,
    const char* aArg2,
    QString aOut)
{
    const char* argv[4];

    argv[0] = aExe;
    argv[1] = aArg1;
    argv[2] = aArg2;
    argv[3] = NULL;
    return saveOutput(aExe, argv, aOut);
}

/* static */
bool
LoggerMain::saveOutput(
    const char* aExe,
    const char* aArg1,
    const char* aArg2,
    const char* aArg3,
    const char* aArg4,
    const char* aArg5,
    const char* aArg6,
    QString aOut)
{
    const char* argv[8];

    argv[0] = aExe;
    argv[1] = aArg1;
    argv[2] = aArg2;
    argv[3] = aArg3;
    argv[4] = aArg4;
    argv[5] = aArg5;
    argv[6] = aArg6;
    argv[7] = NULL;
    return saveOutput(aExe, argv, aOut);
}

QString
LoggerMain::translationPath()
{
    return SailfishApp::pathTo("translations").toLocalFile();
}

void LoggerMain::saveFilesAtStartup(QString aDir)
{
    // Copy OS and hardware versions
    QFile::copy("/etc/sailfish-release", aDir + "/sailfish-release");
    QFile::copy("/etc/hw-release", aDir + "/hw-release");

    // And the package version(s)
    const int n = iRpmPackages.count();

    if (n > 0) {
        QByteArray printablePackages[n];
        const char* args[n + 3];

        args[0] = "rpm";
        args[1] = "-q";
        for (int i = 0; i < n; i++) {
            printablePackages[i] = iRpmPackages.at(i).toLocal8Bit();
            args[i + 2] = printablePackages[i].constData();
        }
        args[n + 2] = NULL;
        saveOutput(args, aDir + "/" + iAppSuffix + "-packages");
    }
}

void
LoggerMain::setupView(
    QQuickView* aView)
{}

int
LoggerMain::run()
{
    // Translations
    QLocale locale;
    QTranslator* translator = new QTranslator(iApp);
    const QString transDir(translationPath());

    if (translator->load(locale, iFullAppName, "-", transDir) ||
        translator->load(iFullAppName, transDir)) {
        iApp->installTranslator(translator);
    } else {
        HWARN("Failed to load" << qPrintable(iFullAppName) << "translations for" << locale);
        HDEBUG("Translation directory" << transDir);
        HDEBUG("App name" << iFullAppName);
        delete translator;
    }

    translator = new QTranslator(iApp);
    if (HarbourTransferMethodsModel::loadTranslations(translator, locale) ||
        HarbourTransferMethodsModel::loadTranslations(translator, QLocale("en_GB"))) {
        iApp->installTranslator(translator);
    } else {
        delete translator;
    }

    // Signal handler
    HarbourSigChildHandler* sigChild = HarbourSigChildHandler::install(iApp);

    // Models and stuff
    LoggerSettings* logSettings = new LoggerSettings(iFullAppName, iApp);
    LoggerLogModel* logModel = new LoggerLogModel(logSettings, iClient, iApp);
    LoggerCategoryModel* categoryModel = new LoggerCategoryModel(logSettings, iClient, iApp);
    LoggerCategoryFilterModel* filterModel = new LoggerCategoryFilterModel(categoryModel);
    LoggerLogSaver* logSaver = new LoggerLogSaver(iAppSuffix, logSettings->tmpDir(), iApp);
    HarbourTransferMethodsModel* transferModel = new HarbourTransferMethodsModel(iApp);
    transferModel->setFilter(logSaver->archiveType());
    logSaver->connect(logModel, SIGNAL(entryAdded(LoggerEntry)), SLOT(addEntry(LoggerEntry)));
    logSaver->connect(sigChild, SIGNAL(processDied(int,int)), SLOT(onProcessDied(int,int)));

    // Save some files
    saveFilesAtStartup(logSaver->dirName());

    // Create and show the view
    QQuickView* view = SailfishApp::createView();
    QQmlContext* context = view->rootContext();
    context->setContextProperty("LogSettings", logSettings);
    context->setContextProperty("LogModel", logModel);
    context->setContextProperty("LogSaver", logSaver);
    context->setContextProperty("CategoryModel", categoryModel);
    context->setContextProperty("CategoryFilterModel", filterModel);
    context->setContextProperty("TransferMethodsModel", transferModel);
    context->setContextProperty("ProcessState", new HarbourProcessState(iApp));
    context->setContextProperty("SystemInfo", new HarbourSystemInfo(iApp));
    context->setContextProperty("AppName", iFullAppName);

    setupView(view);
    view->setSource(SailfishApp::pathTo(iQmlPath));
    view->showFullScreen();

    int ret = iApp->exec();

    if (logSettings->autoResetLogging()) {
        categoryModel->restoreLogLevel();
        categoryModel->reset();
    }

    delete view;
    return ret;
}
