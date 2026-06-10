/*
 * Copyright (C) 2015-2026 Slava Monich <slava@monich.com>
 * Copyright (C) 2015-2017 Jolla Ltd.
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

#include "LoggerSettings.h"
#include "HarbourDebug.h"

#include <MGConfItem>

#define DCONF_KEY(app,key)              QString("/apps/%1/%2").arg(app).arg(key)
#define DCONF_LOG_SIZE_LIMIT(app)       DCONF_KEY(app,"logSizeLimit")
#define DCONF_FONT_SIZE_ADJUSTMENT(app) DCONF_KEY(app,"fontSizeAdjustment")
#define DCONF_AUTO_ENABLE_LOGGING(app)  DCONF_KEY(app,"autoEnableLogging")
#define DCONF_AUTO_RESET_LOGGING(app)   DCONF_KEY(app,"autoResetLogging")
#define DCONF_TMP_DIR(app)              DCONF_KEY(app,"tmpDir")

//===========================================================================
// LoggerSettings::Private
//===========================================================================

class LoggerSettings::Private : public QObject
{
    Q_OBJECT
public:
    Private(const QString&, QObject*);

    AutoEnable autoEnableLogging() const;

public:
    static const int DEFAULT_LOG_SIZE_LIMIT = 1000;
    static const int DEFAULT_FONT_SIZE_ADJUSTMENT = 0;
    static const int DEFAULT_AUTO_ENABLE_LOGGING = AutoEnableAll;
    static const bool DEFAULT_AUTO_RESET_LOGGING = true;
    static const QString DEFAULT_TMP_DIR;

    MGConfItem* iLogSizeLimit;
    MGConfItem* iFontSizeAdjustment;
    MGConfItem* iAutoEnableLogging;
    MGConfItem* iAutoResetLogging;
    MGConfItem* iTmpDir;
};

const QString LoggerSettings::Private::DEFAULT_TMP_DIR("/tmp");

LoggerSettings::Private::Private(
    const QString& aApp,
    QObject* aParent) :
    QObject(aParent),
    iLogSizeLimit(new MGConfItem(DCONF_LOG_SIZE_LIMIT(aApp), this)),
    iFontSizeAdjustment(new MGConfItem(DCONF_FONT_SIZE_ADJUSTMENT(aApp), this)),
    iAutoEnableLogging(new MGConfItem(DCONF_AUTO_ENABLE_LOGGING(aApp), this)),
    iAutoResetLogging(new MGConfItem(DCONF_AUTO_RESET_LOGGING(aApp), this)),
    iTmpDir(new MGConfItem(DCONF_TMP_DIR(aApp), this))
{}

inline
LoggerSettings::AutoEnable
LoggerSettings::Private::autoEnableLogging() const
{
    int value = iAutoEnableLogging->value(DEFAULT_AUTO_ENABLE_LOGGING).toInt();

    switch(value) {
    case AutoEnableNone:
    case AutoEnableAll:
        return (AutoEnable)value;
    }
    return (AutoEnable)DEFAULT_AUTO_ENABLE_LOGGING;
}

//===========================================================================
// LoggerSettings::Private
//===========================================================================

LoggerSettings::LoggerSettings(
    QString aApp,
    QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(aApp, this))
{
    connect(iPrivate->iLogSizeLimit, SIGNAL(valueChanged()),
        this, SIGNAL(logSizeLimitChanged()));
    connect(iPrivate->iFontSizeAdjustment, SIGNAL(valueChanged()),
        this, SIGNAL(fontSizeAdjustmentChanged()));
    connect(iPrivate->iAutoEnableLogging, SIGNAL(valueChanged()),
        this, SIGNAL(autoEnableLoggingChanged()));
    connect(iPrivate->iAutoResetLogging, SIGNAL(valueChanged()),
        this, SIGNAL(autoResetLoggingChanged()));
}

int
LoggerSettings::defaultLogSizeLimit()
{
    return Private::DEFAULT_LOG_SIZE_LIMIT;
}

int
LoggerSettings::logSizeLimit() const
{
    return iPrivate->iLogSizeLimit->
        value(Private::DEFAULT_LOG_SIZE_LIMIT).toInt();
}

int
LoggerSettings::fontSizeAdjustment() const
{
    return iPrivate->iFontSizeAdjustment->
        value(Private::DEFAULT_FONT_SIZE_ADJUSTMENT).toInt();
}

LoggerSettings::AutoEnable
LoggerSettings::autoEnableLogging() const
{
    return iPrivate->autoEnableLogging();
}

bool
LoggerSettings::autoResetLogging() const
{
    return iPrivate->iAutoResetLogging->
        value(Private::DEFAULT_AUTO_RESET_LOGGING).toBool();
}

QString
LoggerSettings::tmpDir() const
{
    return iPrivate->iTmpDir->
        value(Private::DEFAULT_TMP_DIR).toString();
}

#include "LoggerSettings.moc"
