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
                    
                    Button {
                        anchors.centerIn: parent
                        width: 48
                        height: 48
                        flat: true
                        enabled: false
                        icon.name: {
                        if (containerDistroIconName) {
                            return containerDistroIconName;
                        }
                        return "distributor-logo-generic";
                        }
                        icon.width: 48
                        icon.height: 48
                    }
                }
                
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    
                    Text {
                        text: containerName || "No Container Selected"
                        font.pixelSize: 24
                        font.bold: true
                        color: palette.windowText
                    }
                    
                    RowLayout {
                        spacing: 6
                        
                        Text {
                            text: containerImage
                            color: palette.mid
                            font.pixelSize: 12
                        }
                        
                        Button {
                            text: "Copy"
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
                title: "Container Status"
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8
                    
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Text {
                            text: "Status"
                            font.bold: true
                            color: palette.windowText
                        }
                        
                        Item { Layout.fillWidth: true }
                        
                                                 Button {
                             text: "Stop"
                             icon.name: "media-playback-stop-symbolic"
                             visible: containerStatus && containerStatus.toLowerCase().startsWith("up")
                             onClicked: {
                                 if (containerName) {
                                     stateManager.distroboxManager.stopContainer(containerName);
                                 }
                             }
                         }
                        
                        Button {
                            text: "Terminal"
                            icon.name: "terminal-symbolic"
                            onClicked: {
                                if (containerName) {
                                    stateManager.distroboxManager.spawnTerminal(containerName);
                                }
                            }
                        }
                    }
                    
                    Text {
                        text: containerStatus || "Unknown"
                        color: palette.mid
                        font.pixelSize: 12
                    }
                }
            }
            
            // Quick Actions Group
            GroupBox {
                Layout.fillWidth: true
                title: "Quick Actions"
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4
                    
                    ItemDelegate {
                        Layout.fillWidth: true
                        text: "Upgrade Container"
                        icon.name: "software-update-available-symbolic"
                        
                        onClicked: {
                            if (containerName) {
                                stateManager.distroboxManager.upgradeContainer(containerName);
                            }
                        }
                        
                        Label {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.rightMargin: 16
                            text: "Update all packages"
                            color: palette.mid
                            font.pixelSize: 11
                        }
                    }
                    
                    ItemDelegate {
                         Layout.fillWidth: true
                         text: qsTr("Applications")
                         icon.name: "view-list-bullet-symbolic"
                         
                         onClicked: {
                             if (containerName) {
                                 exportableAppsDialog.containerName = containerName;
                                 stateManager.distroboxManager.listExportableApps(containerName);
                                 exportableAppsDialog.open();
                             }
                         }
                        
                        Label {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.rightMargin: 16
                            text: "Manage exportable applications"
                            color: palette.mid
                            font.pixelSize: 11
                        }
                    }
                    
                    ItemDelegate {
                        Layout.fillWidth: true
                        text: qsTr("Clone Container")
                        icon.name: "edit-copy-symbolic"
                        
                        onClicked: {
                            if (containerName) {
                                cloneContainerDialog.sourceContainerName = containerName;
                                cloneContainerDialog.open();
                            }
                        }
                        
                        Label {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.rightMargin: 16
                            text: "Create a copy of this container"
                            color: palette.mid
                            font.pixelSize: 11
                        }
                    }
                    
                    ItemDelegate {
                        Layout.fillWidth: true
                        text: "Generate Desktop Entry"
                        icon.name: "application-x-desktop-symbolic"
                        
                        onClicked: {
                            if (containerName) {
                                stateManager.distroboxManager.generateEntry(containerName);
                            }
                        }
                        
                        Label {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.rightMargin: 16
                            text: "Create desktop shortcut for this container"
                            color: palette.mid
                            font.pixelSize: 11
                        }
                    }
                     
                    ItemDelegate {
                        Layout.fillWidth: true
                        text: {
                            if (containerInstallableFileExtension) {
                                return "Install " + containerInstallableFileExtension.toUpperCase() + " Package";
                            }
                            return "Install Package";
                        }
                        icon.name: "package-symbolic"
                        visible: containerInstallableFileExtension
                        
                        onClicked: {
                            if (containerName) {
                                packageFileDialog.open();
                            }
                        }
                        
                        Label {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.rightMargin: 16
                            text: "Install packages into this container"
                            color: palette.mid
                            font.pixelSize: 11
                        }
                    }
                }
            }
            
            // Danger Zone Group
            GroupBox {
                Layout.fillWidth: true
                title: "Danger Zone"
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4
                    
                    ItemDelegate {
                        Layout.fillWidth: true
                        text: "Delete Desktop Entry"
                        icon.name: "application-x-desktop-symbolic"
                        
                        onClicked: {
                            if (containerName) {
                                stateManager.distroboxManager.deleteEntry(containerName);
                            }
                        }
                        
                        Label {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.rightMargin: 16
                            text: "Remove desktop shortcut for this container"
                            color: "#ff9800"
                            font.pixelSize: 11
                        }
                    }
                     
                    ItemDelegate {
                        Layout.fillWidth: true
                        text: "Delete Container"
                        icon.name: "user-trash-symbolic"
                        
                        onClicked: {
                            deleteConfirmationDialog.open();
                        }
                        
                        Label {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.rightMargin: 16
                            text: "Permanently remove this container and all its data"
                            color: "#f44336"
                            font.pixelSize: 11
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
        title: "Delete Container"
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
        title: "Clone Container"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        
        width: 400
        height: 200
        
        property string sourceContainerName: ""
        
        Column {
            width: parent.width
            spacing: 15
            
            Text {
                text: "Clone container: " + cloneContainerDialog.sourceContainerName
                font.bold: true
                color: palette.windowText
            }
            
            Text {
                text: "Enter name for the new container:"
                color: palette.windowText
            }
            
            TextField {
                id: cloneNameField
                width: parent.width
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
        title: "Select Package File"
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
            // 容器列表更新时，如果当前容器仍然存在，保持显示
            // 容器属性会自动通过 getContainerData 更新
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
}
