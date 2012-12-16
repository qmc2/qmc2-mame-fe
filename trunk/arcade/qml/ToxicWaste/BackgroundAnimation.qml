import QtQuick 1.1
import Qt.labs.particles 1.0
import "ToxicWaste.js" as ToxicWaste

Item {
    id: animationItem
    anchors.fill: parent
    property bool running: false

    function randomize(min, max) {
        return Math.floor(Math.random() * (max - min)) + min;
    }

    function randomizeFloat(min, max) {
        return Math.random() * (max - min) + min;
    }

    states: [
        State {
            name: "running"
            when: opacity > 0.0
            PropertyChanges { target: animationItem; running: true }
        },
        State {
            name: "stopped"
            when: opacity == 0.0
            PropertyChanges { target: animationItem; running: false }
        }
    ]

    Image {
        id: purpleBubble
        source: "images/purple_bubble.png"
        scale: ToxicWaste.scaleFactorX()
        x: randomize(0, toxicWasteMain.width - purpleBubble.width)
        y: randomize(0, toxicWasteMain.height - purpleBubble.height)
        smooth: true
        SequentialAnimation {
            id: purpleBubbleAnimation
            running: animationItem.running
            ParallelAnimation {
                SequentialAnimation {
                    NumberAnimation { id: pn1x; target: purpleBubble; property: "x"; easing.type: Easing.InOutSine; to: randomize(0, toxicWasteMain.width - purpleBubble.width); duration: randomize(3000, 8000) }
                    NumberAnimation { id: pn2x; target: purpleBubble; property: "x"; easing.type: Easing.InOutSine; to: randomize(0, toxicWasteMain.width - purpleBubble.width); duration: randomize(3000, 8000) }
                }
                SequentialAnimation {
                    NumberAnimation { id: pn1y; target: purpleBubble; property: "y"; easing.type: Easing.InOutSine; to: randomize(0, toxicWasteMain.height - purpleBubble.height); duration: randomize(3000, 8000) }
                    NumberAnimation { id: pn2y; target: purpleBubble; property: "y"; easing.type: Easing.InOutSine; to: randomize(0, toxicWasteMain.height - purpleBubble.height); duration: randomize(3000, 8000) }
                }
            }
            ScriptAction {
                script: {
                    pn1x.to = randomize(0, toxicWasteMain.width - purpleBubble.width);
                    pn1x.duration = randomize(3000, 8000);
                    pn2x.to = randomize(0, toxicWasteMain.width - purpleBubble.width);
                    pn2x.duration = randomize(3000, 8000);
                    pn1y.to = randomize(0, toxicWasteMain.height - purpleBubble.height);
                    pn1y.duration = randomize(3000, 8000);
                    pn2y.to = randomize(0, toxicWasteMain.height - purpleBubble.height);
                    pn2y.duration = randomize(3000, 8000);
                    purpleBubbleAnimation.restart();
                }
            }
        }
        Particles {
            y: purpleBubble.height/2
            x: purpleBubble.width/2
            source: "images/purple_bubble.png"
            scale: 0.5
            lifeSpan: 8000
            lifeSpanDeviation: 4000
            count: 5
            angle: 0
            angleDeviation: 360
            velocity: 30
            velocityDeviation: 10
            smooth: true
            ParticleMotionWander {
                xvariance: 30
                pace: 100
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
        SequentialAnimation {
            id: blueBubbleAnimation
            running: animationItem.running
            ParallelAnimation {
                SequentialAnimation {
                    NumberAnimation { id: bn1x; target: blueBubble; property: "x"; easing.type: Easing.InOutSine; to: randomize(0, toxicWasteMain.width - blueBubble.width); duration: randomize(3000, 8000) }
                    NumberAnimation { id: bn2x; target: blueBubble; property: "x"; easing.type: Easing.InOutSine; to: randomize(0, toxicWasteMain.width - blueBubble.width); duration: randomize(3000, 8000) }
                }
                SequentialAnimation {
                    NumberAnimation { id: bn1y; target: blueBubble; property: "y"; easing.type: Easing.InOutSine; to: randomize(0, toxicWasteMain.height - blueBubble.height); duration: randomize(3000, 8000) }
                    NumberAnimation { id: bn2y; target: blueBubble; property: "y"; easing.type: Easing.InOutSine; to: randomize(0, toxicWasteMain.height - blueBubble.height); duration: randomize(3000, 8000) }
                }
            }
            ScriptAction {
                script: {
                    bn1x.to = randomize(0, toxicWasteMain.width - blueBubble.width);
                    bn1x.duration = randomize(3000, 8000);
                    bn2x.to = randomize(0, toxicWasteMain.width - blueBubble.width);
                    bn2x.duration = randomize(3000, 8000);
                    bn1y.to = randomize(0, toxicWasteMain.height - blueBubble.height);
                    bn1y.duration = randomize(3000, 8000);
                    bn2y.to = randomize(0, toxicWasteMain.height - blueBubble.height);
                    bn2y.duration = randomize(3000, 8000);
                    blueBubbleAnimation.restart();
                }
            }
        }
        Particles {
            id: blueBubbleParticles
            y: blueBubble.height/2
            x: blueBubble.width/2
            source: "images/blue_bubble.png"
            scale: 0.5
            lifeSpan: 8000
            lifeSpanDeviation: 4000
            count: 5
            angle: 0
            angleDeviation: 360
            velocity: 30
            velocityDeviation: 10
            smooth: true
            ParticleMotionWander {
                xvariance: 30
                pace: 100
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
        SequentialAnimation {
            id: greenBubbleAnimation
            running: animationItem.running
            ParallelAnimation {
                SequentialAnimation {
                    NumberAnimation { id: gn1x; target: greenBubble; property: "x"; easing.type: Easing.InOutSine; to: randomize(0, toxicWasteMain.width - greenBubble.width); duration: randomize(3000, 8000) }
                    NumberAnimation { id: gn2x; target: greenBubble; property: "x"; easing.type: Easing.InOutSine; to: randomize(0, toxicWasteMain.width - greenBubble.width); duration: randomize(3000, 8000) }
                }
                SequentialAnimation {
                    NumberAnimation { id: gn1y; target: greenBubble; property: "y"; easing.type: Easing.InOutSine; to: randomize(0, toxicWasteMain.height - greenBubble.height); duration: randomize(3000, 8000) }
                    NumberAnimation { id: gn2y; target: greenBubble; property: "y"; easing.type: Easing.InOutSine; to: randomize(0, toxicWasteMain.height - greenBubble.height); duration: randomize(3000, 8000) }
                }
            }
            ScriptAction {
                script: {
                    gn1x.to = randomize(0, toxicWasteMain.width - greenBubble.width);
                    gn1x.duration = randomize(3000, 8000);
                    gn2x.to = randomize(0, toxicWasteMain.width - greenBubble.width);
                    gn2x.duration = randomize(3000, 8000);
                    gn1y.to = randomize(0, toxicWasteMain.height - greenBubble.height);
                    gn1y.duration = randomize(3000, 8000);
                    gn2y.to = randomize(0, toxicWasteMain.height - greenBubble.height);
                    gn2y.duration = randomize(3000, 8000);
                    greenBubbleAnimation.restart();
                }
            }
        }
        Particles {
            y: greenBubble.height/2
            x: greenBubble.width/2
            source: "images/green_bubble.png"
            scale: 0.5
            lifeSpan: 8000
            lifeSpanDeviation: 4000
            count: 5
            angle: 0
            angleDeviation: 360
            velocity: 30
            velocityDeviation: 10
            smooth: true
            ParticleMotionWander {
                xvariance: 30
                pace: 100
            }
        }
    }
}
