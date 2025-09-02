// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

import QtQuick 2.15
import org.kde.kirigami as Kirigami

Kirigami.Icon {
    property string iconName: ""
    property int iconSourceSize: 48

    implicitWidth: iconSourceSize
    implicitHeight: iconSourceSize
    source: iconName
}
