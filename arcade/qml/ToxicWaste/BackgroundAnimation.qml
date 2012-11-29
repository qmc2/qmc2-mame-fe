import QtQuick 1.1
import "ToxicWaste.js" as ToxicWaste

Item {
    id: mask
    anchors.fill: parent
    Rectangle {
        id: rectOne
        width: 50
        height: parent.height
        opacity: 0.2
        scale: ToxicWaste.scaleFactor()
        gradient: Gradient {
            GradientStop { position: 0.0; color: "red" }
            GradientStop { position: 0.5; color: "yellow" }
            GradientStop { position: 1.0; color: "green" }
        }
        SequentialAnimation on x {
            id: anim
            running: true
            loops: Animation.Infinite
            NumberAnimation {
                to: toxicWasteMain.width
                duration: 2000
                easing.type: Easing.InOutCubic
            }
            NumberAnimation {
                to: 0 - rectOne.width
                duration: 2000
                easing.type: Easing.InOutCubic
            }
        }
    }
    Rectangle {
        id: rectTwo
        width: 100
        height: parent.height
        opacity: 0.2
        scale: ToxicWaste.scaleFactor()
        gradient: Gradient {
            GradientStop { position: 0.0; color: "yellow" }
            GradientStop { position: 0.5; color: "green" }
            GradientStop { position: 1.0; color: "red" }
        }
        SequentialAnimation on x {
            PauseAnimation {
                duration: 100
            }
            SequentialAnimation {
                loops: Animation.Infinite
                NumberAnimation {
                    to: toxicWasteMain.width
                    duration: 2000
                    easing.type: Easing.InOutCubic
                }
                NumberAnimation {
                    to: 0 - rectTwo.width
                    duration: 2000
                    easing.type: Easing.InOutCubic
                }
            }
        }
    }
}
