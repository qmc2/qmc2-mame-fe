import QtQuick 1.1
import "ToxicWaste.js" as ToxicWaste

Item {
    id: animationItem
    anchors.fill: parent
    property bool running: false

    function randomize(min, max) {
        return Math.floor(Math.random() * (max - min)) + min;
    }

    Image {
        id: purpleBubble
        source: "images/purple_bubble.png"
        scale: ToxicWaste.scaleFactorX()
        x: randomize(0, toxicWasteMain.width - purpleBubble.width)
        y: randomize(0, toxicWasteMain.height - purpleBubble.height)
        smooth: true
        SequentialAnimation on x {
            id: purpleBubbleAnimationX
            running: animationItem.running
            NumberAnimation { id: pn1x; easing.type: Easing.InOutBounce; to: randomize(0, toxicWasteMain.width - purpleBubble.width); duration: randomize(1000, 10000) }
            NumberAnimation { id: pn2x; easing.type: Easing.InOutBounce; to: randomize(0, toxicWasteMain.width - purpleBubble.width); duration: randomize(1000, 10000) }
            ScriptAction {
                script: {
                    pn1x.to = randomize(0, toxicWasteMain.width - purpleBubble.width);
                    pn1x.duration = randomize(1000, 10000);
                    pn2x.to = randomize(0, toxicWasteMain.width - purpleBubble.width);
                    pn2x.duration = randomize(1000, 10000);
                    purpleBubbleAnimationX.restart();
                }
            }
        }
        SequentialAnimation on y {
            id: purpleBubbleAnimationY
            running: animationItem.running
            NumberAnimation { id: pn1y; easing.type: Easing.InOutBounce; to: randomize(0, toxicWasteMain.height - purpleBubble.height); duration: randomize(1000, 10000) }
            NumberAnimation { id: pn2y; easing.type: Easing.InOutBounce; to: randomize(0, toxicWasteMain.height - purpleBubble.height); duration: randomize(1000, 10000) }
            ScriptAction {
                script: {
                    pn1y.to = randomize(0, toxicWasteMain.height - purpleBubble.height);
                    pn1y.duration = randomize(1000, 10000);
                    pn2y.to = randomize(0, toxicWasteMain.height - purpleBubble.height);
                    pn2y.duration = randomize(1000, 10000);
                    purpleBubbleAnimationY.restart();
                }
            }
        }
    }
    Image {
        id: blueBubble
        source: "images/blue_bubble.png"
        scale: ToxicWaste.scaleFactorX()
        x: randomize(0, toxicWasteMain.width - blueBubble.width)
        y: randomize(0, toxicWasteMain.height - blueBubble.height)
        smooth: true
        SequentialAnimation on x {
            id: blueBubbleAnimationX
            running: animationItem.running
            NumberAnimation { id: bn1x; easing.type: Easing.InOutQuad; to: randomize(0, toxicWasteMain.width - blueBubble.width); duration: randomize(1000, 10000) }
            NumberAnimation { id: bn2x; easing.type: Easing.InOutElastic; to: randomize(0, toxicWasteMain.width - blueBubble.width); duration: randomize(1000, 10000) }
            ScriptAction {
                script: {
                    bn1x.to = randomize(0, toxicWasteMain.width - blueBubble.width);
                    bn1x.duration = randomize(1000, 10000);
                    bn2x.to = randomize(0, toxicWasteMain.width - blueBubble.width);
                    bn2x.duration = randomize(1000, 10000);
                    blueBubbleAnimationX.restart();
                }
            }
        }
        SequentialAnimation on y {
            id: blueBubbleAnimationY
            running: animationItem.running
            NumberAnimation { id: bn1y; easing.type: Easing.InOutQuad; to: randomize(0, toxicWasteMain.height - blueBubble.height); duration: randomize(1000, 10000) }
            NumberAnimation { id: bn2y; easing.type: Easing.InOutElastic; to: randomize(0, toxicWasteMain.height - blueBubble.height); duration: randomize(1000, 10000) }
            ScriptAction {
                script: {
                    bn1y.to = randomize(0, toxicWasteMain.height - blueBubble.height);
                    bn1y.duration = randomize(1000, 10000);
                    bn2y.to = randomize(0, toxicWasteMain.height - blueBubble.height);
                    bn2y.duration = randomize(1000, 10000);
                    blueBubbleAnimationY.restart();
                }
            }
        }
    }
    Image {
        id: greenBubble
        source: "images/green_bubble.png"
        scale: ToxicWaste.scaleFactorX()
        x: randomize(0, toxicWasteMain.width - greenBubble.width)
        y: randomize(0, toxicWasteMain.height - greenBubble.height)
        smooth: true
        SequentialAnimation on x {
            id: greenBubbleAnimationX
            running: animationItem.running
            NumberAnimation { id: gn1x; easing.type: Easing.InOutBounce; to: randomize(0, toxicWasteMain.width - greenBubble.width); duration: randomize(1000, 10000) }
            NumberAnimation { id: gn2x; easing.type: Easing.InOutElastic; to: randomize(0, toxicWasteMain.width - greenBubble.width); duration: randomize(1000, 10000) }
            ScriptAction {
                script: {
                    gn1x.to = randomize(0, toxicWasteMain.width - greenBubble.width);
                    gn1x.duration = randomize(1000, 10000);
                    gn2x.to = randomize(0, toxicWasteMain.width - greenBubble.width);
                    gn2x.duration = randomize(1000, 10000);
                    greenBubbleAnimationX.restart();
                }
            }
        }
        SequentialAnimation on y {
            id: greenBubbleAnimationY
            running: animationItem.running
            NumberAnimation { id: gn1y; easing.type: Easing.InOutBounce; to: randomize(0, toxicWasteMain.height - greenBubble.height); duration: randomize(1000, 10000) }
            NumberAnimation { id: gn2y; easing.type: Easing.InOutElastic; to: randomize(0, toxicWasteMain.height - greenBubble.height); duration: randomize(1000, 10000) }
            ScriptAction {
                script: {
                    gn1y.to = randomize(0, toxicWasteMain.height - greenBubble.height);
                    gn1y.duration = randomize(1000, 10000);
                    gn2y.to = randomize(0, toxicWasteMain.height - greenBubble.height);
                    gn2y.duration = randomize(1000, 10000);
                    greenBubbleAnimationY.restart();
                }
            }
        }
    }
}
