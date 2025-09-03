// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Dialog {
    id: exportableAppsDialog
    title: qsTr("Exportable Apps")
    modal: true
    standardButtons: Dialog.Close
    anchors.centerIn: Overlay.overlay

    width: 400
    height: 500

    // 存储当前容器名称
    property string containerName: ""

    StackLayout {
        id: dialogStackLayout
        anchors.fill: parent

        // 加载提示页面
        Item {
            id: loadingPage

            Column {
                anchors.centerIn: parent
                spacing: 20

                Label {
                    text: qsTr("Loading applications...")
                    font.pointSize: 14
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                BusyIndicator {
                    anchors.horizontalCenter: parent.horizontalCenter
                    running: true
                }
            }
        }

        // 应用列表页面
        ListView {
            id: appsListView
            model: stateManager.exportableAppsModel

            delegate: ItemDelegate {
                id: itemDelegate
                text: model.name
                width: ListView.view.width

                contentItem: RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Label {
                            text: model.name
                            font.bold: true
                        }

                        Label {
                            text: model.path
                            font.pixelSize: 12
                        }
                    }

                    ToolButton {
                        id: actionButton
                        Layout.alignment: Qt.AlignVCenter
                        visible: true

                        // 根据模型中的操作状态更新按钮启用状态
                        enabled: !model.operationInProgress

                        contentItem: Item {
                            anchors.fill: parent

                            RowLayout {
                                anchors.centerIn: parent
                                spacing: 5

                                Text {
                                    text: model.exported ? qsTr("Unexport") : qsTr("Export")
                                    color: actionButton.enabled ? palette.buttonText : palette.mid
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                BusyIndicator {
                                    running: model.operationInProgress
                                    visible: model.operationInProgress
                                    Layout.preferredWidth: 20
                                    Layout.preferredHeight: 20
                                }
                            }
                        }

                        onClicked: {
                            // 设置操作进行中状态到模型
                            stateManager.exportableAppsModel.setOperationInProgress(model.path, true);
                            // 记录当前正在操作的应用路径
                            exportableAppsDialog.currentOperatingAppPath = model.path;

                            if (model.exported) {
                                console.log("Unexport app:", model.name, "path:", model.path);
                                stateManager.distroboxManager.unexportApp(exportableAppsDialog.containerName, model.path);
                            } else {
                                console.log("Export app:", model.name, "path:", model.path);
                                stateManager.distroboxManager.exportApp(exportableAppsDialog.containerName, model.path);
                            }
                        }
                    }
                }

                onClicked: {
                    console.log("App selected:", model.name);
                }
            }
        }

        // 无应用提示页面
        Item {
            id: noAppsPage

            Column {
                anchors.centerIn: parent
                spacing: 20

                Text {
                    text: qsTr("No Applications Found")
                    font.pointSize: 18
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: palette.windowText
                }

                Text {
                    text: qsTr("No exportable applications were found in this container.")
                    anchors.horizontalCenter: parent.horizontalCenter
                    horizontalAlignment: Text.AlignHCenter
                    width: 300
                    wrapMode: Text.WordWrap
                    color: palette.windowText
                }
            }
        }
    }

    // 跟踪当前正在进行操作的应用路径
    property string currentOperatingAppPath: ""

    // 监听模型变化
    Connections {
        target: stateManager.exportableAppsModel

        function onAppsChanged() {
            // 根据应用数量切换显示页面
            if (stateManager.exportableAppsModel.rowCount() > 0) {
                dialogStackLayout.currentIndex = 1; // 显示应用列表
            } else {
                dialogStackLayout.currentIndex = 2; // 显示无应用提示
            }
        }
    }

    // 连接到DistroboxManager的信号
    Connections {
        target: stateManager.distroboxManager

        function onExportAppResult(result) {
            console.log("App exported successfully");
            // 重置当前操作应用的操作状态
            if (exportableAppsDialog.currentOperatingAppPath) {
                stateManager.exportableAppsModel.setOperationInProgress(exportableAppsDialog.currentOperatingAppPath, false);
                exportableAppsDialog.currentOperatingAppPath = "";
            }
            // 主动刷新应用列表以获取最新的导出状态
            stateManager.distroboxManager.listExportableApps(exportableAppsDialog.containerName);
        }

        function onUnexportAppResult(result) {
            console.log("App unexported successfully");
            // 重置当前操作应用的操作状态
            if (exportableAppsDialog.currentOperatingAppPath) {
                stateManager.exportableAppsModel.setOperationInProgress(exportableAppsDialog.currentOperatingAppPath, false);
                exportableAppsDialog.currentOperatingAppPath = "";
            }
            // 主动刷新应用列表以获取最新的导出状态
            stateManager.distroboxManager.listExportableApps(exportableAppsDialog.containerName);
        }

        function onCommandError(error) {
            console.log("Command error:", error);
            // 重置当前操作状态
            if (exportableAppsDialog.currentOperatingAppPath) {
                stateManager.exportableAppsModel.setOperationInProgress(exportableAppsDialog.currentOperatingAppPath, false);
                exportableAppsDialog.currentOperatingAppPath = "";
            }
        }
    }

    onOpened: {
        // 当对话框打开时，显示加载页面
        dialogStackLayout.currentIndex = 0;

        // 强制检查当前模型状态并设置正确的页面
        // FIXME: we shouldn't need to do this.
        Qt.callLater(function() {
            if (stateManager.exportableAppsModel.rowCount() > 0) {
                console.log("Setting to apps list page");
                dialogStackLayout.currentIndex = 1;
            }
        });
    }
}
