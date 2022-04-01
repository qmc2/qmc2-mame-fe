import QtQuick 1.1;

Item {
    id: root
    property bool debug: false

    height: 12
    width: 50
    opacity: 1.0

    property alias exclusiveGroup: checkable.exclusiveGroup
    property alias checked: checkable.checked
    property string text: ""
    property int textSize: 10
    property string textColour: "white"
    property string activeColour: "blue"
    property real opacityDiff: 0.2
    property real opacityReset: 0
    property real textOpacity: opacity - opacityDiff
    property int heightDiff: 2
    property alias heightReset: root.height

    onActiveColourChanged: { checkOuter.border.color = activeFocus ? activeColour : "transparent"; }
    onCheckedChanged: { checkInner.opacity = checked ? 1.0 : 0.5 }

    signal entered();
    signal clicked();
    Component.onCompleted: {
        mouseArea.entered.connect(entered);
        mouseArea.clicked.connect(clicked);
        checkOuterMouseArea.clicked.connect(mouseArea.clicked);
        checkInnerMouseArea.clicked.connect(mouseArea.clicked);
        textTextMouseArea.clicked.connect(mouseArea.clicked);
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
            opacityReset = textOpacity;
            if (textOpacity > 0 )
                textOpacity += opacityDiff;
        }
        onExited: {
            textOpacity = opacityReset;
        }
        onClicked: { root.checked = true;
            debug && console.log("[checkItem] root.focus: '" + root.focus + "'," +
                                 "checkOuter.focus: '" + checkOuter.focus + "'," +
                                 "checkInner.focus: '" + checkInner.focus + "'," +
                                 "textText.focus: '" + textText.focus + "'");
            root.focus = true;
            debug && console.log("[checkItem] root.focus: '" + root.focus + "'," +
                                 "checkOuter.focus: '" + checkOuter.focus + "'," +
                                 "checkInner.focus: '" + checkInner.focus + "'," +
                                 "textText.focus: '" + textText.focus + "'"); }

    }
    onActiveFocusChanged: {
        debug && console.log("[checkItem] activeFocus: '" + activeFocus + "'");
        checkInner.height = activeFocus ? checkInner.height - heightDiff : heightReset;
        checkOuter.border.width = activeFocus ? 2 : 0;
        checkOuter.border.color = activeFocus ? activeColour : "transparent";
    }
    Keys.onPressed: {
        switch ( event.key ) {
        case Qt.Key_Enter:
        case Qt.Key_Return:
        case Qt.Key_Space: {
            checked = true;
            event.accepted = true;
            break;
        }
        }
    }

    Checkable {
        id: checkable
        enabled: true
    }
    Rectangle {
        id: checkOuter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 0
        anchors.left: parent.left
        anchors.leftMargin: 2
        height: parent.height
        width: height
        opacity: 1.0
        radius: height / 2
        color: "transparent"
        border.width: 2
        smooth: true
        Rectangle {
            id: checkInner
            opacity: 0.5
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: 0
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.horizontalCenterOffset: 0
            height: parent.height
            color: "white"
            width: height
            radius: height / 2
            smooth: true
            MouseArea { id: checkInnerMouseArea }
        }
        MouseArea { id: checkOuterMouseArea }
    }
    Text {
        id: textText
        opacity: textOpacity
        anchors.left: parent.left
        anchors.leftMargin: 18
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 0
        verticalAlignment: Text.AlignVCenter
        text: parent.text
        font.pixelSize: parent.textSize
        color: parent.textColour
        smooth: true
        MouseArea { id: textTextMouseArea }
    }
}
