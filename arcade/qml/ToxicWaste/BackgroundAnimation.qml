import QtQuick 1.1
import "ToxicWaste.js" as ToxicWaste

Item {
    id: mask
    anchors.fill: parent
    Rectangle {
        id: rectOne
        width: 50 * ToxicWaste.scaleFactor()
        height: parent.height
        y: 0
        x: -width
        opacity: 0.2
        gradient: Gradient {
            GradientStop { position: 0.0; color: "khaki" }
            GradientStop { position: 1.0; color: "black" }
        }
        SequentialAnimation on x {
            id: animOne
            NumberAnimation {
                to: toxicWasteMain.width
                duration: 2500
                easing.type: Easing.InOutCubic
            }
            NumberAnimation {
                to: -rectOne.width
                duration: 2500
                easing.type: Easing.InOutCubic
            }
            ScriptAction {
                script: {
                    animOne.stop();
                    animOne.start();
                }
            }
        }
    }
    Rectangle {
        id: rectTwo
        width: 100 * ToxicWaste.scaleFactor()
        height: parent.height
        y: 0
        x: -width
        opacity: 0.2
        gradient: Gradient {
            GradientStop { position: 0.0; color: "black" }
            GradientStop { position: 1.0; color: "khaki" }
        }
        SequentialAnimation on x {
            id: animTwo
            property int pauseDuration: 120
            PauseAnimation {
                duration: animTwo.pauseDuration
            }
            SequentialAnimation {
                NumberAnimation {
                    to: toxicWasteMain.width
                    duration: 2500
                    easing.type: Easing.InOutCubic
                }
                NumberAnimation {
                    to: -rectTwo.width
                    duration: 2500
                    easing.type: Easing.InOutCubic
                }
                ScriptAction {
                    script: {
                        animTwo.stop();
                        animTwo.pauseDuration = 0;
                        animTwo.start();
                    }
                }
            }
        }
    }
}
