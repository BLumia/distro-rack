// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs

ScrollView {
    id: root
    property var terminalManager

    ColumnLayout {
        width: root.width
        spacing: 20

        GroupBox {
            title: qsTr("Terminal Selection")
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                Label {
                    text: qsTr("Select your preferred terminal emulator:")
                    Layout.fillWidth: true
                }

                ListView {
                    id: terminalListView
                    Layout.fillWidth: true
                    Layout.preferredHeight: 150
                    model: root.terminalManager ? root.terminalManager.terminalModel : null
                    clip: true

                    delegate: ItemDelegate {
                        width: ListView.view.width

                        contentItem: RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10

                            RadioButton {
                                id: radioButton
                                checked: root.terminalManager ? (root.terminalManager.selectedTerminal === model.name) : false
                                onToggled: {
                                    if (checked && root.terminalManager) {
                                        root.terminalManager.selectedTerminal = model.name
                                    }
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 1

                                Label {
                                    text: model.displayName || model.name
                                    font.bold: true
                                    Layout.fillWidth: true
                                }

                                Label {
                                    text: qsTr("Command: %1").arg(model.command)
                                    font.pointSize: 9
                                    color: palette.placeholderText
                                    Layout.fillWidth: true
                                }

                                Label {
                                    text: model.isCustom ? qsTr("Custom terminal") : qsTr("Built-in terminal")
                                    font.pointSize: 8
                                    color: model.isCustom ? "#2196F3" : "#4CAF50"
                                    Layout.fillWidth: true
                                }
                            }

                            Label {
                                text: root.terminalManager && root.terminalManager.isTerminalAvailable(model.command) ?
                                      qsTr("✓ Available") : qsTr("✗ Not found")
                                color: root.terminalManager && root.terminalManager.isTerminalAvailable(model.command) ?
                                       "#4CAF50" : "#F44336"
                                font.pointSize: 9
                            }

                            ToolButton {
                                text: qsTr("Edit")
                                visible: model.isCustom
                                onClicked: {
                                    editTerminalDialog.editTerminal(model.name, model.displayName, model.command, model.separator)
                                }
                            }

                            ToolButton {
                                text: qsTr("Remove")
                                visible: model.isCustom
                                onClicked: {
                                    removeConfirmDialog.terminalName = model.name
                                    removeConfirmDialog.open()
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    Button {
                        text: qsTr("Add Custom Terminal")
                        icon.name: "list-add"
                        onClicked: {
                            editTerminalDialog.addNewTerminal()
                        }
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: qsTr("Detect Default")
                        icon.name: "view-refresh"
                        onClicked: {
                            if (root.terminalManager) {
                                var defaultTerminal = root.terminalManager.detectDefaultTerminal()
                                root.terminalManager.selectedTerminal = defaultTerminal
                                statusLabel.text = qsTr("Default terminal detected: %1").arg(defaultTerminal)
                                statusLabel.visible = true
                                statusTimer.restart()
                            }
                        }
                    }

                    Button {
                        text: qsTr("Reset to Defaults")
                        icon.name: "edit-undo"
                        onClicked: {
                            resetConfirmDialog.open()
                        }
                    }
                }

                Label {
                    id: statusLabel
                    visible: false
                    color: "#4CAF50"
                    Layout.fillWidth: true

                    Timer {
                        id: statusTimer
                        interval: 3000
                        onTriggered: statusLabel.visible = false
                    }
                }
            }
        }

        GroupBox {
            title: qsTr("Current Selection")
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                Label {
                    text: root.terminalManager ?
                          qsTr("Currently selected: %1").arg(root.terminalManager.selectedTerminal) :
                          qsTr("No terminal manager available")
                    font.bold: true
                    Layout.fillWidth: true
                }

                Label {
                    text: root.terminalManager ?
                          qsTr("System default: %1").arg(root.terminalManager.defaultTerminal) :
                          ""
                    color: palette.placeholderText
                    Layout.fillWidth: true
                }
            }
        }
    }

    // Add/Edit Terminal Dialog
    Dialog {
        id: editTerminalDialog
        title: isEditMode ? qsTr("Edit Terminal") : qsTr("Add Custom Terminal")
        modal: true
        width: 500
        height: 350
        anchors.centerIn: Overlay.overlay

        property bool isEditMode: false
        property string originalName: ""

        standardButtons: Dialog.Save | Dialog.Cancel

        function addNewTerminal() {
            isEditMode = false
            originalName = ""
            nameField.text = ""
            displayNameField.text = ""
            commandField.text = ""
            separatorField.text = "-e"
            open()
        }

        function editTerminal(name, displayName, command, separator) {
            isEditMode = true
            originalName = name
            nameField.text = name
            displayNameField.text = displayName
            commandField.text = command
            separatorField.text = separator
            open()
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            Label {
                text: qsTr("Terminal Configuration")
                font.bold: true
                font.pointSize: 14
                Layout.fillWidth: true
            }

            GridLayout {
                columns: 2
                Layout.fillWidth: true
                columnSpacing: 10
                rowSpacing: 10

                Label { text: qsTr("Name:") }
                TextField {
                    id: nameField
                    Layout.fillWidth: true
                    placeholderText: qsTr("e.g., my-terminal")
                    enabled: !editTerminalDialog.isEditMode
                }

                Label { text: qsTr("Display Name:") }
                TextField {
                    id: displayNameField
                    Layout.fillWidth: true
                    placeholderText: qsTr("e.g., My Custom Terminal")
                }

                Label { text: qsTr("Command:") }
                TextField {
                    id: commandField
                    Layout.fillWidth: true
                    placeholderText: qsTr("e.g., gnome-terminal")
                }

                Label { text: qsTr("Separator:") }
                TextField {
                    id: separatorField
                    Layout.fillWidth: true
                    placeholderText: qsTr("e.g., -e or --")
                }
            }

            Label {
                text: qsTr("The separator is used between the terminal command and the actual command to execute. Common values are '-e', '--', or '-x'.")
                wrapMode: Text.WordWrap
                font.pointSize: 9
                color: palette.placeholderText
                Layout.fillWidth: true
            }
        }

        onAccepted: {
            if (!nameField.text.trim() || !commandField.text.trim()) {
                errorDialog.text = qsTr("Name and command cannot be empty!")
                errorDialog.open()
                return
            }

            var success = false
            if (editTerminalDialog.isEditMode) {
                success = root.terminalManager.updateCustomTerminal(
                    originalName,
                    displayNameField.text.trim(),
                    commandField.text.trim(),
                    separatorField.text.trim()
                )
            } else {
                success = root.terminalManager.addCustomTerminal(
                    nameField.text.trim(),
                    displayNameField.text.trim(),
                    commandField.text.trim(),
                    separatorField.text.trim()
                )
            }

            if (success) {
                statusLabel.text = editTerminalDialog.isEditMode ?
                                   qsTr("Terminal updated successfully") :
                                   qsTr("Terminal added successfully")
                statusLabel.visible = true
                statusTimer.restart()
            } else {
                errorDialog.text = qsTr("Failed to save terminal configuration. Please check if the command is available.")
                errorDialog.open()
            }
        }
    }

    // Remove Confirmation Dialog
    Dialog {
        id: removeConfirmDialog
        title: qsTr("Remove Terminal")
        modal: true
        width: 400
        height: 200
        anchors.centerIn: Overlay.overlay

        property string terminalName: ""

        standardButtons: Dialog.Yes | Dialog.No

        Label {
            text: qsTr("Are you sure you want to remove the terminal '%1'?").arg(removeConfirmDialog.terminalName)
            wrapMode: Text.WordWrap
            anchors.fill: parent
            anchors.margins: 20
        }

        onAccepted: {
            if (root.terminalManager && root.terminalManager.removeCustomTerminal(removeConfirmDialog.terminalName)) {
                statusLabel.text = qsTr("Terminal removed successfully")
                statusLabel.visible = true
                statusTimer.restart()
            }
        }
    }

    // Reset Confirmation Dialog
    Dialog {
        id: resetConfirmDialog
        title: qsTr("Reset to Defaults")
        modal: true
        width: 400
        height: 200
        anchors.centerIn: Overlay.overlay

        standardButtons: Dialog.Yes | Dialog.No

        Label {
            text: qsTr("Are you sure you want to reset all terminal settings to defaults?\n\nThis will remove all custom terminals and reset the selection to the system default.")
            wrapMode: Text.WordWrap
            anchors.fill: parent
            anchors.margins: 20
        }

        onAccepted: {
            if (root.terminalManager) {
                root.terminalManager.resetToDefaults()
                statusLabel.text = qsTr("Settings reset to defaults")
                statusLabel.visible = true
                statusTimer.restart()
            }
        }
    }

    // Error Dialog
    Dialog {
        id: errorDialog
        title: qsTr("Error")
        modal: true
        width: 350
        height: 150
        anchors.centerIn: Overlay.overlay

        property alias text: errorLabel.text

        standardButtons: Dialog.Ok

        Label {
            id: errorLabel
            wrapMode: Text.WordWrap
            anchors.fill: parent
            anchors.margins: 20
        }
    }
}
