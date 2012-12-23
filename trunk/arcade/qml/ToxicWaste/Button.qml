import QtQuick 1.1

Rectangle {
    id: buttonContainer
    property variant text
    signal clicked
    height: text.height + 10
    width: text.width + 20
    border.width: 1
    radius: 5
    smooth: true
    gradient: Gradient {
        GradientStop {
            id: gradientStop1
            position: 0.0
            color: !mouseArea.pressed ? activePalette.light : activePalette.button
        }
        GradientStop {
            id: gradientStop2
            position: 1.0
            color: !mouseArea.pressed ? activePalette.button : activePalette.dark
        }
    }
    onActiveFocusChanged: activeFocus ? border.width = 2 : border.width = 1
    SystemPalette {
        id: activePalette
    }
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            buttonContainer.clicked();
            gradientStop1.color = activePalette.light;
            gradientStop2.color = activePalette.button;
        }
        onEntered: {
            gradientStop1.color = !mouseArea.pressed ? activePalette.button : activePalette.dark;
            gradientStop2.color = !mouseArea.pressed ? activePalette.light : activePalette.button;
        }
        onExited: {
            gradientStop1.color = !mouseArea.pressed ? activePalette.light : activePalette.button;
            gradientStop2.color = !mouseArea.pressed ? activePalette.button : activePalette.dark;
        }
    }
    Text {
        id: text
        anchors.centerIn:parent
        text: parent.text
        color: activePalette.buttonText
        font.pixelSize: 12
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
