// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: themedIcon

    property string iconName: ""
    property int iconSourceSize: 48

    flat: true
    enabled: false

    // Set icon properties
    icon.name: iconName
    icon.width: iconSourceSize
    icon.height: iconSourceSize

    // Make the button size match the icon size
    implicitWidth: iconSourceSize
    implicitHeight: iconSourceSize

    // Remove any padding or margins
    padding: 0
    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0
}
