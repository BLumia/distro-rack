// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs

ApplicationWindow {
    id: window
    width: 800
    height: 600
    visible: true
    title: qsTr("DistroRack")

    header: ToolBar {
        RowLayout {
            anchors.fill: parent

            ToolButton {
                icon.name: "list-add"
                text: qsTr("Create Container")
                onClicked: {
                    createContainerDialog.open()
                }
            }

            Item {
                Layout.fillWidth: true
            }

            ToolButton {
                id: menuButton
                text: qsTr("Menu")
                icon.name: "open-menu-symbolic"
                onClicked: menu.open()

                Menu {
                    id: menu

                    MenuItem {
                        text: qsTr("Refresh")
                        icon.name: "view-refresh"
                        onTriggered: {
                            stateManager.distroboxManager.listContainers()
                        }
                    }

                    MenuSeparator {}

                    MenuItem {
                        text: qsTr("Upgrade All")
                        icon.name: "system-software-update"
                        onTriggered: {
                            upgradeAllDialog.open()
                        }
                    }

                    MenuItem {
                        text: qsTr("Stop All")
                        icon.name: "process-stop"
                        onTriggered: {
                            stopAllDialog.open()
                        }
                    }



                    MenuSeparator {}

                    MenuItem {
                        text: qsTr("Command Log")
                        icon.name: "view-list-symbolic"
                        onTriggered: {
                            taskManagerDialog.open()
                        }
                    }

                    MenuItem {
                        text: qsTr("Generate All Entries")
                        icon.name: "application-x-desktop"
                        onTriggered: {
                            stateManager.distroboxManager.generateAllEntries();
                        }
                    }

                    MenuItem {
                        text: qsTr("Settings")
                        icon.name: "preferences-system"
                        onTriggered: {
                            settingsDialog.open()
                        }
                    }
                }
            }
        }
    }

    // Main content area with sidebar and details view
    SplitView {
        anchors.fill: parent

        // Sidebar for container list
        ContainerList {
            id: containerList
            SplitView.preferredWidth: 250
            SplitView.minimumWidth: 200

            onContainerSelected: function(containerName) {
                containerDetails.updateContainer(containerName)
            }
        }

        // Main content area for container details
        ContainerDetails {
            id: containerDetails
            SplitView.fillWidth: true
        }
    }

    // Dialog for creating new containers
    CreateContainerDialog {
        id: createContainerDialog
    }


    // Task manager dialog
    TaskManagerDialog {
        id: taskManagerDialog
    }

    // Settings dialog
    SettingsDialog {
        id: settingsDialog
        terminalManager: stateManager.terminalManager
    }

    // Initial state when no containers exist
    NoContainersView {
        id: noContainersView
        anchors.fill: parent
        visible: containerList.containerCount === 0
    }

    // 连接StateManager信号
    Connections {
        target: stateManager

        function onTaskManagerRequested() {
            taskManagerDialog.open()
        }

        function onExportableAppsRequested(containerName) {
            console.log("Exportable apps requested for: " + containerName)
            containerDetails.showExportableApps(containerName)
        }

        function onCreateContainerRequested() {
            createContainerDialog.open()
        }
    }

    Connections {
        target: stateManager.distroboxManager

        function onContainerCreated() {
            console.log("Container created successfully")
        }

        function onContainerDeleted() {
            console.log("Container deleted successfully")
        }

        function onCommandError(error) {
            console.log("Command error: " + error)
        }
    }

    // 升级所有容器的确认对话框
    Dialog {
        id: upgradeAllDialog
        title: qsTr("Upgrade All Containers")
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        anchors.centerIn: Overlay.overlay

        width: 400
        height: 150

        Text {
            text: qsTr("Are you sure you want to upgrade all containers?\n\nThis operation may take a very long time and will update all packages in every container.")
            wrapMode: Text.WordWrap
            width: parent.width
            color: palette.windowText
        }

        onAccepted: {
            stateManager.distroboxManager.upgradeAllContainers()
        }
    }

    // 停止所有容器的确认对话框
    Dialog {
        id: stopAllDialog
        title: qsTr("Stop All Containers")
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        anchors.centerIn: Overlay.overlay

        width: 400
        height: 150

        Text {
            text: qsTr("Are you sure you want to stop all running containers?\n\nThis will shut down all containers but will not delete them.")
            wrapMode: Text.WordWrap
            width: parent.width
            color: palette.windowText
        }

        onAccepted: {
            stateManager.distroboxManager.stopAllContainers()
        }
    }


}
