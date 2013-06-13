import QtQuick 1.1

Rectangle {
    id: checkboxContainer
    property variant text
    property bool checked: false
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
        border.color: "black"
        border.width: 1
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
            source: "../images/checkmark.png"
            smooth: true
            anchors.fill: parent
            anchors.margins: 1
            fillMode: Image.PreserveAspectFit
            visible: checkboxContainer.checked
        }
        MouseArea {
            anchors.fill: parent
            onClicked: checkboxContainer.clicked()
        }
    }
    Text {
        id: checkboxText
        text: parent.text
        font.pixelSize: 12
        color: "white"
        anchors.left: checkboxMark.right
        anchors.leftMargin: 5
        smooth: true
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
