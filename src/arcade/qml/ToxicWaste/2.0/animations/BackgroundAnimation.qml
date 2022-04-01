import QtQuick 2.0
import QtQuick.Particles 2.0
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
        smooth: true
        SequentialAnimation {
            running: animationItem.running
            loops: Animation.Infinite
            PathAnimation {
                id: pPathAnim
                duration: randomize(3000, 8000)
                easing.type: Easing.InOutSine
                target: purpleBubble
                orientation: PathAnimation.Fixed
                anchorPoint: Qt.point(purpleBubble.width/2, purpleBubble.height/2)
                path: Path {
                    id: pPath
                    property alias cubicX: pPathCubic.x
                    property alias cubicY: pPathCubic.y
                    startX: randomize(purpleBubble.width, toxicWasteMain.width - purpleBubble.width)
                    startY: randomize(purpleBubble.height, toxicWasteMain.height - purpleBubble.height)
                    PathCubic {
                        id: pPathCubic
                        x: randomize(purpleBubble.width, toxicWasteMain.width - purpleBubble.width)
                        y: randomize(purpleBubble.height, toxicWasteMain.height - purpleBubble.height)
                        control1X: x
                        control1Y: purpleBubble.y
                        control2X: purpleBubble.x
                        control2Y: y
                    }
                }
            }
            ScriptAction {
                script: {
                    pPath.startX = pPathCubic.x;
                    pPath.startY = pPathCubic.y;
                    pPath.cubicX = randomize(purpleBubble.width, toxicWasteMain.width - purpleBubble.width);
                    pPath.cubicY = randomize(purpleBubble.height, toxicWasteMain.height - purpleBubble.height);
                    pPathAnim.duration = randomize(3000, 8000);
                }
            }
        }
        ParticleSystem {
            anchors.horizontalCenter: purpleBubble.horizontalCenter
            anchors.verticalCenter: purpleBubble.verticalCenter
            ImageParticle {
                source: purpleBubble.source
            }
            Emitter {
                size: purpleBubble.width/2 * ToxicWaste.scaleFactorX()
                sizeVariation: 5
                lifeSpan: 5000
                lifeSpanVariation: 3000
                maximumEmitted: 5
                velocity: AngleDirection {
                    angle: 0
                    angleVariation: 360
                    magnitude: 50
                }
            }
        }
    }
    Image {
        id: blueBubble
        source: "../images/blue_bubble.png"
        scale: ToxicWaste.scaleFactorX()
        smooth: true
        SequentialAnimation {
            running: animationItem.running
            loops: Animation.Infinite
            PathAnimation {
                id: bPathAnim
                duration: randomize(3000, 8000)
                easing.type: Easing.InOutSine
                target: blueBubble
                orientation: PathAnimation.Fixed
                anchorPoint: Qt.point(blueBubble.width/2, blueBubble.height/2)
                path: Path {
                    id: bPath
                    property alias cubicX: bPathCubic.x
                    property alias cubicY: bPathCubic.y
                    startX: randomize(blueBubble.width, toxicWasteMain.width - blueBubble.width)
                    startY: randomize(blueBubble.height, toxicWasteMain.height - blueBubble.height)
                    PathCubic {
                        id: bPathCubic
                        x: randomize(blueBubble.width, toxicWasteMain.width - blueBubble.width)
                        y: randomize(blueBubble.height, toxicWasteMain.height - blueBubble.height)
                        control1X: x
                        control1Y: blueBubble.y
                        control2X: blueBubble.x
                        control2Y: y
                    }
                }
            }
            ScriptAction {
                script: {
                    bPath.startX = bPathCubic.x;
                    bPath.startY = bPathCubic.y;
                    bPath.cubicX = randomize(blueBubble.width, toxicWasteMain.width - blueBubble.width);
                    bPath.cubicY = randomize(blueBubble.height, toxicWasteMain.height - blueBubble.height);
                    bPathAnim.duration = randomize(3000, 8000);
                }
            }
        }
        ParticleSystem {
            anchors.horizontalCenter: blueBubble.horizontalCenter
            anchors.verticalCenter: blueBubble.verticalCenter
            ImageParticle {
                source: blueBubble.source
            }
            Emitter {
                size: blueBubble.width/2 * ToxicWaste.scaleFactorX()
                sizeVariation: 5
                lifeSpan: 5000
                lifeSpanVariation: 3000
                maximumEmitted: 5
                velocity: AngleDirection {
                    angle: 0
                    angleVariation: 360
                    magnitude: 50
                }
            }
        }
    }
    Image {
        id: greenBubble
        source: "../images/green_bubble.png"
        scale: ToxicWaste.scaleFactorX()
        smooth: true
        SequentialAnimation {
            running: animationItem.running
            loops: Animation.Infinite
            PathAnimation {
                id: gPathAnim
                duration: randomize(3000, 8000)
                easing.type: Easing.InOutSine
                target: greenBubble
                orientation: PathAnimation.Fixed
                anchorPoint: Qt.point(greenBubble.width/2, greenBubble.height/2)
                path: Path {
                    id: gPath
                    property alias cubicX: gPathCubic.x
                    property alias cubicY: gPathCubic.y
                    startX: randomize(greenBubble.width, toxicWasteMain.width - greenBubble.width)
                    startY: randomize(greenBubble.height, toxicWasteMain.height - greenBubble.height)
                    PathCubic {
                        id: gPathCubic
                        x: randomize(greenBubble.width, toxicWasteMain.width - greenBubble.width)
                        y: randomize(greenBubble.height, toxicWasteMain.height - greenBubble.height)
                        control1X: x
                        control1Y: greenBubble.y
                        control2X: greenBubble.x
                        control2Y: y
                    }
                }
            }
            ScriptAction {
                script: {
                    gPath.startX = gPathCubic.x;
                    gPath.startY = gPathCubic.y;
                    gPath.cubicX = randomize(greenBubble.width, toxicWasteMain.width - greenBubble.width);
                    gPath.cubicY = randomize(greenBubble.height, toxicWasteMain.height - greenBubble.height);
                    gPathAnim.duration = randomize(3000, 8000);
                }
            }
        }
        ParticleSystem {
            anchors.horizontalCenter: greenBubble.horizontalCenter
            anchors.verticalCenter: greenBubble.verticalCenter
            ImageParticle {
                source: greenBubble.source
            }
            Emitter {
                size: greenBubble.width/2 * ToxicWaste.scaleFactorX()
                sizeVariation: 5
                lifeSpan: 5000
                lifeSpanVariation: 3000
                maximumEmitted: 5
                velocity: AngleDirection {
                    angle: 0
                    angleVariation: 360
                    magnitude: 50
                }
            }
        }
    }
}
