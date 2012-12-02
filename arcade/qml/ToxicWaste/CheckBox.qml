import QtQuick 1.1

Rectangle {
    id: checkboxContainer
    property variant text
    property bool checked: false
    signal clicked
    smooth: true
    color: "#00000000"
    Rectangle {
        id: checkboxMark
        border.color: "black"
        border.width: 2
        smooth: true
        anchors.left: checkboxContainer.left
        anchors.leftMargin: 0
        color: "white"
        width: 16
        radius: 2
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        anchors.top: parent.top
        anchors.topMargin: 0
        Image {
            id: checkboxMarkImage
            source: "images/checkmark.png"
            smooth: true
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            visible: checkboxContainer.checked
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                checkboxContainer.checked = !checkboxContainer.checked
                checkboxContainer.clicked();
            }
        }
    }
    Text {
        id: checkboxText
        text: parent.text
        font.pixelSize: 12
        anchors.left: checkboxMark.right
        anchors.leftMargin: 5
    }
}
