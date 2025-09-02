// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import DistroRack 1.0

Dialog {
    id: taskManagerDialog
    title: qsTr("Running Tasks")
    modal: true
    standardButtons: Dialog.Close
    anchors.centerIn: Overlay.overlay

    width: 600
    height: 500

    property string selectedTaskId: ""

    StackLayout {
        id: stackLayout
        anchors.fill: parent
        currentIndex: stateManager.taskModel.rowCount() > 0 ? 1 : 0

        // Empty state page
        Item {
            id: emptyPage

            Column {
                anchors.centerIn: parent
                spacing: 20

                Text {
                    text: qsTr("No Running Tasks")
                    font.pointSize: 18
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: palette.windowText
                }

                Text {
                    text: qsTr("Tasks such as creating, deleting and upgrading containers will appear here.")
                    anchors.horizontalCenter: parent.horizontalCenter
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width - 40
                    wrapMode: Text.WordWrap
                    color: palette.windowText
                }
            }
        }

        // Task list page
        ColumnLayout {
            id: taskListPage
            spacing: 10

            // Task list
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: 10

                ListView {
                    id: taskListView
                    model: stateManager.taskModel
                    spacing: 2

                    delegate: ItemDelegate {
                        width: ListView.view.width
                        height: rowLayout.implicitHeight + 20

                        Rectangle {
                            anchors.fill: parent
                            color: parent.hovered ? palette.highlight : "transparent"
                            opacity: 0.1
                            radius: 4
                        }

                        RowLayout {
                            id: rowLayout
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 10

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Text {
                                    text: model.target ? model.target + ": " + model.name : model.name
                                    font.bold: true
                                    color: palette.windowText
                                }

                                Text {
                                    text: model.statusString
                                    font.pixelSize: 12
                                    color: {
                                        switch(model.statusString) {
                                            case "pending": return "orange"
                                            case "executing": return "blue"
                                            case "successful": return "green"
                                            case "failed": return "red"
                                            default: return palette.windowText
                                        }
                                    }
                                }

                                Label {
                                    text: model.description
                                    font.pixelSize: 11
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }
                            }

                            // Status indicator
                            Rectangle {
                                width: 12
                                height: 12
                                radius: 6
                                color: {
                                    switch(model.statusString) {
                                        case "pending": return "orange"
                                        case "executing": return "blue"
                                        case "successful": return "green"
                                        case "failed": return "red"
                                        default: return "gray"
                                    }
                                }
                            }
                        }

                        onClicked: {
                            taskManagerDialog.selectedTaskId = model.taskId
                            stateManager.selectedTaskId = model.taskId
                            stackLayout.currentIndex = 2 // 切换到详情页面
                        }
                    }
                }
            }

            // Bottom button
            Button {
                id: clearEndedTasksButton
                Layout.alignment: Qt.AlignCenter
                text: qsTr("Clear Ended Tasks")
                enabled: hasEndedTasks()
                onClicked: {
                    stateManager.clearEndedTasks()
                }

                function hasEndedTasks() {
                    for (var i = 0; i < stateManager.taskModel.rowCount(); i++) {
                        var isEnded = stateManager.taskModel.data(stateManager.taskModel.index(i, 0), TaskModel.IsEndedRole)
                        if (isEnded) return true
                    }
                    return false
                }

                // 监听模型变化以更新按钮状态
                Connections {
                    target: stateManager.taskModel
                    function onDataChanged() {
                        clearEndedTasksButton.enabled = clearEndedTasksButton.hasEndedTasks()
                    }
                    function onRowsInserted() {
                        clearEndedTasksButton.enabled = clearEndedTasksButton.hasEndedTasks()
                    }
                    function onRowsRemoved() {
                        clearEndedTasksButton.enabled = clearEndedTasksButton.hasEndedTasks()
                    }
                }
            }
        }

        // Task detail page
        ColumnLayout {
            id: taskDetailPage
            spacing: 10

            // Header with back button
            RowLayout {
                Layout.fillWidth: true
                Layout.margins: 10

                Button {
                    text: qsTr("← Back")
                    onClicked: {
                        stackLayout.currentIndex = 1
                        taskManagerDialog.selectedTaskId = ""
                        stateManager.selectedTaskId = ""
                    }
                }

                Text {
                    Layout.fillWidth: true
                    text: qsTr("Task Details")
                    font.bold: true
                    font.pointSize: 14
                    horizontalAlignment: Text.AlignCenter
                    color: palette.windowText
                }

                Item {
                    width: 80 // Spacer to center the title
                }
            }

            // Task info
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: 10
                clip: true
                contentWidth: availableWidth

                ColumnLayout {
                    width: parent.width
                    spacing: 10

                    Text {
                        id: detailTaskName
                        Layout.fillWidth: true
                        font.bold: true
                        font.pointSize: 12
                        color: palette.windowText
                        wrapMode: Text.WordWrap
                    }

                    Text {
                        id: detailTaskStatus
                        Layout.fillWidth: true
                        font.pixelSize: 11
                        color: palette.windowText
                    }

                    Label {
                        id: detailTaskDescription
                        Layout.fillWidth: true
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 150
                        Layout.preferredHeight: 200
                        Layout.maximumHeight: 300
                        color: palette.base
                        border.color: palette.mid
                        border.width: 1
                        radius: 4

                        ScrollView {
                            anchors.fill: parent
                            anchors.margins: 5

                            TextArea {
                                id: detailOutputArea
                                readOnly: true
                                selectByMouse: true
                                wrapMode: TextArea.Wrap
                                color: palette.windowText
                                font.family: "Courier, monospace"
                                font.pixelSize: 10
                                placeholderText: "Task output will appear here..."
                            }
                        }
                    }

                    Text {
                        id: detailErrorMessage
                        Layout.fillWidth: true
                        visible: text.length > 0
                        color: "red"
                        wrapMode: Text.WordWrap
                        font.pixelSize: 11
                    }
                }
            }
        }
    }

    // Update detail view when selected task changes
    Connections {
        target: stateManager
        function onSelectedTaskIdChanged() {
            updateTaskDetail()
        }
    }

    // Update detail view when task model changes
    Connections {
        target: stateManager.taskModel
        function onTaskUpdated(taskId) {
            if (taskId === taskManagerDialog.selectedTaskId) {
                updateTaskDetail()
            }
        }
    }

    function updateTaskDetail() {
        if (!taskManagerDialog.selectedTaskId) return

        // 使用实际的角色常量而不是硬编码数字
        for (var i = 0; i < stateManager.taskModel.rowCount(); i++) {
            var index = stateManager.taskModel.index(i, 0)
            var taskId = stateManager.taskModel.data(index, TaskModel.IdRole)

            console.log("Checking task:", taskId, "against selected:", taskManagerDialog.selectedTaskId)

            if (taskId === taskManagerDialog.selectedTaskId) {
                var name = stateManager.taskModel.data(index, TaskModel.NameRole)
                var description = stateManager.taskModel.data(index, TaskModel.DescriptionRole)
                var target = stateManager.taskModel.data(index, TaskModel.TargetRole)
                var statusString = stateManager.taskModel.data(index, TaskModel.StatusStringRole)
                var output = stateManager.taskModel.data(index, TaskModel.OutputRole)
                var errorMessage = stateManager.taskModel.data(index, TaskModel.ErrorMessageRole)

                console.log("Task details:", name, statusString, description)

                detailTaskName.text = target ? target + ": " + name : name
                detailTaskStatus.text = "Status: " + statusString
                detailTaskDescription.text = description
                detailOutputArea.text = output || ""
                detailErrorMessage.text = errorMessage || ""
                break
            }
        }
    }

    onOpened: {
        // 当对话框打开时，显示任务列表页面
        stackLayout.currentIndex = stateManager.taskModel.rowCount() > 0 ? 1 : 0
        selectedTaskId = ""
        stateManager.selectedTaskId = ""
    }
}
