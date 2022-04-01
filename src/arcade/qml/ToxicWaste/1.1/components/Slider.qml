/****************************************************************************
 **
 ** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
 ** Contact: http://www.qt-project.org/legal
 **
 ** This file is part of the QtDeclarative module of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL$
 ** Commercial License Usage
 ** Licensees holding valid commercial Qt licenses may use this file in
 ** accordance with the commercial license agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and Digia.  For licensing terms and
 ** conditions see http://qt.digia.com/licensing.  For further information
 ** use the contact form at http://qt.digia.com/contact-us.
 **
 ** GNU Lesser General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU Lesser
 ** General Public License version 2.1 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.LGPL included in the
 ** packaging of this file.  Please review the following information to
 ** ensure the GNU Lesser General Public License version 2.1 requirements
 ** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 **
 ** In addition, as a special exception, Digia gives you certain additional
 ** rights.  These rights are described in the Digia Qt LGPL Exception
 ** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
 **
 ** GNU General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU
 ** General Public License version 3.0 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL included in the
 ** packaging of this file.  Please review the following information to
 ** ensure the GNU General Public License version 3.0 requirements will be
 ** met: http://www.gnu.org/copyleft/gpl.html.
 **
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

import QtQuick 1.1

Item {
    id: slider
    width: 400
    height: 16

    // value is read/write.
    property real value: 0
    property real maximum: 1
    property real minimum: 0
    property int xMax: sliderBackground.width - handle.width - 4
    property string fgColour1: "lightgray"
    property string fgColour2: "gray"
    property string bgColour1: "black"
    property string bgColour2: "black"
    property string activeColour: "red"
    property int slidePercentage: 1
    property string sliderText: ""
    property color sliderTextColor: "white"
    property real handleOpacity: 0.8
    property bool showAsPercent: true
    property real defaultValue: 0
    property real specialValue: -1
    property string specialValueText: ""
    property string suffixText: ""
    property string prefixText: ""

    onValueChanged: updatePos()
    onXMaxChanged: updatePos()
    onMinimumChanged: updatePos()
    onMaximumChanged: updatePos()
    Component.onCompleted: updatePos()

    function updatePos() {
        if ( value == minimum ) {
            handle.x = 20;
            return;
        }
        if ( maximum > minimum ) {
            var pos = (value - minimum) * slider.xMax / (maximum - minimum);
            pos = Math.min(pos, sliderBackground.width - handle.width - 4);
            pos = Math.max(pos, 2);
            handle.x = pos + 20;
        } else
            handle.x = 20;
    }

    function slide(change) {
        value = Math.max(minimum, value + (change / 100) * (maximum - minimum))
    }

    Rectangle {
        id: resetButton
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        border.width: slider.activeFocus ? 2 : 0
        border.color: parent.activeColour
        radius: 4
        width: 16
        x: 0
        y: 1
        smooth: true
        gradient: Gradient {
            GradientStop { position: 0.0; color: bgColour1 }
            GradientStop { position: 1.0; color: bgColour2 }
        }
        Image {
            anchors.fill: parent
            anchors.margins: 2
            source: "../images/x.png"
            fillMode: Image.PreserveAspectFit
            smooth: true
        }
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onClicked: slider.value = slider.defaultValue
            onEntered: resetButton.opacity = 0.7
            onExited: resetButton.opacity = 1
        }
    }

    Rectangle {
        id: sliderBackground
        anchors.left: resetButton.right
        anchors.leftMargin: 2
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        border.width: slider.activeFocus ? 2 : 0
        border.color: parent.activeColour
        radius: 4
        smooth: true
        gradient: Gradient {
            GradientStop { position: 0.0; color: bgColour1 }
            GradientStop { position: 1.0; color: bgColour2 }
        }
        Text {
            anchors.fill: parent
            anchors.leftMargin: 2
            smooth: true
            color: slider.sliderTextColor
            text: sliderText
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
        }
        Text {
            anchors.fill: parent
            anchors.rightMargin: 2
            smooth: true
            color: slider.sliderTextColor
            text: slider.specialValueText.length > 0 && slider.specialValue === Math.round(slider.value) ? slider.specialValueText : slider.showAsPercent ? Math.round(slider.value * 100) + "%" : slider.prefixText + Math.round(slider.value) + slider.suffixText
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignRight
        }
    }

    Rectangle {
        id: handle
        smooth: true
        y: 1
        width: 15
        height: slider.height - 2
        radius: 4
        opacity: slider.handleOpacity
        gradient: Gradient {
            GradientStop { position: 0.0; color: fgColour1 }
            GradientStop { position: 1.0; color: fgColour2 }
        }
        MouseArea {
            id: mouse
            anchors.fill: parent
            drag.target: parent
            drag.axis: Drag.XAxis
            drag.minimumX: 20
            drag.maximumX: slider.xMax + 20
            onPositionChanged: value = (maximum - minimum) * (handle.x - 20) / slider.xMax + minimum
            hoverEnabled: true
            onEntered: handle.opacity = 1.0
            onExited: handle.opacity = slider.handleOpacity
        }
    }

    Keys.onPressed: {
        switch ( event.key ) {
        case Qt.Key_Left: {
            slide(-slidePercentage)
            event.accepted = true;
            break;
        }
        case Qt.Key_Right: {
            slide(slidePercentage)
            event.accepted = true;
            break;
        }
        }
    }
}
