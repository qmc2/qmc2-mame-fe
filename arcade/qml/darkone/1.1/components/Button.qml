import QtQuick 1.1

Item {
    id: root
    property variant text
    height: text.height + 10
    width: text.width + 20

    signal entered();
    signal clicked();
    Component.onCompleted: {
        mouseArea.entered.connect(entered);
        mouseArea.clicked.connect(clicked);
        containerMouseArea.clicked.connect(mouseArea.clicked);
        textMouseArea.clicked.connect(mouseArea.clicked);
    }

    onActiveFocusChanged: {
        activeFocus ? border.width = 2 : border.width = 1
    }
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
            gradientStop1.color = !mouseArea.pressed ? activePalette.button : activePalette.dark;
            gradientStop2.color = !mouseArea.pressed ? activePalette.light : activePalette.button;
        }
        onExited: {
            gradientStop1.color = !mouseArea.pressed ? activePalette.light : activePalette.button;
            gradientStop2.color = !mouseArea.pressed ? activePalette.button : activePalette.dark;
        }
        onClicked: {
            buttonContainer.clicked();
            gradientStop1.color = activePalette.light;
            gradientStop2.color = activePalette.button;
        }
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

    Rectangle {
        id: container
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
        MouseArea { id: containerMouseArea }
    }
    SystemPalette {
        id: activePalette
    }
    Text {
        id: text
        anchors.centerIn:parent
        text: parent.text
        color: activePalette.buttonText
        font.pixelSize: 12
        smooth: true
        MouseArea { id: textMouseArea }
    }
}
