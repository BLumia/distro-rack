// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import DistroRack 1.0

Item {
    id: containerList

    property alias containerCount: listView.count
    property bool autoSelectFirst: true  // 是否自动选择第一个容器

    signal containerSelected(string containerName)

    // 用于延迟选择第一个容器的计时器
    Timer {
        id: selectFirstContainerTimer
        interval: 100
        repeat: false
        onTriggered: {
            if (stateManager.containerModel && stateManager.containerModel.rowCount() > 0) {
                listView.currentIndex = 0;
                var firstContainerName = stateManager.containerModel.data(
                    stateManager.containerModel.index(0, 0),
                    ContainerModel.NameRole
                );
                // 更新状态管理器中的选中容器
                stateManager.selectedContainerName = firstContainerName;
                // 发送容器选择信号
                containerList.containerSelected(firstContainerName);
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        ScrollView {
            id: scrollView
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: listView
                model: stateManager.containerModel

                delegate: ItemDelegate {
                    width: ListView.view.width
                    text: model.name
                    icon.name: model.distroIconName
                    highlighted: ListView.isCurrentItem
                    onClicked: {
                        listView.currentIndex = index;
                        stateManager.selectedContainerName = model.name;
                        containerList.containerSelected(model.name);
                    }
                }
            }
        }

        // Tasks button at the bottom
        Button {
            id: tasksButton
            Layout.fillWidth: true
            Layout.margins: 12
            text: qsTr("Tasks")
            icon.name: "view-list-symbolic"
            flat: true

            onClicked: {
                taskManagerDialog.open()
            }

            // Show warning indicator if there are failed tasks
            Rectangle {
                id: warningIndicator
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 6
                width: 8
                height: 8
                radius: 4
                color: "orange"
                visible: false

                Connections {
                    target: stateManager.taskModel
                    function onDataChanged() {
                        warningIndicator.visible = hasFailedTasks()
                    }
                    function onRowsInserted() {
                        warningIndicator.visible = hasFailedTasks()
                    }
                    function onRowsRemoved() {
                        warningIndicator.visible = hasFailedTasks()
                    }
                }

                function hasFailedTasks() {
                    if (!stateManager.taskModel) return false

                    for (var i = 0; i < stateManager.taskModel.rowCount(); i++) {
                        var index = stateManager.taskModel.index(i, 0)
                        var status = stateManager.taskModel.data(index, TaskModel.StatusRole)
                        if (status === 3) { // Failed status
                            return true
                        }
                    }
                    return false
                }
            }
        }
    }

    // 监听 ContainerModel 的变化
    Connections {
        target: stateManager.containerModel

        function onContainersChanged() {
            // 如果容器列表不为空且设置为自动选择，则选择第一个容器
            if (autoSelectFirst && stateManager.containerModel.rowCount() > 0) {
                // 延迟执行以确保UI已更新
                selectFirstContainerTimer.start();
            }
        }
    }

    Connections {
        target: stateManager.distroboxManager

        function onCommandError(error) {
            console.log("Command error: " + error);
        }
    }
}
