import QtQuick 1.1
import Qt.labs.shaders 1.0
import "ToxicWaste.js" as ToxicWaste

Rectangle {
    property int fps: 0
    property bool fpsVisible: true
    property bool showBackgroundAnimation: true
    Component.onCompleted: ToxicWaste.init()
    id: toxicWasteMain
    width: ToxicWaste.baseWidth()
    height: ToxicWaste.baseHeight()
    z: 0
    gradient: Gradient {
        GradientStop {
            position: 0.0
            color: "#3aa82b"
        }
        GradientStop {
            position: 0.75
            color: "#ffffff"
        }
        GradientStop {
            position: 1.0
            color: "#000000"
        }
    }
    ListView {
        id: gamelistView
        scale: ToxicWaste.scaleFactor()
        height: parent.height / scale - 20
        flickDeceleration: 1500
        maximumFlickVelocity: 3000
        snapMode: ListView.NoSnap
        interactive: true
        keyNavigationWraps: false
        anchors.verticalCenterOffset: 0
        anchors.horizontalCenterOffset: 0
        x: parent.width / scale / 2 - width / 2
        y: 10
        z: 3
        width: 304
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 10
        orientation: ListView.Vertical
        flickableDirection: Flickable.AutoFlickDirection
        smooth: true
        preferredHighlightBegin: 0
        preferredHighlightEnd: 64
        highlight: Rectangle {
            id: itemHighlighter
            smooth: true
            color: "#c0f08c"
            radius: 50
            border.color: "black"
            border.width: 2
            width: 304
            height: 64
            anchors.horizontalCenter: parent.horizontalCenter
            opacity: 0.7
            y: 10
            z: 0
        }
        highlightRangeMode: ListView.StrictlyEnforceRange
        highlightFollowsCurrentItem: true
        delegate: Item {
            property string gameId: id
            id: gamelistItemDelegate
            height: 64
            Image {
                id: gamelistItemImage
                fillMode: Image.PreserveAspectFit
                smooth: true
                source: "images/gameitem_bg.png"
                height: gamelistItemDelegate.height
                opacity: 0.7
                Text {
                    property bool fontResized: false
                    id: gamelistItemText
                    text: name
                    color: "black"
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.bold: true
                    font.italic: true
                    font.pixelSize: parent.height - 40
                    elide: Text.ElideMiddle
                }
                MouseArea {
                    id: gamelistItemMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton
                    onEntered: ToxicWaste.itemEntered(gamelistItemText, gamelistItemImage)
                    onExited: ToxicWaste.itemExited(gamelistItemText, gamelistItemImage)
                    onClicked: {
                        gamelistView.currentIndex = index;
                        ToxicWaste.itemClicked(gamelistItemDelegate.gameId, gamelistItemText.text);
                    }
                    onDoubleClicked: {
                        gamelistView.currentIndex = index;
                        ToxicWaste.itemClicked(gamelistItemDelegate.gameId, gamelistItemText.text);
                    }
                }
            }
            states: State {
                name: "active"
                StateChangeScript {
                    name: "stateChangeScript"
                    script: ToxicWaste.setCurrentItem(gamelistItemText, gamelistItemImage)
                }
            }
        }
        model: ListModel {
            id: gamelistModel
        }
    }
    Text {
        id: gamenameText
        anchors.top: parent.top
        anchors.topMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 5
        transformOrigin: Text.TopLeft
        color: "black"
        text: ""
        font.pixelSize: 10
        scale: ToxicWaste.scaleFactor()
        z: 3
    }
    Text {
        id: fpsText
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 5
        transformOrigin: Text.BottomLeft
        color: "lightgrey"
        text: qsTr("FPS") + ": " + parent.fps.toString()
        font.pixelSize: 10
        scale: ToxicWaste.scaleFactor()
        visible: parent.fpsVisible
        z: 3
    }
    Rectangle {
        id: confirmQuitDialog
        smooth: true
        radius: 10
        scale: ToxicWaste.scaleFactor()
        x: parent.width / 2 - width / 2
        y: parent.height / 2 - height / 2
        width: 200
        height: 100
        border.color: "black"
        border.width: 2
        color: "#c0f08c"
        opacity: 0.0
        state: "hidden"
        z: 4
        Text {
            text: qsTr("Really quit?")
            anchors.horizontalCenterOffset: 0
            anchors.top: parent.top
            anchors.topMargin: 5
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 12
        }
        states: [
            State {
                name: "hidden"
                PropertyChanges { target: confirmQuitDialog; opacity: 0.0 }
            },
            State {
                name: "shown"
                PropertyChanges { target: confirmQuitDialog; opacity: 1.0 }
            }
        ]
        transitions: Transition {
            from: "hidden"
            to: "shown"
            reversible: true
            PropertyAnimation { property: "opacity"; duration: 250 }
        }
    }
    BackgroundAnimation {
        id: backgroundAnim
        visible: parent.showBackgroundAnimation
        z: 2
    }
    ShaderEffectSource {
        id: effectSource
        anchors.fill: parent
        sourceItem: Image {
            source: "images/shadereffectsource.png"
            anchors.fill: parent
        }
        live: false
        hideSource: true
    }
    RadialWaveEffect {
        id: layer
        anchors.fill: parent;
        source: effectSource
        wave: 0.0
        waveOriginX: 0.5
        waveOriginY: 0.5
        waveWidth: 0.01
        z: 1
        NumberAnimation on wave {
            id: waveAnim
            running: true
            loops: Animation.Infinite
            easing.type: Easing.Linear
            from: 0.0000;
            to: 2.0000;
            duration: 3000
        }
    }
    focus: true
    Keys.onPressed: {
        switch ( event.key ) {
        case Qt.Key_Escape:
            if ( confirmQuitDialog.state == "hidden" )
                confirmQuitDialog.state = "shown";
            else
                confirmQuitDialog.state = "hidden";
            event.accepted = true;
            break;
        }
    }
    Keys.forwardTo: [gamelistView]
}
