/*
 * Copyright (C) 2019-2026 Slava Monich <slava@monich.com>
 * Copyright (C) 2019-2022 Jolla Ltd.
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

#include <sailfishapp.h>

#include <QtQuick/QQuickView>

#ifdef APP_PREFIX
#  define APP_NAME_PREFIX QT_STRINGIFY(APP_PREFIX)
#  pragma message("App prefix: " APP_NAME_PREFIX)
#else
#  define APP_NAME_PREFIX "sailfish-log-viewer"
#endif

class NfcLogger :
    public LoggerMain
{
    Q_OBJECT

public:
    NfcLogger(int*, char**, QStringList);

protected:
#ifdef APP_TRANSLATIONS_PATH
#  define APP_TRANS_DIR QT_STRINGIFY(APP_TRANSLATIONS_PATH)
#  pragma message("Translations dir: " APP_TRANS_DIR)
    QString translationPath() Q_DECL_OVERRIDE;
#endif
    void setupView(QQuickView*) Q_DECL_OVERRIDE;
};

NfcLogger::NfcLogger(int* aArgc, char** aArgv, QStringList aPackages) :
    LoggerMain(aArgc, aArgv, "org.sailfishos.nfc.daemon", aPackages,
    APP_NAME_PREFIX, "nfc", "qml/main.qml")
{}

#ifdef APP_TRANSLATIONS_PATH
QString
NfcLogger::translationPath()
{
    return APP_TRANS_DIR;
}
#endif

void
NfcLogger::setupView(
    QQuickView* aView)
{
    //: Settings page title (app name)
    //% "NFC Logger"
    aView->setTitle(qtTrId("openrepos-logger-nfc-app_name"));
    LoggerMain::setupView(aView);
}

Q_DECL_EXPORT int main(int argc, char* argv[])
{
    QStringList packages;

    packages.append("libgbinder");
    packages.append("libncicore");
    packages.append("libnciplugin");
    packages.append("libglibutil");
    packages.append("nfcd");
    packages.append("nfcd-binder-plugin");
    packages.append("pn54x-binder-plugin");
    return NfcLogger(&argc, argv, packages).run();
}

#include "main.moc"
