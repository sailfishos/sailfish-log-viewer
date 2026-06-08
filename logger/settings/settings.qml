// SPDX-FileCopyrightText: 2016-2026 Slava Monich <slava@monich.com>
// SPDX-FileCopyrightText: 2016-2021 Jolla Ltd.
// SPDX-License-Identifier: BSD-3-Clause

import QtQuick 2.0
import Sailfish.Silica 1.0
import org.nemomobile.configuration 1.0

Page {
    readonly property string rootPath: "/apps/" + appName() + "/"
    property alias title: pageHeader.title
    property bool inApp

    // jolla-settings expects these properties:
    property var applicationName
    property var applicationIcon

    // Deduce package name from the path
    function appName() {
        var parts = Qt.resolvedUrl("dummy").split('/')
        if (parts.length > 2) {
            var name = parts[parts.length-3]
            if (name.indexOf("-log") >= 0) {
                return name
            }
        }
        return "sailfish-log-viewer"
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height

        Column {
            id: content
            width: parent.width

            PageHeader {
                id: pageHeader

                rightMargin: Theme.horizontalPageMargin + (appIcon.visible ? (height - appIcon.padding) : 0)
                title: applicationName ? applicationName : qsTrId(appName() + "-app_name")
                description: applicationName ?
                    //: Settings page header description (app version)
                    //% "Version %1"
                    qsTrId("logger-settings-page-header-version").arg("1.0.22") :
                    ""

                Image {
                    id: appIcon
                    readonly property int padding: Theme.paddingLarge
                    readonly property int size: pageHeader.height - 2 * padding
                    x: pageHeader.width - width - Theme.horizontalPageMargin
                    y: padding
                    width: size
                    height: size
                    sourceSize: Qt.size(size,size)
                    source: applicationIcon ? applicationIcon : ""
                    visible: appIcon.status === Image.Ready
                }
            }

            SectionHeader {
                //: Section header
                //% "Display"
                text: qsTrId("logger-settings-section-header-display")
            }

            ComboBox {
                id: logSizeLimitComboBox
                //% "Screen buffer size"
                label: qsTrId("logger-settings-logsizelimit")
                //% "Don't worry, everything will be written to the log file regardless of the screen buffer size."
                description: qsTrId("logger-settings-logsizelimit-description")
                value: currentItem ? currentItem.text : ""
                menu: logSizeLimitMenu
                ContextMenu {
                    id: logSizeLimitMenu
                    readonly property int defaultIndex: 1
                    MenuItem {
                        text: maxLines
                        readonly property int maxLines: 100
                    }
                    MenuItem {
                        text: maxLines
                        readonly property int maxLines: 1000
                    }
                    MenuItem {
                        text: maxLines
                        readonly property int maxLines: 10000
                    }
                    MenuItem {
                        //% "Unlimited"
                        text: qsTrId("logger-settings-logsizelimit-unlimited")
                        readonly property int maxLines: 0
                    }
                }
                onCurrentIndexChanged: if (currentItem) logSizeLimit.value = currentItem.maxLines
                Component.onCompleted: updateSelection(logSizeLimit.value)
                function updateSelection(value) {
                    if (value === undefined) {
                        currentIndex = logSizeLimitMenu.defaultIndex
                    } else {
                        var n = logSizeLimitMenu.children.length
                        for (var i=0; i<n; i++) {
                            if (value === logSizeLimitMenu.children[i].maxLines) {
                                currentIndex = i
                                return
                            }
                        }
                        currentItem = null // Non-standard value
                    }
                }

                ConfigurationValue {
                    id: logSizeLimit
                    key: rootPath + "logSizeLimit"
                    onValueChanged: logSizeLimitComboBox.updateSelection(value)
                    defaultValue: 1000
                }
            }

            Slider {
                id: fontSizeSlider
                width: parent.width
                minimumValue: 0
                maximumValue: Theme.fontSizeLarge - Theme.fontSizeTiny
                stepSize: 1
                //% "Font size"
                label: qsTrId("logger-settings-fontsize-label")
                valueText: Theme.fontSizeTiny + sliderValue
                onSliderValueChanged: fontSizeAdjustment.value = sliderValue
                Component.onCompleted: value = fontSizeAdjustment.value

                ConfigurationValue {
                    id: fontSizeAdjustment
                    key: rootPath + "fontSizeAdjustment"
                    onValueChanged: fontSizeSlider.value = value
                    defaultValue: 0
                }
            }

            SectionHeader {
                //: Section header
                //% "Logging"
                text: qsTrId("logger-settings-section-header-logging")
            }

            TextSwitch {
                id: autoEnableLoggingSwitch
                automaticCheck: false
                //: Text switch label
                //% "Automatically enable logging"
                text: qsTrId("logger-settings-autoenable_logging-switch-label")
                //: Text switch description
                //% "Enable all logging categories when the application starts."
                description: qsTrId("logger-settings-autoenable_logging-switch-description")
                onClicked: autoEnableLogging.value = checked ? autoEnableLogging.autoEnableNone : autoEnableLogging.autoEnableAll
                Component.onCompleted: updateCheck()
                function updateCheck() { checked = (autoEnableLogging.value !== autoEnableLogging.autoEnableNone)  }

                ConfigurationValue {
                    id: autoEnableLogging
                    key: rootPath + "autoEnableLogging"
                    readonly property int autoEnableNone: 0
                    readonly property int autoEnableAll: 1
                    onValueChanged: autoEnableLoggingSwitch.updateCheck()
                    defaultValue: autoEnableAll
                }
            }

            TextSwitch {
                id: autoResetLoggingSwitch
                automaticCheck: false
                //: Text switch label
                //% "Automatically reset logging on exit"
                text: qsTrId("logger-settings-autoreset_logging-switch-label")
                //: Text switch description
                //% "Reset all logging categories to their default values when the application is exiting. Otherwise logging would continue in the background, wasting the system resources and eating up your battery."
                description: qsTrId("logger-settings-autoreset_logging-switch-description")
                onClicked: autoResetLogging.value = !checked
                Component.onCompleted: checked = autoResetLogging.value

                ConfigurationValue {
                    id: autoResetLogging
                    key: rootPath + "autoResetLogging"
                    onValueChanged: autoResetLoggingSwitch.checked = value
                    defaultValue: true
                }
            }
        }
    }
}
