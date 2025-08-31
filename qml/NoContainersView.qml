// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: noContainersView
    color: palette.window

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
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                stateManager.showCreateContainer()
            }
        }

        Button {
            text: qsTr("Refresh")
            icon.name: "view-refresh"
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                stateManager.distroboxManager.listContainers()
            }
        }
    }
}
