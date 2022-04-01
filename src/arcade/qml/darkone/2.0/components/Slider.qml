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

/*
 * modified: Pete Beardmore
 * date: 2012, 2013
 */

import QtQuick 2.0

Item {
    id: root
    property bool debug: false

    // value is read/write.
    property real value: 1
    property real maximum: 1
    property real minimum: 1
    property int xMin: 2
    property int xMax: width - handle.width - 4 - activeFocusBorder
    property string fgColour1: "lightgray"
    property string fgColour2: "gray"
    property string bgColour1: "black"
    property string bgColour2: "black"
    property string activeColour: "red"
    property int slidePercentage: 1
    property int heightReset: root.height - 2 * activeFocusBorder
    property int activeFocusBorder: 2

    width: 400
    height: 16

    onValueChanged: updatePos();
    onXMaxChanged: updatePos();
    onMinimumChanged: updatePos();
    onActiveFocusChanged: {
        debug && console.log("[slider] root.activeFocus: '" + root.activeFocus + "'," +
                             "root.height: '" + root.height + "'," +
                             "root.heightReset: '" + root.heightReset + "'");
        updatePos();
    }

    function updatePos() {
        if (maximum > minimum) {
            var pos = root.xMin + ( ((root.xMax - root.xMin)/(maximum - minimum)) * (value - minimum) );
            pos = Math.min(pos, xMax);
            pos = Math.max(pos, xMin);
            handle.x = root.activeFocus ? pos + activeFocusBorder : pos;
        } else {
            handle.x = root.activeFocus ? root.xMin + activeFocusBorder : root.xMin;
        }
    }
    function slide(change) {
        value = Math.min(maximum, Math.max(minimum, value + (change / 100) * (maximum - minimum)))
    }

    Rectangle {
        id: sliderBackground
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: root.activeFocus ? 0 : activeFocusBorder
        anchors.right: parent.right
        anchors.rightMargin: root.activeFocus ? 0 : activeFocusBorder
        height: root.activeFocus ? root.height : root.heightReset
        width: root.activeFocus ? root.width : root.width - 4
        radius: root.activeFocus ? 3 + activeFocusBorder : 3
        smooth: true
        border.width: root.activeFocus ? activeFocusBorder : 0
        border.color: parent.activeColour
        gradient: Gradient {
            GradientStop { position: 0.0; color: bgColour1 }
            GradientStop { position: 1.0; color: bgColour2 }
        }

        Rectangle {
            id: handle;
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: 0
            height: root.heightReset - 2
            width: 15
            radius: 4
            smooth: true
            gradient: Gradient {
                GradientStop { position: 0.0; color: fgColour1 }
                GradientStop { position: 1.0; color: fgColour2 }
            }
            MouseArea {
                id: mouse
                anchors.fill: parent; drag.target: parent
                drag.axis: Drag.XAxis
                drag.minimumX: root.activeFocus ? root.xMin + activeFocusBorder : root.xMin
                drag.maximumX: root.activeFocus ? root.xMax + activeFocusBorder : root.xMax
                onPositionChanged: { value = minimum + ( (((root.activeFocus ? handle.x - activeFocusBorder : handle.x ) - root.xMin) / (root.xMax - root.xMin)) * (maximum - minimum) ) }
            }
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
