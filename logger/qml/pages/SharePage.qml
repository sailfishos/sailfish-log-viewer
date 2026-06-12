// SPDX-FileCopyrightText: 2016-2026 Slava Monich <slava@monich.com>
// SPDX-FileCopyrightText: 2016-2021 Jolla Ltd.
// SPDX-License-Identifier: BSD-3-Clause

import QtQuick 2.0
import Sailfish.Silica 1.0
import org.nemomobile.notifications 1.0

import "../harbour"

Page {
    id: page

    allowedOrientations: window.allowedOrientations

    property var logSaver: LogSaver
    property var shareModel: TransferMethodsModel // Context property
    readonly property string _sharingApiVersion: SystemInfo.packageVersion("declarative-transferengine-qt5")
    readonly property bool _sailfishShare: !_sharingApiVersion || SystemInfo.compareVersions(_sharingApiVersion, "0.4.0") >= 0 // QML API break
    readonly property bool _readyToShare: !logSaver.packing && !logSaver.saving && !minWaitTimer.running

    // For the page slide animation to kick in, the initial value of
    // backNavigation has to be true. Once the transition has started,
    // backNavigation is turned off until the log has been saved.
    showNavigationIndicator: status !== PageStatus.Inactive
    backNavigation: status === PageStatus.Inactive || _readyToShare

    // The timer makes sure that animation is displayed for at least 1 second
    Timer {
        id: minWaitTimer
        interval: 1000
        running: true
    }

    Notification {
        id: notification
    }

    Connections {
        target: logSaver
        onSaveFinished: {
            notification.close()
            if (success) {
                //% "Saved %1"
                notification.previewBody = qsTrId("logger-sharepage-save-ok").arg(logSaver.archiveFile)
            } else {
                //% "Failed to save %1"
                notification.previewBody = qsTrId("logger-sharepage-save-error").arg(logSaver.archiveFile)
            }
            notification.publish()
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: parent.height

        PullDownMenu {
            visible: !_sailfishShare && (_readyToShare || active)
            MenuItem {
                //% "Save to documents"
                text: qsTrId("logger-sharepage-pm-save-to-documents")
                onClicked: logSaver.save()
            }
            onActiveChanged: {
                if (!active && logSaver.saving) {
                    // Copying hasn't finished by the time menu was closed
                    minWaitTimer.start()
                }
            }
        }

        PageHeader {
            id: header
            //% "Pack and send"
            title: qsTrId("logger-sharepage-header")
        }

        Item {
            id: shareMethods
            anchors {
                top: header.bottom
                left: parent.left
            }
            width: parent.width
            visible: opacity > 0
            opacity: _readyToShare
            Behavior on opacity { FadeAnimation {} }

            Loader {
                active: _sailfishShare
                anchors.fill: parent
                sourceComponent: Component {
                    Item {
                        anchors.fill: parent

                        Image {
                            id: archiveIcon

                            x: Theme.horizontalPageMargin
                            source: "image://theme/icon-m-file-archive-folder?" + Theme.secondaryColor
                        }

                        Column {
                            id: fileInfo

                            anchors {
                                left: archiveIcon.right
                                leftMargin: Theme.paddingLarge
                                right: parent.right
                            }

                            Label {
                                width: parent.width
                                text: logSaver.archiveFile
                                truncationMode: TruncationMode.Fade
                                color: Theme.secondaryColor
                            }

                            Label {
                                readonly property int _kB: 1024
                                readonly property int _MB: _kB*1024
                                readonly property int _GB: _MB*1024
                                readonly property int _TB: _GB*1024
                                readonly property int fileSize: logSaver.archiveSize

                                width: parent.width
                                text: fileSize <= 0 ? "" :
                                    //% "%1 B"
                                    (fileSize < _kB) ? qsTrId("logger-sharepage-la-file_size-bytes").arg(fileSize) :
                                    //% "%1 kB"
                                    (fileSize < 1000*_kB) ? qsTrId("logger-sharepage-la-file_size-kilobytes").arg((fileSize/_kB).toLocaleString(Qt.locale(), 'f', (fileSize < 10*_kB) ? 2 : 1)) :
                                    //% "%1 MB"
                                    (fileSize < 1000*_MB) ? qsTrId("logger-sharepage-la-file_size-megabytes").arg((fileSize/_MB).toLocaleString(Qt.locale(), 'f', (fileSize < 10*_MB) ? 2 : 1)) :
                                    //% "%1 GB"
                                    (fileSize < 1000*_GB) ? qsTrId("logger-sharepage-la-file_size-gigabytes").arg((fileSize/_GB).toLocaleString(Qt.locale(), 'f', (fileSize < 10*_GB) ? 2 : 1)) :
                                    //% "%1 TB"
                                    qsTrId("logger-sharepage-la-file_size-terabytes").arg((fileSize/_TB).toLocaleString(Qt.locale(), 'f', 1))
                                font.pixelSize: Theme.fontSizeExtraSmall
                                color: Theme.secondaryColor
                                opacity: 0.8
                            }
                        }

                        Item {
                            width: parent.width
                            anchors {
                                top: fileInfo.bottom
                                bottom: parent.bottom
                            }

                            Column {
                                anchors.centerIn: parent
                                spacing: Theme.paddingLarge

                                Button {
                                    //% "Save to documents"
                                    text: qsTrId("logger-sharepage-pm-save-to-documents")
                                    onClicked: logSaver.save()
                                }
                                Button {
                                    property var shareAction

                                    //% "Share"
                                    text: qsTrId("logger-sharepage-bt-share")
                                    onClicked: {
                                        if (!shareAction) {
                                            shareAction = Qt.createQmlObject("import Sailfish.Share 1.0;ShareAction {}", page, "SailfishShare")
                                        }
                                        if (shareAction) {
                                            shareAction.resources = [ logSaver.archiveUrl ]
                                            shareAction.trigger()
                                        } else {
                                            enabled = false
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Loader {
                active: !_sailfishShare
                anchors.fill: parent
                sourceComponent: Component {
                    HarbourShareMethodList {
                        anchors.fill: parent
                        model: shareModel
                        source: logSaver.archivePath
                        type: logSaver.archiveType
                        //: Default email subject
                        //% "Log"
                        subject: qsTrId("logger-sharepage-default-subject")
                        //: Default email recipient
                        //% ""
                        emailTo: qsTrId("logger-sharepage-default-email")
                        //% "Add account"
                        addAccountText: qsTrId("logger-sharemethodlist-add-account")
                        VerticalScrollDecorator {}
                    }
                }
            }
        }

        Label {
            id: warning

            visible: opacity > 0
            opacity: _readyToShare
            height: implicitHeight
            Behavior on opacity { FadeAnimation {} }
            wrapMode: Text.WordWrap
            font.pixelSize: Theme.fontSizeExtraSmall
            verticalAlignment: Text.AlignTop
            color: Theme.secondaryColor
            //% "Keep in mind that some of the information contained in this archive may be considered private. If you would like to check what you are about to send, please consider sending it to yourself and checking its contents first."
            text: qsTrId("logger-sharepage-warning")
        }

        states: [
            State {
                name: "PORTRAIT"
                when: page.orientation === Orientation.Portrait
                AnchorChanges {
                    target: shareMethods
                    anchors {
                        right: parent.right
                        bottom: warning.top
                    }
                }
                PropertyChanges {
                    target: shareMethods
                    anchors.bottomMargin: Theme.paddingLarge
                    anchors.rightMargin: 0
                }
                PropertyChanges {
                    target: warning
                    x: Theme.horizontalPageMargin
                    y: window.height - height - Theme.paddingLarge
                    width: parent.width - 2*Theme.horizontalPageMargin
                }
            },
            State {
                name: "LANDSCAPE"
                when: page.orientation !== Orientation.Portrait
                AnchorChanges {
                    target: shareMethods
                    anchors {
                        right: warning.left
                        bottom: parent.bottom
                    }
                }
                PropertyChanges {
                    target: shareMethods
                    anchors.bottomMargin: 0
                    anchors.rightMargin: Theme.horizontalPageMargin
                }
                PropertyChanges {
                    target: warning
                    x: parent.width - width - Theme.horizontalPageMargin
                    y: header.y + header.height
                    width: parent.width*2/5 - 2*Theme.horizontalPageMargin
                }
            }
        ]
    }

    Column {
        visible: opacity > 0
        opacity: _readyToShare ? 0 : 1
        anchors.centerIn: parent
        spacing: Theme.paddingLarge
        width: Math.max(busyIndicator.width, pleaseWaitLabel.width)
        Behavior on opacity { FadeAnimation {} }
        BusyIndicator {
            id: busyIndicator
            anchors.horizontalCenter: parent.horizontalCenter
            size: BusyIndicatorSize.Large
            running: !_readyToShare
        }
        Label {
            id: pleaseWaitLabel
            anchors.horizontalCenter: parent.horizontalCenter
            horizontalAlignment: Text.AlignHCenter
            color: Theme.highlightColor
            //% "Please wait"
            text: qsTrId("logger-sharepage-please-wait")
        }
    }
}
