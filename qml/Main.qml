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
                            // Open settings dialog
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

        onAccepted: {
            // 当用户点击OK时，调用创建容器函数
            if (createContainerDialog.nameField.text && createContainerDialog.imageCombo.currentText) {
                // 收集卷信息
                var volumes = []
                // TODO: 从卷列表中收集卷信息

                stateManager.distroboxManager.createContainer(
                    createContainerDialog.nameField.text,
                    createContainerDialog.imageCombo.currentText,
                    createContainerDialog.nvidiaCheckBox.checked,
                    createContainerDialog.initCheckBox.checked,
                    createContainerDialog.homeDirField.text,
                    volumes
                )
            }
        }
    }


    // Task manager dialog
    TaskManagerDialog {
        id: taskManagerDialog
    }

    // Initial state when no containers exist
    Rectangle {
        id: noContainersView
        anchors.fill: parent
        visible: containerList.count === 0
        color: window.palette.window

        ColumnLayout {
            anchors.centerIn: parent

            Label {
                text: qsTr("No Containers")
                font.pointSize: 24
                Layout.alignment: Qt.AlignHCenter
            }

            Label {
                text: qsTr("Get started by creating a new container")
                Layout.alignment: Qt.AlignHCenter
            }

            Button {
                text: qsTr("Create Container")
                icon.name: "list-add"
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    createContainerDialog.open()
                }
            }
        }
    }

    // 连接StateManager信号
    Connections {
        target: stateManager

        function onTaskManagerRequested() {
            taskManagerDialog.open()
        }

        function onExportableAppsRequested(containerName) {
            console.log("Exportable apps requested for: " + containerName)
            // TODO: 打开 ExportableAppsDialog
        }

        function onCreateContainerRequested() {
            createContainerDialog.open()
        }
    }

    // 连接DistroboxManager信号
    Connections {
        target: stateManager.distroboxManager

        function onContainerCreated() {
            // 容器创建成功后显示提示
            console.log("Container created successfully")
        }

        function onContainerDeleted() {
            // 容器删除成功后显示提示
            console.log("Container deleted successfully")
        }

        function onCommandError(error) {
            // 显示错误信息
            console.log("Command error: " + error)
        }
    }

    // 升级所有容器的确认对话框
    Dialog {
        id: upgradeAllDialog
        title: qsTr("Upgrade All Containers")
        modal: true
        standardButtons: Dialog.Yes | Dialog.No

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
