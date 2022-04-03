import QtQuick 1.1

Rectangle {
    id: checkboxContainer
    property variant text
    property bool checked: false
    property string textColor: "black"
    property string frameColor: "black"
    property color hoverColor: "#266294"
    signal clicked
    smooth: true
    color: "transparent"
    onActiveFocusChanged: {
        if ( activeFocus )
            checkboxMark.border.width = 2;
        else
            checkboxMark.border.width = 1;
    }
    onClicked: checked = !checked
    Rectangle {
        id: checkboxMark
        border.color: checkboxContainer.frameColor
        border.width: checkboxContainer.activeFocus ? 2 : 1
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
            source: "qrc:qml/darkone/2.0/images/checkmark.png"
            smooth: true
            anchors.fill: parent
            anchors.margins: 1
            fillMode: Image.PreserveAspectFit
            visible: checkboxContainer.checked
        }
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: checkboxContainer.clicked()
        }
        Behavior on border.color { ColorAnimation { duration: 50 } }
        states: [
            State {
                name: "hover"; when: mouseArea.containsMouse
                PropertyChanges { target: checkboxMark; border.width: checkboxContainer.activeFocus ? 2 : 1; border.color: checkboxContainer.hoverColor }
            },
            State {
                name: "nohover"; when: !mouseArea.containsMouse
                PropertyChanges { target: checkboxMark; border.width: checkboxContainer.activeFocus ? 2 : 1; border.color: checkboxContainer.frameColor }
            }
        ]
    }
    Text {
        id: checkboxText
        text: parent.text
        font.pixelSize: 12
        color: checkboxContainer.textColor
        anchors.left: checkboxMark.right
        anchors.leftMargin: 5
        anchors.right: parent.right
        smooth: true
        elide: Text.ElideRight
    }
    Keys.onPressed: {
        switch ( event.key ) {
        case Qt.Key_Enter:
        case Qt.Key_Return:
        case Qt.Key_Space:
            if ( !(event.modifiers & Qt.AltModifier) )
                clicked();
            break;
        }
    }
}
