import QtQuick 1.1
import Qt.labs.shaders 1.0
import "ToxicWaste.js" as ToxicWaste

Rectangle {
    property int fps: 0
    property bool fpsVisible: true
    property bool showBackgroundAnimation: true
    property bool fullScreen: false
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
            color: "white"
            radius: 50
            border.color: "black"
            border.width: 2
            width: 304
            height: 64
            anchors.horizontalCenter: parent.horizontalCenter
            opacity: 1.0
            y: 10
            z: 0
        }
        highlightRangeMode: ListView.StrictlyEnforceRange
        highlightFollowsCurrentItem: true
        delegate: Item {
            property string gameId: id
            id: gamelistItemDelegate
            width: 304
            height: 64
            Rectangle {
                id: gamelistItemBackground
                smooth: true
                anchors.fill: parent
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "lightgrey" }
                    GradientStop { position: 0.5; color: "white" }
                    GradientStop { position: 1.0; color: "lightgrey" }
                }
                opacity: 0.7
                radius: 50
                border.color: "black"
                border.width: 2
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
                    onEntered: {
                        if ( confirmQuitDialog.state == "hidden" && preferencesDialog.state == "hidden" )
                            ToxicWaste.itemEntered(gamelistItemText, gamelistItemBackground);
                    }
                    onExited: {
                        if ( confirmQuitDialog.state == "hidden" && preferencesDialog.state == "hidden" )
                            ToxicWaste.itemExited(gamelistItemText, gamelistItemBackground);
                    }
                    onClicked: {
                        gamelistView.currentIndex = index;
                        ToxicWaste.itemClicked(gamelistItemText, gamelistItemBackground);
                    }
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
    Rectangle {
        id: confirmQuitDialog
        smooth: true
        radius: 10
        scale: ToxicWaste.scaleFactor()
        x: parent.width / 2 - width / 2
        y: parent.height / 2 - height / 2
        width: 120
        height: 80
        border.color: "black"
        border.width: 2
        color: "#c0f08c"
        opacity: 0.0
        state: "hidden"
        z: 5
        Text {
            text: qsTr("Really quit?")
            anchors.top: parent.top
            anchors.topMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.horizontalCenterOffset: 0
            font.pixelSize: 12
        }
        Row {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            anchors.horizontalCenterOffset: 0
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10
            Button {
                text: qsTr("Yes")
                onClicked: Qt.quit()
            }
            Button {
                text: qsTr("No")
                onClicked: confirmQuitDialog.state = "hidden"
            }
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
    Rectangle {
        id: preferencesDialog
        smooth: true
        radius: 10
        scale: ToxicWaste.scaleFactor()
        x: parent.width / 2 - width / 2
        y: parent.height / 2 - height / 2
        width: 228
        height: 100
        border.color: "black"
        border.width: 2
        color: "#c0f08c"
        opacity: 0.0
        state: "hidden"
        z: 4
        Text {
            id: headerText
            text: qsTr("Preferences")
            anchors.top: parent.top
            anchors.topMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.horizontalCenterOffset: 0
            font.pixelSize: 12
            font.bold: true
        }
        CheckBox {
            anchors.top: headerText.bottom
            anchors.topMargin: 10
            anchors.bottom: headerText.bottom
            anchors.bottomMargin: -26
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            checked: toxicWasteMain.showBackgroundAnimation
            text: qsTr("Show background animation?")
            onClicked: toxicWasteMain.showBackgroundAnimation = checked
        }
        Button {
            id: okButton
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            anchors.horizontalCenterOffset: 0
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Close")
            onClicked: preferencesDialog.state = "hidden"
        }
        states: [
            State {
                name: "hidden"
                PropertyChanges { target: preferencesDialog; opacity: 0.0 }
            },
            State {
                name: "shown"
                PropertyChanges { target: preferencesDialog; opacity: 1.0 }
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
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
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
    Rectangle {
        id: menuAndStatusBar
        x: 0
        z: 4
        width: 800
        height: 24
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        transformOrigin: Rectangle.Bottom
        opacity: 0.3
        smooth: true
        scale: ToxicWaste.scaleFactor()
        gradient: Gradient {
            GradientStop { position: 0.0; color: "lightgrey" }
            GradientStop { position: 1.0; color: "black" }
        }
        Text {
            id: fpsText
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 5
            anchors.left: parent.left
            anchors.leftMargin: 5
            color: "white"
            text: qsTr("FPS") + ": " + toxicWasteMain.fps.toString()
            visible: toxicWasteMain.fpsVisible
        }
        Image {
            id: exitButton
            anchors.top: parent.top
            anchors.topMargin: 5
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 5
            anchors.right: parent.right
            anchors.rightMargin: 5
            source: "images/exit.png"
            smooth: true
            fillMode: Image.PreserveAspectFit
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: parent.opacity = 0.5
                onExited: parent.opacity = 1.0
                onClicked: { parent.opacity = 1.0; confirmQuitDialog.state = "shown"; }
            }
        }
        Image {
            id: fullScreenToggleButton
            anchors.top: parent.top
            anchors.topMargin: 5
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 5
            anchors.right: exitButton.left
            anchors.rightMargin: 5
            source: "images/fullscreen.png"
            state: toxicWasteMain.fullScreen ? "fullscreen" : "windowed"
            smooth: true
            fillMode: Image.PreserveAspectFit
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: parent.opacity = 0.5
                onExited: parent.opacity = 1.0
                onClicked: {
                    if ( fullScreenToggleButton.state == "windowed" ) {
                        fullScreenToggleButton.state = "fullscreen"
                        toxicWasteMain.fullScreen = true;
                    } else {
                        fullScreenToggleButton.state = "windowed"
                        toxicWasteMain.fullScreen = false;
                    }
                    parent.opacity = 1.0;
                }
            }
            states: [
                State {
                    name: "fullscreen"
                    PropertyChanges { target: fullScreenToggleButton; source: "images/windowed.png" }
                },
                State {
                    name: "windowed"
                    PropertyChanges { target: fullScreenToggleButton; source: "images/fullscreen.png" }
                }
            ]
        }
        Image {
            id: preferencesButton
            anchors.top: parent.top
            anchors.topMargin: 5
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 5
            anchors.right: fullScreenToggleButton.left
            anchors.rightMargin: 5
            source: "images/preferences.png"
            smooth: true
            fillMode: Image.PreserveAspectFit
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: parent.opacity = 0.5
                onExited: parent.opacity = 1.0
                onClicked: { parent.opacity = 1.0; preferencesDialog.state = "shown"; }
            }
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
    onFullScreenChanged: {
        if ( fullScreen )
            viewer.showFullScreen();
        else
            viewer.showNormal();
    }
}
