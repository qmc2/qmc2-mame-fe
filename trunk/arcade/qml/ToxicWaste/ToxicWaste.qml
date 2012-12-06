import QtQuick 1.1
import Qt.labs.shaders 1.0
import "ToxicWaste.js" as ToxicWaste

Rectangle {
    property int fps: 0

    // restored properties
    property bool fpsVisible: false
    property bool showBackgroundAnimation: false
    property bool fullScreen: false
    property string version: ""

    // delayed init
    Timer {
        id: initTimer
        interval: 50
        running: false
        repeat: false
        onTriggered: ToxicWaste.init()
    }
    Component.onCompleted: initTimer.start()

    id: toxicWasteMain
    width: ToxicWaste.baseWidth()
    height: ToxicWaste.baseHeight()
    z: 0
    gradient: Gradient {
        GradientStop { position: 0.00; color: "#3aa82b" }
        GradientStop { position: 0.75; color: "#ffffff" }
        GradientStop { position: 1.00; color: "#000000" }
    }
    Image {
        id: toxicImage
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: "images/toxic.png"
        z: 2
        smooth: true
        opacity: 0.7
    }
    Rectangle {
        id: overlayRect
        height: parent.height
        width: parent.width/2
        z: 3
        anchors.top: parent.top
        anchors.topMargin: 10 * ToxicWaste.scaleFactorX()
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10 * ToxicWaste.scaleFactorX()
        anchors.left: parent.left
        anchors.leftMargin: 50 * ToxicWaste.scaleFactorX()
        color: "#00000000"
        Flipable {
            id: overlayFlip
            property bool flipped: false
            anchors.fill: parent
            front: Image {
                id: overlayImageFront
                source: "images/overlay.png"
                fillMode: Image.PreserveAspectFit
                width: 380
                scale: ToxicWaste.scaleFactorX()
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: ToxicWaste.overlayOffset(height)
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.horizontalCenterOffset: 0
                smooth: true
            }
            back: Rectangle {
                id: overlayRectBack
                anchors.centerIn: parent
                anchors.fill: parent
                border.color: "black"
                border.width: 2
                radius: 10
                smooth: true
                gradient: Gradient {
                    GradientStop { position: 0.00; color: "#ffffff" }
                    GradientStop { position: 0.50; color: "#c0f08c" }
                }
                Text {
                    id: title
                    text: qsTr("Let's flip :)")
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 10 * scale
                    font.pixelSize: 12
                    font.bold: true
                    scale: ToxicWaste.scaleFactorX()
                }
            }
            transform: Rotation {
                id: overlayRotation
                origin.x: overlayFlip.width/2
                origin.y: overlayFlip.height/2
                axis.x: 0; axis.y: 1; axis.z: 0
                angle: 0
            }
            states: State {
                name: "back"
                PropertyChanges { target: overlayRotation; angle: 180 }
                when: overlayFlip.flipped
            }
            transitions: Transition {
                NumberAnimation { target: overlayRotation; property: "angle"; duration: 500 }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: overlayFlip.flipped = !overlayFlip.flipped
            }
        }
    }
    ListView {
        id: gamelistView
        scale: ToxicWaste.scaleFactorX()
        flickDeceleration: 2000
        maximumFlickVelocity: 4000
        snapMode: ListView.NoSnap
        interactive: true
        keyNavigationWraps: false
        z: 3
        width: 280
        height: parent.height / scale - 20
        anchors.horizontalCenterOffset: 240 * scale
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10
        orientation: ListView.Vertical
        flickableDirection: Flickable.AutoFlickDirection
        smooth: true
        preferredHighlightBegin: 0
        preferredHighlightEnd: 0
        highlight: Rectangle {
            id: itemHighlighter
            smooth: true
            color: "white"
            radius: 10
            border.color: "black"
            border.width: 2
            width: 280
            height: 72
            opacity: 1.0
            z: 0
        }
        highlightRangeMode: ListView.StrictlyEnforceRange
        highlightFollowsCurrentItem: true
        delegate: Item {
            property string gameId: id
            id: gamelistItemDelegate
            width: 280
            height: 72
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
                radius: 10
                border.color: "black"
                border.width: 2
                Text {
                    property bool fontResized: false
                    id: gamelistItemText
                    text: model.modelData.description
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
                    onContainsMouseChanged: {
                        if ( mapToItem(menuAndStatusBar, mouseX, mouseY).y < 0 ) {
                            if ( containsMouse )
                                ToxicWaste.itemEntered(gamelistItemText, gamelistItemBackground);
                            else
                                ToxicWaste.itemExited(gamelistItemText, gamelistItemBackground);
                        }
                    }
                    onClicked: {
                        gamelistView.currentIndex = index;
                        ToxicWaste.itemClicked(gamelistItemText, gamelistItemBackground);
                    }
                }
            }
        }
        model: gameListModel
        function firstVisibleItem() { return indexAt(contentX + 10, contentY + 10); }
        function lastVisibleItem() { return indexAt(contentX + width - 10, contentY + height - 10); }
        function itemsPerPage() { return Math.floor(height / 82); }
        Keys.onPressed: {
            switch ( event.key ) {
            case Qt.Key_PageUp:
                if ( currentIndex - itemsPerPage() > 0 ) {
                    incrementCurrentIndex();
                    contentY = contentY - height + 70;
                    if ( currentIndex > 0 )
                        decrementCurrentIndex();
                    else {
                        contentY = 0;
                        currentIndex = 0;
                    }
                } else
                    currentIndex = 0;
                event.accepted = true;
                break;
            case Qt.Key_PageDown:
                if ( currentIndex + itemsPerPage() < gameListModelCount - 1 ) {
                    decrementCurrentIndex();
                    contentY = contentY + height - 70;
                    if ( currentIndex < gameListModelCount - 1 )
                        incrementCurrentIndex();
                    else {
                        contentY = contentHeight - 82;
                        currentIndex = gameListModelCount - 1;
                    }
                } else
                    currentIndex = gameListModelCount - 1;
                event.accepted = true;
                break;
            case Qt.Key_End:
                positionViewAtEnd();
                event.accepted = true;
                break;
            case Qt.Key_Home:
                positionViewAtBeginning();
                event.accepted = true;
                break;
            }
        }
    }
    Rectangle {
        id: confirmQuitDialog
        smooth: true
        radius: 10
        scale: ToxicWaste.scaleFactorX()
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
        MouseArea {
            anchors.fill: parent
        }
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
        scale: ToxicWaste.scaleFactorX()
        x: parent.width / 2 - width / 2
        y: parent.height / 2 - height / 2
        width: 228
        height: 126
        border.color: "black"
        border.width: 2
        color: "#c0f08c"
        opacity: 0.0
        state: "hidden"
        z: 4
        MouseArea {
            anchors.fill: parent
        }
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
            id: showBgAnimCheckBox
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
        CheckBox {
            id: showFpsCheckBox
            anchors.top: showBgAnimCheckBox.bottom
            anchors.topMargin: 10
            anchors.bottom: showBgAnimCheckBox.bottom
            anchors.bottomMargin: -26
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            checked: toxicWasteMain.fpsVisible
            text: qsTr("Show FPS counter?")
            onClicked: toxicWasteMain.fpsVisible = checked
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
        opacity: 0.5
        smooth: true
        scale: ToxicWaste.scaleFactorX()
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
    BackgroundAnimation {
        id: backgroundAnim
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        opacity: parent.showBackgroundAnimation ? 1.0 : 0.0
        onVisibleChanged: {
            if ( visible )
                opacity = 1.0
            else
                opacity = 0.0
        }
        Behavior on opacity {
            NumberAnimation { properties: "opacity"; duration: 1000 }
        }
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
        case Qt.Key_F11:
            fullScreen = !fullScreen;
            break;
        }
    }
    Keys.forwardTo: [gamelistView]
    onFullScreenChanged: {
        if ( !ToxicWaste.initializing ) {
            if ( fullScreen ) {
                viewer.switchToFullScreen();
                fullScreenToggleButton.state = "fullscreen";
            } else {
                viewer.switchToWindowed();
                fullScreenToggleButton.state = "windowed";
            }
        }
    }
}
