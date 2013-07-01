import QtQuick 1.1

Item {
    id: sliderItem
    property bool debug: false

    height: 12
    width: 100
    opacity: 1.0

    property string textPrefix: ""
    property string textSuffix: ""
    property string textColour: "white"
    property int textSize: 10
    property alias sliderWidth: slider.width
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

    signal clicked();
    Component.onCompleted: {
        mouseArea.clicked.connect(clicked);
    }

    onActiveFocusChanged: {
        debug && console.log("[slideritem] activeFocus: '" + activeFocus + "'");
        slider.focus = true;
    }
    onClicked: {
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
    }
    Slider {
        id: slider
        property int index: prefsText.index + 5
        height: parent.height - 2
        anchors.verticalCenter: parent.verticalCenter
        anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
        anchors.left: textPrefix == "" ? parent.left : textPrefixText.right
        anchors.leftMargin: textPrefix == "" ? 0 : 5
        smooth: true
    }
    Text {
        id: textSuffixText
        opacity: textOpacity
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 0
        anchors.left: slider.right
        anchors.leftMargin: 5
        text: parent.textSuffix
        font.pixelSize: parent.textSize
        color: parent.textColour
        verticalAlignment: Text.AlignVCenter
        smooth: true
    }
}
