import QtQuick 2.0

Item {
    id: tabWidget

    default property alias content: stack.children
    property int current: 0
    property color baseColor: parent.color
    property color activeBorderColor: "transparent"
    property color inactiveBorderColor: "transparent"
    property color activeTextColor: "black"
    property color inactiveTextColor: "black"
    property int fontSize: 12
    property real scaleFactor: 1

    onCurrentChanged: setOpacities()
    Component.onCompleted: setOpacities()

    function pageId(childItem) {
        var pgId = -1;
        for (var i = 0; i < stack.children.length; i++) {
            if ( childItem === stack.children[i] ) {
                pgId = i;
                break;
            }
        }
        return pgId;
    }

    function setOpacities() {
        for (var i = 0; i < stack.children.length; i++)
            stack.children[i].opacity = (i == current ? 1: 0);
    }

    function tabHeaderWidth() {
        return (tabWidget.width - 4 * (stack.children.length - 1)) / stack.children.length;
    }

    function firstKeyNavItem() {
        return stack.children[current].firstKeyNavItem;
    }

    function lastKeyNavItem() {
        return stack.children[current].lastKeyNavItem;
    }

    function nextKeyNavItem(nextItem, page) {
        if ( pageId(page) !== current )
            return stack.children[current].firstKeyNavItem;
        else
            return nextItem;
    }

    function previousKeyNavItem(prevItem, page) {
        if ( pageId(page) !== current )
            return stack.children[current].lastKeyNavItem;
        else
            return prevItem;
    }

    Row {
        id: header

        Repeater {
            model: stack.children.length
            delegate: Rectangle {
                width: tabHeaderWidth()
                x: (tabHeaderWidth() + 4) * index
                height: tabWidget.fontSize + 6
                color: tabWidget.current == index ? baseColor : "transparent"
                border.color: tabWidget.current == index ? activeBorderColor : inactiveBorderColor
                border.width: 1
                radius: 5
                smooth: true
                Text {
                    horizontalAlignment: Qt.AlignHCenter
                    verticalAlignment: Qt.AlignVCenter
                    anchors.fill: parent
                    text: stack.children[index].title
                    elide: Text.ElideRight
                    font.bold: tabWidget.current == index
                    font.pixelSize: tabWidget.fontSize
                    color: tabWidget.current == index ? activeTextColor : inactiveTextColor
                    smooth: true
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: tabWidget.current = index
                }
            }
        }
    }

    Item {
        id: stack
        width: tabWidget.width
        anchors.top: header.bottom
        anchors.topMargin: 3
        anchors.bottom: tabWidget.bottom
    }
}
