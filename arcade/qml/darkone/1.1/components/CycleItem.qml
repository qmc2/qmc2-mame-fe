import QtQuick 1.1

Item {
    id: cycleItem
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
    property real opacityDiff: 0.2
    property real resetOpacity: 0
    property real textOpacity: opacity - opacityDiff
    onOpacityChanged: { textOpacity: opacity - opacityDiff; }

    signal clicked();
    Component.onCompleted: {
         mouseAreaText.clicked.connect(clicked);
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
        id: textPrefixText
        opacity: textOpacity
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 0
        anchors.left: parent.left
        text: parent.textPrefix
        font.pixelSize: parent.textSize
        color: parent.textColour
        verticalAlignment: Text.AlignVCenter
        smooth: true
    }
    Rectangle {
        id: cycler
        anchors.left: textPrefix == "" ? parent.left : textPrefixText.right
        anchors.leftMargin: textPrefix == "" ? 0 : 5
        height: parent.height
        width: cyclerWidth > -1 ? cyclerWidth : parent.width - textPrefixText.paintedWidth - textSuffixText.paintedWidth
        color: "transparent"
        Image {
            id: prevButton
            opacity: 0.75
            width: cycleItem.imageWidth > -1 ? cycleItem.imageWidth : parent.height
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            source: cycleItem.image
            rotation: cycleItem.imageRotation + 180
            fillMode: Image.PreserveAspectFit
            smooth: true
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onContainsMouseChanged: containsMouse ? parent.opacity = 1.0 : parent.opacity = 0.75
                onClicked: value = cycle(-1);
            }
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.right: KeyNavigation.tab
            KeyNavigation.tab: textText
            Keys.onPressed: {
                switch ( event.key ) {
                    case Qt.Key_Enter:
                    case Qt.Key_Return:
                    case Qt.Key_Space: {
                        cycle(-1);
                        event.accepted = true;
                        break;
                    }
                }
            }
        }
        Text {
            id: textText
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: prevButton.right
            anchors.leftMargin: 5
            anchors.right: nextButton.left
            anchors.rightMargin: 5
            text: value
            font.bold: selectedItem != "" && selectedItem == value ? true : false
//            font.pixelSize: selectedItem != "" && selectedItem == value ? cycleItem.textSize + 1 : cycleItem.textSize
            font.pixelSize: cycleItem.textSize
            color: selectedItem != "" && selectedItem == value ? cycleItem.activeColour : cycleItem.textColour
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            smooth: true
            MouseArea {
                id: mouseAreaText
                anchors.fill: parent
            }
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.left: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.right: KeyNavigation.backtab
            KeyNavigation.backtab: prevButton
            KeyNavigation.tab: nextButton
        }
        Image {
            id: nextButton
            opacity: 0.75
            width: cycleItem.imageWidth > -1 ? cycleItem.imageWidth : parent.height
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            source: cycleItem.image
            fillMode: Image.PreserveAspectFit
            smooth: true
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onContainsMouseChanged: containsMouse ? parent.opacity = 1.0 : parent.opacity = 0.75
                onClicked: value = cycle(1);
            }
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.left: KeyNavigation.backtab
            KeyNavigation.backtab: textText
            Keys.onPressed: {
                switch ( event.key ) {
                    case Qt.Key_Enter:
                    case Qt.Key_Return:
                    case Qt.Key_Space: {
                        cycle(1);
                        event.accepted = true;
                        break;
                    }
                }
            }
        }
    }
    Text {
        id: textSuffixText
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
    }
}
