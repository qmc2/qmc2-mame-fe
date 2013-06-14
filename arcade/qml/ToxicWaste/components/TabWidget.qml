import QtQuick 1.1

Item {
    id: tabWidget

    default property alias content: stack.children
    property int current: 0
    property string baseColor: parent.color
    property string activeBorderColor: "transparent"
    property int fontSize: 12

    onCurrentChanged: setOpacities()
    Component.onCompleted: setOpacities()

    function setOpacities() {
        for (var i = 0; i < stack.children.length; i++)
            stack.children[i].opacity = (i == current ? 1 : 0)
    }

    Row {
        id: header

        Repeater {
            model: stack.children.length
            delegate: Rectangle {
                width: tabWidget.width / stack.children.length - 2;
                x: width * index + 2
                height: 30
                color: tabWidget.current == index ? baseColor : "transparent"
                border.color: tabWidget.current == index ? activeBorderColor : baseColor
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
        anchors.topMargin: 2
        anchors.bottom: tabWidget.bottom
    }
}
