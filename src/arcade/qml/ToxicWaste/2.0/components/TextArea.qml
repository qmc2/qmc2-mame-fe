import QtQuick 2.0

Rectangle {
    id: container

    property string displayText: ""
    property real fontSize: 12
    property color fontColor: "white"
    property alias arrowIcon: downArrow.source

    Flickable {
        id: textView
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds
        contentHeight: textDisplay.height
        clip: true
        Text { // I would like to use a read-only TextEdit here as it's much faster on transforms, but with Qt 4.8.4 at least this doesn't trigger the linkActivated signal (only when readOnly == false)
            id: textDisplay
            text: container.displayText
            font.pointSize: container.fontSize
            color: container.fontColor
            wrapMode: Text.WordWrap
            smooth: true
            width: parent.width
            onLinkActivated: viewer.linkActivated(link)
        }
    }
    Image {
        id: upArrow
        anchors.top: parent.top
        anchors.topMargin: 2
        anchors.left: parent.right
        anchors.leftMargin: -10
        source: downArrow.source
        scale: -1
        smooth: true
        width: 8
        opacity: textView.contentY > 0 ? 0.33 : 0
        Behavior on opacity { NumberAnimation { duration: 200 } }
        fillMode: Image.PreserveAspectFit
    }
    Image {
        id: downArrow
        anchors.top: parent.bottom
        anchors.topMargin: -8
        anchors.left: parent.right
        anchors.leftMargin: -10
        smooth: true
        width: 8
        opacity: textView.contentY + container.height + 1 < textView.contentHeight ? 0.33 : 0
        Behavior on opacity { NumberAnimation { duration: 200 } }
        fillMode: Image.PreserveAspectFit
    }
}
