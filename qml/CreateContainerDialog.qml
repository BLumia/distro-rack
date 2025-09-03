// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs

Dialog {
    id: createDialog
    title: qsTr("Create a Distrobox")
    modal: true
    anchors.centerIn: Overlay.overlay

    width: Math.min(500, Overlay.overlay ? Overlay.overlay.width * 0.9 : 500)
    height: Math.min(600, Overlay.overlay ? Overlay.overlay.height * 0.9 : 600)

    // Properties for the guided creation
    property alias nameField: nameField
    property alias imageCombo: imageCombo
    property alias nvidiaCheckBox: nvidiaCheckBox
    property alias initCheckBox: initCheckBox
    property alias homeDirField: homeDirField
    property alias volumesList: volumesList

    // Properties for assemble
    property string assembleFilePath: ""
    property string assembleUrl: ""

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // View switcher
        TabBar {
            id: tabBar
            Layout.fillWidth: true

            TabButton {
                text: qsTr("Guided")
            }
            TabButton {
                text: qsTr("From File")
            }
            TabButton {
                text: qsTr("From URL")
            }
        }

        StackLayout {
            id: stackLayout
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            // Guided creation page
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: availableWidth

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 24
                    spacing: 20

                    GroupBox {
                        title: qsTr("Settings")
                        label: Label { text: parent.title; font.bold: true; } // workaround dtk6declarative <= 6.0.41 bug
                        Layout.fillWidth: true

                        GridLayout {
                            columns: 2
                            rowSpacing: 12
                            columnSpacing: 12
                            anchors.fill: parent

                            Label {
                                text: qsTr("Name")
                                font.bold: true
                            }

                            TextField {
                                id: nameField
                                Layout.fillWidth: true
                                placeholderText: qsTr("Enter container name")
                            }

                            Label {
                                text: qsTr("Base Image")
                                font.bold: true
                            }

                            ComboBox {
                                id: imageCombo
                                Layout.fillWidth: true
                                editable: true
                                model: ListModel {
                                    ListElement { text: "ubuntu:latest" }
                                    ListElement { text: "fedora:latest" }
                                    ListElement { text: "debian:latest" }
                                    ListElement { text: "archlinux:latest" }
                                    ListElement { text: "opensuse/tumbleweed" }
                                    ListElement { text: "alpine:latest" }
                                }
                            }

                            Label {
                                text: qsTr("Custom Home Directory")
                                font.bold: true
                            }

                            RowLayout {
                                Layout.fillWidth: true

                                TextField {
                                    id: homeDirField
                                    Layout.fillWidth: true
                                    placeholderText: qsTr("Leave empty for default")
                                }

                                Button {
                                    text: qsTr("Browse...")
                                    onClicked: {
                                        folderDialog.open()
                                    }
                                }
                            }
                        }
                    }

                    GroupBox {
                        title: qsTr("Options")
                        label: Label { text: parent.title; font.bold: true; } // workaround dtk6declarative <= 6.0.41 bug
                        Layout.fillWidth: true

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 8

                            CheckBox {
                                id: nvidiaCheckBox
                                text: qsTr("NVIDIA Support")
                            }

                            CheckBox {
                                id: initCheckBox
                                text: qsTr("Init process")
                            }
                        }
                    }

                    GroupBox {
                        title: qsTr("Volumes")
                        label: Label { text: parent.title; font.bold: true; } // workaround dtk6declarative <= 6.0.41 bug
                        Layout.fillWidth: true

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 12

                            Label {
                                text: qsTr("Specify volumes in the format 'host_path:container_path'")
                                font.pixelSize: 11
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                            }

                            RowLayout {
                                Layout.fillWidth: true

                                TextField {
                                    id: volumeField
                                    Layout.fillWidth: true
                                    placeholderText: "/path/on/host:/path/in/container"
                                }

                                Button {
                                    text: qsTr("Add Volume")
                                    enabled: volumeField.text.trim() !== ""
                                    onClicked: {
                                        if (volumeField.text.trim() !== "") {
                                            volumesModel.append({text: volumeField.text.trim()})
                                            volumeField.text = ""
                                        }
                                    }
                                }
                            }

                            ListView {
                                id: volumesList
                                Layout.fillWidth: true
                                Layout.preferredHeight: Math.min(contentHeight, 120)
                                model: ListModel {
                                    id: volumesModel
                                }

                                delegate: RowLayout {
                                    width: volumesList.width
                                    spacing: 8

                                    Label {
                                        text: model.text
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                    }

                                    Button {
                                        text: qsTr("Remove")
                                        icon.name: "user-trash-symbolic"
                                        flat: true
                                        onClicked: {
                                            volumesModel.remove(index)
                                        }
                                    }
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignCenter

                        Item { Layout.fillWidth: true }

                        Button {
                            text: qsTr("Create")
                            enabled: nameField.text.trim() !== "" && imageCombo.currentText.trim() !== ""
                            highlighted: true
                            onClicked: {
                                createGuidedContainer()
                            }
                        }

                        Item { Layout.fillWidth: true }
                    }

                    Item {
                        Layout.fillHeight: true
                    }
                }
            }

            // From File page
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: availableWidth

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 24
                    spacing: 20

                    GroupBox {
                        title: qsTr("Assemble from File")
                        label: Label { text: parent.title; font.bold: true; } // workaround dtk6declarative <= 6.0.41 bug
                        Layout.fillWidth: true

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 12

                            Label {
                                text: qsTr("Create a container from an assemble file")
                                Layout.fillWidth: true
                            }

                            RowLayout {
                                Layout.fillWidth: true

                                TextField {
                                    id: filePathField
                                    Layout.fillWidth: true
                                    placeholderText: qsTr("Select assemble file...")
                                    text: createDialog.assembleFilePath
                                    readOnly: true
                                }

                                Button {
                                    text: qsTr("Browse...")
                                    onClicked: {
                                        assembleFileDialog.open()
                                    }
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignCenter

                        Item { Layout.fillWidth: true }

                        Button {
                            text: qsTr("Create")
                            enabled: createDialog.assembleFilePath !== ""
                            highlighted: true
                            onClicked: {
                                createFromFile()
                            }
                        }

                        Item { Layout.fillWidth: true }
                    }

                    Item {
                        Layout.fillHeight: true
                    }
                }
            }

            // From URL page
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: availableWidth

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 24
                    spacing: 20

                    GroupBox {
                        title: qsTr("From URL")
                        label: Label { text: parent.title; font.bold: true; } // workaround dtk6declarative <= 6.0.41 bug
                        Layout.fillWidth: true

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 12

                            Label {
                                text: qsTr("Create a container from a remote URL")
                                Layout.fillWidth: true
                            }

                            TextField {
                                id: urlField
                                Layout.fillWidth: true
                                placeholderText: "https://example.com/container.ini"
                                text: createDialog.assembleUrl
                                onTextChanged: {
                                    createDialog.assembleUrl = text
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignCenter

                        Item { Layout.fillWidth: true }

                        Button {
                            text: qsTr("Create")
                            enabled: createDialog.assembleUrl.trim() !== ""
                            highlighted: true
                            onClicked: {
                                createFromUrl()
                            }
                        }

                        Item { Layout.fillWidth: true }
                    }

                    Item {
                        Layout.fillHeight: true
                    }
                }
            }
        }
    }

    // File dialogs
    FolderDialog {
        id: folderDialog
        title: qsTr("Select Home Directory")
        onAccepted: {
            homeDirField.text = selectedFile.toString().replace('file://', '')
        }
    }

    FileDialog {
        id: assembleFileDialog
        title: qsTr("Select Assemble File")
        nameFilters: ["Configuration files (*.ini *.yaml *.yml)", "All files (*)"]
        onAccepted: {
            createDialog.assembleFilePath = selectedFile.toString().replace('file://', '')
        }
    }

    // Methods
    function createGuidedContainer() {
        // Collect volumes
        var volumes = []
        for (var i = 0; i < volumesModel.count; i++) {
            volumes.push(volumesModel.get(i).text)
        }

        // Call the C++ method
        stateManager.distroboxManager.createContainer(
            nameField.text.trim(),
            imageCombo.currentText.trim(),
            nvidiaCheckBox.checked,
            initCheckBox.checked,
            homeDirField.text.trim() || "",
            volumes
        )

        // Reset form
        resetForm()
        close()
    }

    function createFromFile() {
        stateManager.distroboxManager.assembleFromFile(createDialog.assembleFilePath)

        // Reset
        createDialog.assembleFilePath = ""
        close()
    }

    function createFromUrl() {
        stateManager.distroboxManager.assembleFromUrl(createDialog.assembleUrl.trim())

        // Reset
        createDialog.assembleUrl = ""
        close()
    }

    function resetForm() {
        nameField.text = ""
        imageCombo.currentIndex = 0
        nvidiaCheckBox.checked = false
        initCheckBox.checked = false
        homeDirField.text = ""
        volumeField.text = ""
        volumesModel.clear()
        createDialog.assembleFilePath = ""
        createDialog.assembleUrl = ""
        tabBar.currentIndex = 0
    }

    onOpened: {
        nameField.forceActiveFocus()
    }

    onClosed: {
        resetForm()
    }
}
