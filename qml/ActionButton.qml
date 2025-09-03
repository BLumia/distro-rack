// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

AbstractButton {
    id: actionButton

    property alias description: descriptionLabel.text
    property alias descriptionColor: descriptionLabel.color

    implicitHeight: 40
    padding: 5

    background: Rectangle {
        color: "transparent"
        radius: 4

        Rectangle {
            anchors.fill: parent
            color: actionButton.hovered ? Qt.rgba(0, 0, 0, 0.05) : "transparent"
            radius: parent.radius
            Behavior on color {
                ColorAnimation { duration: 150 }
            }
        }
    }

    contentItem: RowLayout {
        spacing: 12

        ThemedIcon {
            id: iconItem
            iconName: actionButton.icon.name
            iconSourceSize: 20
            Layout.preferredWidth: 20
            Layout.alignment: Qt.AlignVCenter
        }

        Label {
            id: mainLabel
            text: actionButton.text
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            font.pixelSize: 14
        }

        Label {
            id: descriptionLabel
            Layout.alignment: Qt.AlignVCenter
            font.pixelSize: 11
            color: palette.text
        }
    }

    // 可选：添加按下效果
    transform: Scale {
        origin.x: actionButton.width / 2
        origin.y: actionButton.height / 2
        xScale: actionButton.pressed ? 0.98 : 1.0
        yScale: actionButton.pressed ? 0.98 : 1.0
        Behavior on xScale { NumberAnimation { duration: 100 } }
        Behavior on yScale { NumberAnimation { duration: 100 } }
    }
}
