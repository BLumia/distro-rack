// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs

Dialog {
    id: settingsDialog
    title: qsTr("Settings")
    modal: true
    width: 600
    height: 500
    anchors.centerIn: Overlay.overlay

    property alias terminalManager: terminalTabContent.terminalManager

    standardButtons: Dialog.Close

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TabBar {
            id: tabBar
            Layout.fillWidth: true
            visible: false

            TabButton {
                text: qsTr("Terminal")
                width: implicitWidth
            }

            TabButton {
                text: qsTr("General")
                width: implicitWidth
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 10
            currentIndex: tabBar.currentIndex

            TerminalSettingsTab {
                id: terminalTabContent
                terminalManager: settingsDialog.terminalManager
            }

            // General Settings Tab (placeholder for future settings)
            Item {
                Label {
                    anchors.centerIn: parent
                    text: qsTr("General settings will be added here in the future")
                    color: palette.placeholderText
                }
            }
        }
    }
}
