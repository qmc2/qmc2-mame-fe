import QtQuick 2.0

Item {
    id: root
    property bool debug: false

    height: 12
    width: 100
    opacity: 1.0

    property string textPrefix: ""
    property string textSuffix: ""
    property string textColour: "white"
    property int textSize: 10
    property int sliderWidth
    property int textPrefixWidth
    property int textSuffixWidth
    property alias activeColour: slider.activeColour
    property alias fgColour1: slider.fgColour1
    property alias fgColour2: slider.fgColour2
    property alias bgColour1: slider.bgColour1
    property alias bgColour2: slider.bgColour2
    property alias value: slider.value
    property alias maximum: slider.maximum
    property alias minimum: slider.minimum
    property alias slidePercentage: slider.slidePercentage
    property real opacityDiff: 0.2
    property real resetOpacity: 0
    property real textOpacity: opacity - opacityDiff

    onOpacityChanged: { textOpacity: opacity - opacityDiff; }

    signal entered();
    signal clicked();
    Component.onCompleted: {
        mouseArea.entered.connect(entered);
        mouseArea.clicked.connect(clicked);
        textPrefixMouseArea.clicked.connect(mouseArea.clicked);
        textSuffixMouseArea.clicked.connect(mouseArea.clicked);
    }

    onActiveFocusChanged: {
        debug && console.log("[slideritem] activeFocus: '" + activeFocus + "'");
        slider.focus = true;
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
            slider.focus = true;
        }
    }

    Text {
        id: textPrefix
        opacity: textOpacity
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 0
        anchors.left: parent.left
        text: parent.textPrefix
        font.pixelSize: parent.textSize
        color: parent.textColour
        verticalAlignment: Text.AlignVCenter
        smooth: true

        Component.onCompleted: { if (parent.textPrefixWidth)
                parent.textPrefixWidth; }

        MouseArea { id: textPrefixMouseArea }
    }
    Slider {
        id: slider
        height: parent.height + 2
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 0
        anchors.left: textPrefix == "" ? parent.left : textPrefix.right
        anchors.leftMargin: textPrefix == "" ? 0 : 5
        width: sliderWidth ? sliderWidth : parent.width - (textPrefix == "" ? 0 : 5 + (textPrefixWidth || textPrefix.paintedWidth)) - (textSuffix == "" ? 0 : 5 + (textSuffixWidth || textSuffix.paintedWidth))
    }
    Text {
        id: textSuffix
        opacity: textOpacity
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 0
        anchors.left: slider.right
        anchors.leftMargin: textSuffix == "" ? 0 : 5
        anchors.rightMargin: 5
        text: parent.textSuffix
        font.pixelSize: parent.textSize
        color: parent.textColour
        verticalAlignment: Text.AlignVCenter
        smooth: true

        Component.onCompleted: { if (parent.textSuffixWidth)
                parent.textSuffixWidth; }

        MouseArea { id: textSuffixMouseArea }
    }
}
