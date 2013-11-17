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
        onClicked: { checked = true;
                     focus = true; }
    }
    onCheckedChanged: { checkInner.opacity = checked ? 1.0 : 0.5 }
    onActiveFocusChanged: {
        debug && console.log("[checkItem] activeFocus: '" + activeFocus + "'");
        checkInner.height = activeFocus ? checkInner.height - heightDiff : heightReset;
        checkOuter.border.width = activeFocus ? 2 : 0;
        checkOuter.border.color = activeFocus ? activeColour : "transparent";
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
        }
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
    }
    Keys.onPressed: {
        switch ( event.key ) {
            case Qt.Key_Space: {
                checked = true;
                event.accepted = true;
                break;
            }
        }
    }
}
