// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

import QtQuick 2.15
import org.deepin.dtk 1.0 as D

D.DciIcon {
    id: themedIcon

    property string iconName: ""
    property int iconSourceSize: 48

    // Bind to DciIcon properties
    name: iconName
    sourceSize: Qt.size(iconSourceSize, iconSourceSize)
}
