import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

D.ApplicationWindow {
    id: root
    flags: Qt.Window | Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint | Qt.WindowTitleHint
    width: 800
    height: 600
    visible: true
    title: "DistroRack DDE"
    D.DWindow.enabled: true
    // color: "transparent"

    // D.StyledBehindWindowBlur {
    //     anchors.fill: parent
    //     control: root
    //     blendColor: {
    //         if (valid) {
    //             return DS.Style.control.selectColor(control ? control.palette.window : undefined, Qt.rgba(1, 1, 1, 0.8), Qt.rgba(0.06, 0.06, 0.06, 0.8))
    //         }
    //             return DS.Style.control.selectColor(undefined,
    //                                                 DS.Style.behindWindowBlur.lightNoBlurColor,
    //                                                 DS.Style.behindWindowBlur.darkNoBlurColor)
    //     }
    // }

    header: D.TitleBar {
        id: titleBar
        icon.name: "terminal-distrobox-icon"
        leftContent: D.ActionButton {
            palette.windowText: D.ColorSelector.textColor
            anchors {
                verticalCenter: parent.verticalCenter
                leftMargin: 30
                left: parent.left
            }
            hoverEnabled: enabled
            activeFocusOnTab: true
            icon {
                name: "sidebar"
            }
            background: Rectangle {
                property D.Palette pressedColor: D.Palette {
                    normal: Qt.rgba(0, 0, 0, 0.2)
                    normalDark: Qt.rgba(1, 1, 1, 0.25)
                }
                property D.Palette hoveredColor: D.Palette {
                    normal: Qt.rgba(0, 0, 0, 0.1)
                    normalDark: Qt.rgba(1, 1, 1, 0.1)
                }
                radius: DS.Style.control.radius
                color: parent.pressed ? D.ColorSelector.pressedColor : (parent.hovered ? D.ColorSelector.hoveredColor : "transparent")
                border {
                    color: parent.palette.highlight
                    width: parent.visualFocus ? DS.Style.control.focusBorderWidth : 0
                }
            }
            onClicked: {
                createContainerDialog.open()
            }
        }
        title: "DistroRack DDE"
        menu: Menu {
            Action {
                text: qsTr("Refresh")
                icon.name: "view-refresh"

                onTriggered: {
                    stateManager.distroboxManager.listContainers()
                }
            }
            Action {
                text: qsTr("Upgrade All")
                icon.name: "system-software-update"

                onTriggered: {
                    upgradeAllDialog.open()
                }
            }
            Action {
                text: qsTr("Stop All")
                icon.name: "process-stop"

                onTriggered: {
                    stopAllDialog.open()
                }
            }
            Action {
                text: qsTr("Generate All Entries")
                icon.name: "application-x-desktop"

                onTriggered: {
                    stateManager.distroboxManager.generateAllEntries();
                }
            }
            D.MenuSeparator {}
            D.ThemeMenu {}
            D.QuitAction {}
        }
    }

    // Main content area with sidebar and details view
    SplitView {
        anchors.fill: parent
        // topPadding: titleBar.height
        
        // Sidebar for container list
        ContainerList {
            id: containerList
            SplitView.preferredWidth: 200
            SplitView.minimumWidth: 0
            SplitView.maximumWidth: 250
            
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
        color: root.palette.window
        
        ColumnLayout {
            anchors.centerIn: parent
            
            Label {
                text: "No Containers"
                font.pointSize: 24
                Layout.alignment: Qt.AlignHCenter
            }
            
            Label {
                text: "Get started by creating a new container"
                Layout.alignment: Qt.AlignHCenter
            }
            
            Button {
                text: "Create Container"
                icon.name: "list-add"
                Layout.alignment: Qt.AlignHCenter
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
        title: "Upgrade All Containers"
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        
        width: 400
        height: 150
        
        Text {
            text: "Are you sure you want to upgrade all containers?\n\nThis operation may take a very long time and will update all packages in every container."
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
        title: "Stop All Containers"
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        
        width: 400
        height: 150
        
        Text {
            text: "Are you sure you want to stop all running containers?\n\nThis will shut down all containers but will not delete them."
            wrapMode: Text.WordWrap
            width: parent.width
            color: palette.windowText
        }
        
        onAccepted: {
            stateManager.distroboxManager.stopAllContainers()
        }
    }
    

}