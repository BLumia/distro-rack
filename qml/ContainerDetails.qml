// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import DistroRack 1.0

Item {
    id: containerDetails

    property string currentContainerName: ""
    property var containerModel: stateManager.containerModel

    // 获取当前容器的模型索引
    readonly property var currentContainerIndex: containerModel ? containerModel.getContainerModelIndex(currentContainerName) : null

    function updateContainer(containerName) {
        currentContainerName = containerName;
    }

    // 辅助函数：从模型获取容器数据
    function getContainerData(role) {
        if (!containerModel || !currentContainerIndex || !currentContainerIndex.valid) {
            return "";
        }
        return containerModel.data(currentContainerIndex, role);
    }

    // 容器属性的便捷访问器
    readonly property string containerName: getContainerData(ContainerModel.NameRole)
    readonly property string containerImage: getContainerData(ContainerModel.ImageRole)
    readonly property string containerStatus: getContainerData(ContainerModel.StatusRole)
    readonly property string containerDistro: getContainerData(ContainerModel.DistroRole)
    readonly property string containerDistroColor: getContainerData(ContainerModel.DistroColorRole)
    readonly property string containerDistroIconName: getContainerData(ContainerModel.DistroIconNameRole)
    readonly property string containerInstallableFileExtension: getContainerData(ContainerModel.InstallableFileExtensionRole)

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 24

            // Container Header
            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Rectangle {
                    width: 64
                    height: 64
                    color: "transparent"
                    radius: 8

                    ThemedIcon {
                        anchors.centerIn: parent
                        iconSourceSize: 48
                        iconName: containerDistroIconName || "distributor-logo-generic"
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Label {
                        text: containerName || "No Container Selected"
                        font.pixelSize: 24
                        font.bold: true
                    }

                    RowLayout {
                        spacing: 6

                        Label {
                            text: containerImage
                            font.pixelSize: 12
                        }

                        ToolButton {
                            text: qsTr("Copy")
                            icon.name: "edit-copy-symbolic"
                            flat: true
                            implicitHeight: 24
                            font.pixelSize: 10

                            onClicked: {
                                if (containerImage) {
                                    // Copy image URL to clipboard (simplified)
                                    console.log("Copying:", containerImage)
                                }
                            }
                        }
                    }
                }
            }

            // Container Status Group
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("Container Status")
                label: Label { text: parent.title; font.bold: true; leftPadding: 5 } // workaround dtk6declarative <= 6.0.41 bug

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            text: qsTr("Status")
                            font.bold: true
                        }

                        Item { Layout.fillWidth: true }

                        Button {
                            text: qsTr("Stop")
                            icon.name: "media-playback-stop-symbolic"
                            visible: containerStatus && containerStatus.toLowerCase().startsWith("up")
                            onClicked: {
                                if (containerName) {
                                    stateManager.distroboxManager.stopContainer(containerName);
                                }
                            }
                        }

                        Button {
                            text: qsTr("Terminal")
                            icon.name: "terminal-symbolic"
                            onClicked: {
                                if (containerName) {
                                    stateManager.distroboxManager.spawnTerminal(containerName);
                                }
                            }
                        }
                    }

                    Label {
                        text: containerStatus || "Unknown"
                        font.pixelSize: 12
                    }
                }
            }

            // Quick Actions Group
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("Quick Actions")
                label: Label { text: parent.title; font.bold: true; leftPadding: 5 } // workaround dtk6declarative <= 6.0.41 bug

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    ActionButton {
                        Layout.fillWidth: true
                        text: qsTr("Upgrade Container")
                        icon.name: "software-update-available-symbolic"
                        description: qsTr("Update all packages")

                        onClicked: {
                            if (containerName) {
                                stateManager.distroboxManager.upgradeContainer(containerName);
                            }
                        }
                    }

                    ActionButton {
                        Layout.fillWidth: true
                        text: qsTr("Applications")
                        icon.name: "view-list-bullet-symbolic"
                        description: qsTr("Manage exportable applications")

                        onClicked: {
                            if (containerName) {
                                stateManager.showExportableApps(containerName);
                            }
                        }
                    }

                    ActionButton {
                        Layout.fillWidth: true
                        text: qsTr("Clone Container")
                        icon.name: "edit-copy-symbolic"
                        description: qsTr("Create a copy of this container")

                        onClicked: {
                            if (containerName) {
                                cloneContainerDialog.sourceContainerName = containerName;
                                cloneContainerDialog.open();
                            }
                        }
                    }

                    ActionButton {
                        Layout.fillWidth: true
                        text: qsTr("Generate Desktop Entry")
                        icon.name: "application-x-desktop-symbolic"
                        description: qsTr("Create desktop shortcut for this container")

                        onClicked: {
                            if (containerName) {
                                stateManager.distroboxManager.generateEntry(containerName);
                            }
                        }
                    }

                    ActionButton {
                        Layout.fillWidth: true
                        text: {
                            if (containerInstallableFileExtension) {
                                return qsTr("Install %1 Package").arg(containerInstallableFileExtension.toUpperCase());
                            }
                            return qsTr("Install Package");
                        }
                        icon.name: "package-symbolic"
                        description: qsTr("Install packages into this container")
                        visible: containerInstallableFileExtension

                        onClicked: {
                            if (containerName) {
                                packageFileDialog.open();
                            }
                        }
                    }
                }
            }

            // Danger Zone Group
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("Danger Zone")
                label: Label { text: parent.title; font.bold: true; leftPadding: 5 } // workaround dtk6declarative <= 6.0.41 bug

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    ActionButton {
                        Layout.fillWidth: true
                        text: qsTr("Delete Desktop Entry")
                        icon.name: "application-x-desktop-symbolic"
                        description: qsTr("Remove desktop shortcut for this container")
                        descriptionColor: "#ff9800"

                        onClicked: {
                            if (containerName) {
                                stateManager.distroboxManager.deleteEntry(containerName);
                            }
                        }
                    }

                    ActionButton {
                        Layout.fillWidth: true
                        text: qsTr("Delete Container")
                        icon.name: "user-trash-symbolic"
                        description: qsTr("Permanently remove this container and all its data")
                        descriptionColor: "#f44336"

                        onClicked: {
                            deleteConfirmationDialog.open();
                        }
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }

    // Delete confirmation dialog
    MessageDialog {
        id: deleteConfirmationDialog
        title: qsTr("Delete Container")
        text: containerName ?
            "Are you sure you want to delete '" + containerName + "'?\n\nThis action cannot be undone." :
            "Are you sure you want to delete this container?"
        buttons: MessageDialog.Yes | MessageDialog.No

                onAccepted: {
        if (containerName) {
            stateManager.distroboxManager.deleteContainer(containerName);
            currentContainerName = "";
        }
    }
    }

    // Clone container dialog
    Dialog {
        id: cloneContainerDialog
        title: qsTr("Clone Container")
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        anchors.centerIn: Overlay.overlay

        width: 400
        height: Math.max(implicitHeight, 200)

        property string sourceContainerName: ""

        ColumnLayout {
            anchors.fill: parent
            spacing: 15

            Text {
                text: qsTr("Clone container: %1").arg(cloneContainerDialog.sourceContainerName)
                font.bold: true
                color: palette.windowText
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            Text {
                text: qsTr("Enter name for the new container:")
                color: palette.windowText
                Layout.fillWidth: true
            }

            TextField {
                id: cloneNameField
                Layout.fillWidth: true
                placeholderText: "Enter new container name"

                onAccepted: {
                    if (text.trim() !== "") {
                        cloneContainerDialog.accept();
                    }
                }
            }
        }

        onAccepted: {
            if (cloneNameField.text.trim() !== "" && cloneContainerDialog.sourceContainerName) {
                stateManager.distroboxManager.cloneContainer(
                    cloneContainerDialog.sourceContainerName,
                    cloneNameField.text.trim()
                );
                cloneNameField.text = "";
            }
        }

        onRejected: {
            cloneNameField.text = "";
        }
    }

    // Connection to handle container selection from state manager
    Connections {
        target: stateManager

        function onSelectedContainerNameChanged() {
            // Find container data by name and update display
            if (stateManager.selectedContainerName) {
                // This would ideally come from a container model
                // For now, we'll update when container list changes
            }
        }
    }

    // Exportable Apps Dialog
    ExportableAppsDialog {
        id: exportableAppsDialog
    }

    // Package File Dialog
    FileDialog {
        id: packageFileDialog
        title: qsTr("Select Package File")
        fileMode: FileDialog.OpenFile
        nameFilters: {
            if (containerInstallableFileExtension) {
                if (containerInstallableFileExtension === ".deb") {
                    return ["Debian packages (*.deb)"];
                } else if (containerInstallableFileExtension === ".rpm") {
                    return ["RPM packages (*.rpm)"];
                }
            }
            return ["All packages (*.deb *.rpm)"];
        }

        onAccepted: {
            if (containerName && selectedFile) {
                // Convert URL to local path
                const filePath = selectedFile.toString().replace("file://", "");
                stateManager.distroboxManager.installPackage(containerName, filePath);
            }
        }
    }

    // 监听 ContainerModel 的变化，而不是直接监听 onContainerListUpdated
    Connections {
        target: stateManager

        function onSelectedContainerNameChanged() {
            // 当选择的容器名称改变时，更新当前容器
            updateContainer(stateManager.selectedContainerName);
        }
    }

    Connections {
        target: stateManager.containerModel

        function onContainersChanged() {
            // 容器列表更新时，强制刷新属性绑定
            // 通过重新设置 currentContainerName 来触发属性更新
            var tempName = currentContainerName;
            if (tempName) {
                currentContainerName = "";
                currentContainerName = tempName;
            }
        }
    }

    Connections {
        target: stateManager.distroboxManager

        function onExportableAppsUpdated(apps) {
            console.log("Exportable apps updated");
            // ExportableAppsDialog now uses StateManager's ExportableAppsModel automatically
            // Refresh container list to update status (container might have been started)
            stateManager.distroboxManager.listContainers();
        }

        function onTerminalSpawned() {
            console.log("Terminal spawned successfully");
        }
    }

    // 公开方法，供 Main.qml 调用
    function showExportableApps(containerName) {
        if (containerName) {
            exportableAppsDialog.containerName = containerName;
            stateManager.distroboxManager.listExportableApps(containerName);
            exportableAppsDialog.open();
        }
    }
}
