import QtQuick 2.0

Item {
    id: root
    property bool debug: false

    height: 12
    width: 100
    opacity: 1.0

    property alias text: textInput.text
    property alias inputWidth: inputBox.width
    property alias inputColour: textInput.color

    property string textPrefix: ""
    property string textSuffix: ""
    property int textSize: 10
    property string textColour: "white"
    property string activeColour: "blue"
    property real opacityDiff: 0.2
    property real resetOpacity: 0
    property real textOpacity: opacity - opacityDiff
    onOpacityChanged: { textOpacity: opacity - opacityDiff; }

    signal accepted();
    signal entered();
    signal clicked();
    Component.onCompleted: {
        mouseArea.entered.connect(entered);
        mouseArea.clicked.connect(clicked);
        textInput.accepted.connect(accepted);
        textPrefixMouseArea.clicked.connect(mouseArea.clicked);
        textInputMouseArea.clicked.connect(mouseArea.clicked);
        textSuffixMouseArea.clicked.connect(mouseArea.clicked);
    }

    onActiveFocusChanged: {
        debug && console.log("[inputitem] activeFocus: '" + activeFocus + "'");
        textInput.focus = true;
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
            textInput.focus = true;
        }
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

        MouseArea { id: textPrefixMouseArea }
    }
    Rectangle {
        id: inputBox
        height: parent.height + 1
        width: parent.width
        anchors.top: parent.top
        anchors.topMargin: 0
        anchors.left: textPrefix == "" ? parent.left : textPrefixText.right
        anchors.leftMargin: textPrefix == "" ? 0 : 5
        smooth: true
        color: "white"
        border.width: textInput.activeFocus ? 2 : 0
        border.color: activeColour

        TextInput {
            id: textInput
            anchors.verticalCenter: parent.verticalCenter
            anchors.topMargin: 2
            anchors.bottomMargin: 2
            anchors.left: parent.left
            anchors.leftMargin: 2
            anchors.right: parent.right
            anchors.rightMargin: 2
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: root.textSize
            autoScroll: true
            clip: true
            selectByMouse: true
            smooth: true

            cursorDelegate: Rectangle {
                color: "black"
                width: 1
                anchors.verticalCenter: parent.verticalCenter
                visible: parent.activeFocus
            }

            MouseArea { id: textInputMouseArea }
        }
    }
    Text {
        id: textSuffixText
        opacity: textOpacity
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 0
        anchors.left: inputBox.right
        anchors.leftMargin: 5
        text: parent.textSuffix
        font.pixelSize: parent.textSize
        color: parent.textColour
        verticalAlignment: Text.AlignVCenter
        smooth: true

        MouseArea { id: textSuffixMouseArea }
    }
}
