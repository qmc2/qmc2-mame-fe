import QtQuick 2.0

Item {
    id: root
    property bool debug: false

    height: 12
    width: 100
    opacity: 1.0

    property string image: ""
    property int imageWidth: -1
    property int imageRotation: 0
    property int cyclerWidth: -1
    property variant items: ["item1", "item2", "item3", "item4", "item5"]
    property string selectedItem: ""
    property string value: ""
    property string textPrefix: ""
    property string textSuffix: ""
    property int textSize: 10
    property string textColour: "white"
    property string activeColour: "blue"
    property int activeBorderWidth: 1
    property real opacityDiff: 0.2
    property real resetOpacity: 0
    property real textOpacity: opacity - opacityDiff
    onOpacityChanged: { textOpacity: opacity - opacityDiff; }

    signal select();
    signal entered();
    signal exited();
    signal clicked();
    Component.onCompleted: {
        mouseArea.entered.connect(entered);
        textPrefixMouseArea.entered.connect(mouseArea.entered);
        prevButtonMouseArea.entered.connect(mouseArea.entered);
        textMouseArea.entered.connect(mouseArea.entered);
        nextButtonMouseArea.entered.connect(mouseArea.entered);
        textSuffixMouseArea.entered.connect(mouseArea.entered);
        mouseArea.exited.connect(exited);
        textPrefixMouseArea.exited.connect(mouseArea.exited);
        prevButtonMouseArea.exited.connect(mouseArea.exited);
        textMouseArea.exited.connect(mouseArea.exited);
        nextButtonMouseArea.exited.connect(mouseArea.exited);
        textSuffixMouseArea.exited.connect(mouseArea.exited);
        mouseArea.clicked.connect(clicked);
        textMouseArea.clicked.connect(mouseArea.clicked);
        value = items[1];
    }

    onActiveFocusChanged: {
        debug && console.log("[cycleitem] activeFocus: '" + activeFocus + "'")
        textText.focus = true;
    }
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
            resetOpacity = textOpacity;
            if (textOpacity > 0 )
                textOpacity += opacityDiff;
        }
        onExited: {
            textOpacity = resetOpacity;
        }
        onClicked: {
            textText.focus = true;
            select();            
        }
    }

    // cycle items in a list, returning prev/next item given the offset direction
    // offset: integer, positive for next item, negative for previous item
    function cycle(offset) {
        var first;
        var current;
        var previous;
        var target;
        var set = false;
        var l;
        for (l in items) {
            current = items[l];
            if (set) {
                if (offset > 0) {
                    target = current;
                    break;
                }
            } else if (current == value) {
                set = true;
                if (offset < 0 && previous) {
                    target = previous;
                    break;
                }
            } else if (!first)
                first = current
            previous = current
        }
        if (set && !target) {
            if (offset > 0)
                target = first
            else if (offset < 0)
                target = current
        }
        debug && console.log("[cycle] value: '" + value + "', " +
                                     "offset: '" + offset + "', " +
                                     "target: '" + target + "'");
        return target;
    }

    Text {
        id: textPrefix
        opacity: textOpacity
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 0
        anchors.left: parent.left
        text: parent.textPrefix
        font.pixelSize: parent.textSize
        color: parent.textColour
        verticalAlignment: Text.AlignVCenter
        smooth: true
        MouseArea { id: textPrefixMouseArea }        
    }
    Rectangle {
        id: cycler
        anchors.left: textPrefix == "" ? parent.left : textPrefix.right
        anchors.leftMargin: textPrefix == "" ? 0 : 5
        height: parent.height
        width: cyclerWidth > -1 ? cyclerWidth : parent.width - textPrefix.paintedWidth - textSuffix.paintedWidth
        color: "transparent"
        Image {
            id: prevButton
            opacity: 0.75
            width: root.imageWidth > -1 ? root.imageWidth : parent.height
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            source: root.image
            rotation: root.imageRotation + 180
            fillMode: Image.PreserveAspectFit
            smooth: true
            MouseArea {
                id: prevButtonMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onContainsMouseChanged: containsMouse ? parent.opacity = 1.0 : parent.opacity = 0.75
                onClicked: { 
                    textText.focus = true;
                    value = cycle(-1);
                }
            }
            Keys.onPressed: {
                switch ( event.key ) {
                    case Qt.Key_Enter:
                    case Qt.Key_Return:
                    case Qt.Key_Space: {
                        value = cycle(-1);
                        event.accepted = true;
                        break;
                    }
                }
            }
        }
        Rectangle {
            anchors.left: prevButton.right
            anchors.leftMargin: 5
            anchors.right: nextButton.left
            anchors.rightMargin: 5
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            color: "transparent"

            Rectangle {
                id: borderTop
                anchors.top: parent.top
                anchors.topMargin: -2
                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.min(parent.width, textText.paintedWidth + 8)
                height: activeBorderWidth
                visible: textText.activeFocus ? true : false
                color: activeColour 
            }
            Text {
                id: textText
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 2
                anchors.right: parent.right
                text: value
                font.bold: selectedItem != "" && selectedItem == value ? true : false
                font.pixelSize: root.textSize
                color: selectedItem != "" && selectedItem == value ? root.activeColour : root.textColour
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                smooth: true
                MouseArea { id: textMouseArea }
                Keys.onPressed: {
                    switch ( event.key ) {
                        case Qt.Key_Left: {
                            value = cycle(-1);
                            event.accepted = true;
                            break;
                        }
                        case Qt.Key_Right: {
                            value = cycle(1);
                            event.accepted = true;
                            break;
                        }
                        case Qt.Key_Enter:
                        case Qt.Key_Return:
                        case Qt.Key_Space: {
                            select();
                            event.accepted = true;
                            break;
                        }
                    }
                }
            }
            Rectangle {
                id: borderBottom
                anchors.bottom: parent.bottom
                anchors.bottomMargin: -2
                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.min(parent.width, textText.paintedWidth + 8)
                height: activeBorderWidth
                visible: textText.activeFocus ? true : false
                color: activeColour 
            }
        }
        Image {
            id: nextButton
            opacity: 0.75
            width: root.imageWidth > -1 ? root.imageWidth : parent.height
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            source: root.image
            fillMode: Image.PreserveAspectFit
            smooth: true
            MouseArea {
                id: nextButtonMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onContainsMouseChanged: containsMouse ? parent.opacity = 1.0 : parent.opacity = 0.75
                onClicked: { 
                    textText.focus = true;
                    value = cycle(1);
                }
            }
            Keys.onPressed: {
                switch ( event.key ) {
                    case Qt.Key_Enter:
                    case Qt.Key_Return:
                    case Qt.Key_Space: {
                        value = cycle(1);
                        event.accepted = true;
                        break;
                    }
                }
            }
        }
    }
    Text {
        id: textSuffix
        opacity: textOpacity
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 0
        anchors.left: cycler.right
        anchors.leftMargin: 5
        text: parent.textSuffix
        font.pixelSize: parent.textSize
        color: parent.textColour
        verticalAlignment: Text.AlignVCenter
        smooth: true
        MouseArea { id: textSuffixMouseArea }        
    }
}
