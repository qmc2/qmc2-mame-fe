import QtQuick 1.1
import Qt.labs.particles 1.0
import "../ToxicWaste.js" as ToxicWaste

Item {
    id: animationItem
    property bool running: false
    function randomize(min, max) {
        return Math.floor(Math.random() * (max - min)) + min;
    }
    anchors.fill: parent
    onRunningChanged: {
        if ( running )
            viewer.log("ToxicWaste: " + qsTr("Starting animation"));
        else
            viewer.log("ToxicWaste: " + qsTr("Animation stopped"));
    }
    states: [
        State {
            name: "stopped"
            when: opacity == 0.0
            PropertyChanges { target: animationItem; running: false }
        },
        State {
            name: "running"
            when: opacity > 0.0
            PropertyChanges { target: animationItem; running: true }
        }
    ]
    Image {
        id: purpleBubble
        source: "../images/purple_bubble.png"
        scale: ToxicWaste.scaleFactorX()
        Component.onCompleted: {
            x = randomize(0, toxicWasteMain.width - purpleBubble.width)
            y = randomize(0, toxicWasteMain.height - purpleBubble.height)
        }
        smooth: true
        SequentialAnimation {
            id: purpleBubbleAnimation
            running: animationItem.running
            ParallelAnimation {
                NumberAnimation { id: px; target: purpleBubble; property: "x"; easing.type: Easing.InOutQuad; to: randomize(0, toxicWasteMain.width - purpleBubble.width); duration: randomize(3000, 8000) }
                NumberAnimation { id: py; target: purpleBubble; property: "y"; easing.type: Easing.InOutQuad; to: randomize(0, toxicWasteMain.height - purpleBubble.height); duration: randomize(3000, 8000) }
            }
            ScriptAction {
                script: {
                    px.to = randomize(0, toxicWasteMain.width - purpleBubble.width);
                    px.duration = randomize(3000, 8000);
                    py.to = randomize(0, toxicWasteMain.height - purpleBubble.height);
                    py.duration = randomize(3000, 8000);
                    purpleBubbleAnimation.restart();
                }
            }
        }
        Particles {
            id: purpleBubbleParticles
            y: purpleBubble.height/2
            x: purpleBubble.width/2
            source: "../images/purple_bubble.png"
            scale: 0.5
            lifeSpan: 8000
            lifeSpanDeviation: 4000
            count: animationItem.running ? 5 : 0
            emissionRate: animationItem.running ? -1 : 0
            angle: 0
            angleDeviation: 360
            velocity: 30
            velocityDeviation: 10
            fadeInDuration: 250
            fadeOutDuration: 500
            smooth: true
            z: 1
            ParticleMotionWander {
                xvariance: 30
                yvariance: 30
                pace: 100
            }
        }
    }
    Image {
        id: blueBubble
        source: "../images/blue_bubble.png"
        scale: ToxicWaste.scaleFactorX()
        Component.onCompleted: {
            x = randomize(0, toxicWasteMain.width - blueBubble.width)
            y = randomize(0, toxicWasteMain.height - blueBubble.height)
        }
        smooth: true
        SequentialAnimation {
            id: blueBubbleAnimation
            running: animationItem.running
            ParallelAnimation {
                NumberAnimation { id: bx; target: blueBubble; property: "x"; easing.type: Easing.InOutQuad; to: randomize(0, toxicWasteMain.width - blueBubble.width); duration: randomize(3000, 8000) }
                NumberAnimation { id: by; target: blueBubble; property: "y"; easing.type: Easing.InOutQuad; to: randomize(0, toxicWasteMain.height - blueBubble.height); duration: randomize(3000, 8000) }
            }
            ScriptAction {
                script: {
                    bx.to = randomize(0, toxicWasteMain.width - blueBubble.width);
                    bx.duration = randomize(3000, 8000);
                    by.to = randomize(0, toxicWasteMain.height - blueBubble.height);
                    by.duration = randomize(3000, 8000);
                    blueBubbleAnimation.restart();
                }
            }
        }
        Particles {
            id: blueBubbleParticles
            y: blueBubble.height/2
            x: blueBubble.width/2
            source: "../images/blue_bubble.png"
            scale: 0.5
            lifeSpan: 8000
            lifeSpanDeviation: 4000
            count: animationItem.running ? 5 : 0
            emissionRate: animationItem.running ? -1 : 0
            angle: 0
            angleDeviation: 360
            velocity: 30
            velocityDeviation: 10
            fadeInDuration: 250
            fadeOutDuration: 500
            smooth: true
            z: 1
            ParticleMotionWander {
                xvariance: 30
                yvariance: 30
                pace: 100
            }
        }
    }
    Image {
        id: greenBubble
        source: "../images/green_bubble.png"
        scale: ToxicWaste.scaleFactorX()
        Component.onCompleted: {
            x = randomize(0, toxicWasteMain.width - greenBubble.width)
            y = randomize(0, toxicWasteMain.height - greenBubble.height)
        }
        smooth: true
        SequentialAnimation {
            id: greenBubbleAnimation
            running: animationItem.running
            ParallelAnimation {
                NumberAnimation { id: gx; target: greenBubble; property: "x"; easing.type: Easing.InOutQuad; to: randomize(0, toxicWasteMain.width - greenBubble.width); duration: randomize(3000, 8000) }
                NumberAnimation { id: gy; target: greenBubble; property: "y"; easing.type: Easing.InOutQuad; to: randomize(0, toxicWasteMain.height - greenBubble.height); duration: randomize(3000, 8000) }
            }
            ScriptAction {
                script: {
                    gx.to = randomize(0, toxicWasteMain.width - greenBubble.width);
                    gx.duration = randomize(3000, 8000);
                    gy.to = randomize(0, toxicWasteMain.height - greenBubble.height);
                    gy.duration = randomize(3000, 8000);
                    greenBubbleAnimation.restart();
                }
            }
        }
        Particles {
            id: greenBubbleParticles
            y: greenBubble.height/2
            x: greenBubble.width/2
            source: "../images/green_bubble.png"
            scale: 0.5
            lifeSpan: 8000
            lifeSpanDeviation: 4000
            count: animationItem.running ? 5 : 0
            emissionRate: animationItem.running ? -1 : 0
            angle: 0
            angleDeviation: 360
            velocity: 30
            velocityDeviation: 10
            fadeInDuration: 250
            fadeOutDuration: 500
            smooth: true
            z: 1
            ParticleMotionWander {
                xvariance: 30
                yvariance: 30
                pace: 100
            }
        }
    }
}
