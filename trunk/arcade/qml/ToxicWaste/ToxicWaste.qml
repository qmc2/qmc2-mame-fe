import QtQuick 1.1
import Qt.labs.shaders 1.0
import "ToxicWaste.js" as ToxicWaste

Rectangle {
    id: toxicWasteMain

    property int fps: 0
    property bool ignoreLaunch: false

    // restored properties
    property bool fpsVisible: false
    property bool showBackgroundAnimation: false
    property bool animateInForeground: false
    property bool fullScreen: false
    property string secondaryImageType: "preview"
    property bool cabinetFlipped: false
    property int lastIndex: 0
    property bool menuHidden: false
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

    Timer {
        id: resetIgnoreLaunchTimer
        interval: 100
        running: false
        repeat: false
        onTriggered: toxicWasteMain.ignoreLaunch = false
    }

    width: ToxicWaste.baseWidth
    height: ToxicWaste.baseHeight
    z: 0
    Image {
       id: backgroundImage
       anchors.fill: parent
       fillMode: Image.PreserveAspectFit
       source: "images/shadereffectsource.png"
       smooth: true
       z: 0
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
    Item {
        id: overlayItem
        z: 3
        anchors.top: parent.top
        anchors.topMargin: 10 * ToxicWaste.scaleFactorX()
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10 * ToxicWaste.scaleFactorX()
        anchors.left: parent.left
        anchors.leftMargin: 20 * ToxicWaste.scaleFactorX()
        anchors.right: parent.right
        anchors.rightMargin: 320 * ToxicWaste.scaleFactorX()
        Flipable {
            id: overlayFlip
            anchors.fill: parent
            front: Item {
                anchors.fill: parent
                Image {
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
                Rectangle {
                    id: previewRect
                    width: 230
                    height: 170
                    opacity: 1.0
                    anchors.verticalCenter: overlayImageFront.verticalCenter
                    anchors.verticalCenterOffset: -157 * ToxicWaste.scaleFactorX()
                    anchors.horizontalCenter: overlayImageFront.horizontalCenter
                    anchors.horizontalCenterOffset: 0
                    scale: ToxicWaste.scaleFactorX()
                    color: "#202020"
                    smooth: true
                    z: -1
                    Image {
                        id: previewImage
                        source: "image://qmc2/prv/" + gameListModel[gamelistView.currentIndex].id
                        smooth: true
                        anchors.fill: parent
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectFit
                    }
                }
            }
            back: Rectangle {
                id: overlayRectBack
                anchors.fill: parent
                border.color: "black"
                border.width: 2
                radius: 4
                smooth: true
                gradient: Gradient {
                    GradientStop { position: 0.00; color: "#ffffff" }
                    GradientStop { position: 0.50; color: "#007bff" }
                }
                Text {
                    id: itemDescription
                    text: ToxicWaste.gameCardHeader()
                    font.pixelSize: 12 * ToxicWaste.scaleFactorX()
                    anchors.top: parent.top
                    anchors.topMargin: 10 * ToxicWaste.scaleFactorX()
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 76 * ToxicWaste.scaleFactorX()
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.WordWrap
                }
                Rectangle {
                    id: imageViewerRect
                    anchors.top: itemDescription.bottom
                    anchors.topMargin: 10 * ToxicWaste.scaleFactorX()
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 400 * ToxicWaste.scaleFactorX()
                    height: 300 * ToxicWaste.scaleFactorX()
                    smooth: true
                    radius: 10
                    color: "#202020"
                    border.color: "black"
                    border.width: 2
                    Image {
                        id: imageViewer
                        source: ToxicWaste.imageUrl(toxicWasteMain.secondaryImageType)
                        smooth: true
                        anchors.fill: parent
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectFit
                    }
                    Rectangle {
                        id: itemTypeSelector
                        gradient: Gradient {
                            GradientStop { position: 0.00; color: "black" }
                            GradientStop { position: 0.75; color: "white" }
                            GradientStop { position: 1.00; color: "black" }
                        }
                        radius: 15 * ToxicWaste.scaleFactorX()
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: imageViewer.bottom
                        anchors.topMargin: 10 * ToxicWaste.scaleFactorX()
                        height: 30 * ToxicWaste.scaleFactorX()
                        width: 200 * ToxicWaste.scaleFactorX()
                        smooth: true
                        Text {
                            id: imageTypeText
                            text: ToxicWaste.gameImageType(toxicWasteMain.secondaryImageType)
                            color: "black"
                            font.bold: true
                            font.pixelSize: 12 * ToxicWaste.scaleFactorX()
                            anchors.fill: parent
                            anchors.centerIn: parent
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            smooth: true
                        }
                        Image {
                            id: nextImageButton
                            opacity: 0.5
                            source: "images/arrow.png"
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.horizontalCenterOffset: 100 * ToxicWaste.scaleFactorX() - width/2
                            anchors.verticalCenter: parent.verticalCenter
                            height: parent.height - 2
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                onContainsMouseChanged: containsMouse ? parent.opacity = 1.0 : parent.opacity = 0.5
                                onClicked: {
                                    toxicWasteMain.secondaryImageType = ToxicWaste.nextImageType(toxicWasteMain.secondaryImageType);
                                    searchTextInput.focus = false;
                                }
                            }
                        }
                        Image {
                            id: previousImageButton
                            opacity: 0.5
                            source: "images/arrow.png"
                            mirror: true
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.horizontalCenterOffset: -100 * ToxicWaste.scaleFactorX() + width/2
                            anchors.verticalCenter: parent.verticalCenter
                            height: parent.height - 2
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                onContainsMouseChanged: containsMouse ? parent.opacity = 1.0 : parent.opacity = 0.5
                                onClicked: {
                                    toxicWasteMain.secondaryImageType = ToxicWaste.previousImageType(toxicWasteMain.secondaryImageType);
                                    searchTextInput.focus = false;
                                }
                            }
                        }
                    }
                    Rectangle {
                        anchors.fill: imageViewer
                        color: "transparent"
                        border.color: "black"
                        border.width: 4 * ToxicWaste.scaleFactorX()
                        radius: 4 * ToxicWaste.scaleFactorX()
                    }
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
                when: toxicWasteMain.cabinetFlipped
            }
            transitions: Transition {
                NumberAnimation { target: overlayRotation; property: "angle"; duration: 500 }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    toxicWasteMain.cabinetFlipped = !toxicWasteMain.cabinetFlipped;
                    searchTextInput.focus = false;
                }
                z: -1
            }
        }
    }
    Rectangle {
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 29 * ToxicWaste.scaleFactorX() - toxicWasteMain.height/2
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 29 * ToxicWaste.scaleFactorX() - toxicWasteMain.width/2
        scale: ToxicWaste.scaleFactorX()
        width: 48
        height: 48
        z: 4
        color: "darkgrey"
        radius: 26
        smooth: true
        Image {
            id: launchButton
            source: ToxicWaste.launchButtonSource()
            opacity: 0.8
            smooth: true
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            z: 4
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: parent.opacity = 1.0
                onExited: parent.opacity = 0.8
                onClicked: {
                    viewer.launchEmulator(gameListModel[gamelistView.currentIndex].id);
                    searchTextInput.focus = false;
                }
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
        currentIndex: toxicWasteMain.lastIndex
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
                opacity: 0.8
                radius: 10
                border.color: "black"
                border.width: 2
                Text {
                    property bool fontResized: false
                    id: gamelistItemText
                    text: model.modelData.description
                    color: "black"
                    font.bold: true
                    font.italic: true
                    font.pixelSize: parent.height / 3
                    elide: Text.ElideRight
                    wrapMode: Text.WordWrap
                    anchors.fill: parent
                    anchors.margins: 10
                    smooth: true
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
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
                    onDoubleClicked: {
                        gamelistView.currentIndex = index;
                        ToxicWaste.itemClicked(gamelistItemText, gamelistItemBackground);
                        searchTextInput.focus = false;
                        launchButton.opacity = 1.0;
                        viewer.launchEmulator(gameListModel[gamelistView.currentIndex].id);
                        launchButtonFlashTimer.start();
                    }
                    onClicked: {
                        gamelistView.currentIndex = index;
                        ToxicWaste.itemClicked(gamelistItemText, gamelistItemBackground);
                        searchTextInput.focus = false;
                    }
                }
            }
        }
        model: gameListModel
        function firstVisibleItem() { return indexAt(contentX + 10, contentY + 10); }
        function lastVisibleItem() { return indexAt(contentX + width - 10, contentY + height - 10); }
        function itemsPerPage() { return Math.floor(height / 82); }
        Timer {
            id: launchButtonFlashTimer
            running: false
            onTriggered: launchButton.opacity = 0.8
            interval: 100
        }
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
            case Qt.Key_Enter:
            case Qt.Key_Return:
                if ( !searchTextInput.focus && !(event.modifiers & Qt.AltModifier) && !toxicWasteMain.ignoreLaunch ) {
                    launchButton.opacity = 1.0;
                    viewer.launchEmulator(gameListModel[gamelistView.currentIndex].id);
                    launchButtonFlashTimer.start();
                }
                break;
            }
        }
        onCurrentIndexChanged: toxicWasteMain.lastIndex = currentIndex;
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
        color: "#007bff"
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
            color: "white"
        }
        Row {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            anchors.horizontalCenterOffset: 0
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10
            Button {
                id: buttonYes
                text: qsTr("Yes")
                onClicked: {
                    toxicWasteMain.ignoreLaunch = true;
                    Qt.quit();
                }
                onFocusChanged: {
                    if ( !focus )
                        toxicWasteMain.focus = true;
                }
                KeyNavigation.tab: buttonNo
                KeyNavigation.backtab: buttonNo
                KeyNavigation.left: buttonNo
                KeyNavigation.right: buttonNo
            }
            Button {
                id: buttonNo
                text: qsTr("No")
                onClicked: {
                    toxicWasteMain.ignoreLaunch = true;
                    confirmQuitDialog.state = "hidden";
                    resetIgnoreLaunchTimer.restart();
                }
                onFocusChanged: {
                    if ( !focus )
                        toxicWasteMain.focus = true;
                }
                KeyNavigation.tab: buttonYes
                KeyNavigation.backtab: buttonYes
                KeyNavigation.left: buttonYes
                KeyNavigation.right: buttonYes
            }
        }
        onStateChanged: {
            if ( state == "shown" )
                buttonNo.focus = true;
            else {
                buttonNo.focus = false;
                buttonYes.focus = false;
                if ( preferencesDialog.state == "shown" )
                    closeButton.focus = true;
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
        onOpacityChanged: {
            if ( opacity > 0 )
                searchTextInput.focus = false;
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
        height: 152
        border.color: "black"
        border.width: 2
        color: "#007bff"
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
            color: "white"
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
            onClicked: {
                toxicWasteMain.showBackgroundAnimation = checked;
                toxicWasteMain.ignoreLaunch = true;
                resetIgnoreLaunchTimer.restart();
            }
            onFocusChanged: {
                if ( !focus )
                    toxicWasteMain.focus = true;
            }
            KeyNavigation.tab: animInFgCheckBox
            KeyNavigation.backtab: closeButton
            KeyNavigation.right: animInFgCheckBox
            KeyNavigation.left: closeButton
        }
        CheckBox {
            id: animInFgCheckBox
            anchors.top: showBgAnimCheckBox.bottom
            anchors.topMargin: 10
            anchors.bottom: showBgAnimCheckBox.bottom
            anchors.bottomMargin: -26
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            checked: toxicWasteMain.animateInForeground
            text: qsTr("Animate in foreground?")
            onClicked: {
                toxicWasteMain.animateInForeground = checked;
                toxicWasteMain.ignoreLaunch = true;
                resetIgnoreLaunchTimer.restart();
            }
            onFocusChanged: {
                if ( !focus )
                    toxicWasteMain.focus = true;
            }
            KeyNavigation.tab: showFpsCheckBox
            KeyNavigation.backtab: showBgAnimCheckBox
            KeyNavigation.right: showFpsCheckBox
            KeyNavigation.left: showBgAnimCheckBox
        }
        CheckBox {
            id: showFpsCheckBox
            anchors.top: animInFgCheckBox.bottom
            anchors.topMargin: 10
            anchors.bottom: animInFgCheckBox.bottom
            anchors.bottomMargin: -26
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            checked: toxicWasteMain.fpsVisible
            text: qsTr("Show FPS counter?")
            onClicked: {
                toxicWasteMain.fpsVisible = checked;
                toxicWasteMain.ignoreLaunch = true;
                resetIgnoreLaunchTimer.restart();
            }
            onFocusChanged: {
                if ( !focus )
                    toxicWasteMain.focus = true;
            }
            KeyNavigation.tab: closeButton
            KeyNavigation.backtab: animInFgCheckBox
            KeyNavigation.right: closeButton
            KeyNavigation.left: animInFgCheckBox
        }
        Button {
            id: closeButton
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            anchors.horizontalCenterOffset: 0
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Close")
            onClicked: {
                toxicWasteMain.ignoreLaunch = true;
                preferencesDialog.state = "hidden";
                resetIgnoreLaunchTimer.restart();
            }
            onFocusChanged: {
                if ( !focus )
                    toxicWasteMain.focus = true;
            }
            KeyNavigation.tab: showBgAnimCheckBox
            KeyNavigation.backtab: showFpsCheckBox
            KeyNavigation.right: showBgAnimCheckBox
            KeyNavigation.left: showFpsCheckBox
        }
        onStateChanged: {
            if ( state == "shown" && confirmQuitDialog.state == "hidden" )
                closeButton.focus = true;
            else {
                closeButton.focus = false;
                showBgAnimCheckBox.focus = false;
                showFpsCheckBox = false;
            }
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
    Image {
        id: showHideMenuBarButton
        source: "images/hide_show_menu.png"
        height: 20 * ToxicWaste.scaleFactorX()
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: parent.height/2 - (menuAndStatusBar.height/2) * ToxicWaste.scaleFactorX()
        anchors.horizontalCenter: parent.left
        anchors.horizontalCenterOffset: width/2 + 2 * ToxicWaste.scaleFactorX()
        fillMode: Image.PreserveAspectFit
        opacity: 0.5
        rotation: toxicWasteMain.menuHidden ? 0 : 180
        smooth: true
        z: 5
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: parent.opacity = 1.0
            onExited: parent.opacity = 0.5
            onClicked: {
                parent.opacity = 0.5;
                if ( parent.rotation == 0 ) {
                    parent.rotation = 180;
                    toxicWasteMain.menuHidden = false;
                } else {
                    parent.rotation = 0;
                    toxicWasteMain.menuHidden = true;
                }
                searchTextInput.focus = false;
            }
        }
    }
    onMenuHiddenChanged: {
        if ( menuHidden )
            menuAndStatusBar.anchors.bottomMargin -= 64;
        else
            menuAndStatusBar.anchors.bottomMargin += 64;
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
            anchors.leftMargin: (showHideMenuBarButton.width + 2)/ToxicWaste.scaleFactorX() + 5
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
                onClicked: {
                    parent.opacity = 1.0;
                    confirmQuitDialog.state = "shown";
                    searchTextInput.focus = false;
                }
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
                    searchTextInput.focus = false;
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
                onClicked: {
                    parent.opacity = 1.0;
                    preferencesDialog.state = "shown";
                    searchTextInput.focus = false;
                }
            }
        }
        Item {
            id: searchBox
            anchors.centerIn: parent
            width: 229 / ToxicWaste.scaleFactorX()
            height: 24
            scale: ToxicWaste.scaleFactorX()
            Image {
                id: searchImage
                source: "images/find.png"
                height: 20 / ToxicWaste.scaleFactorX()
                fillMode: Image.PreserveAspectFit
                smooth: true
                anchors.verticalCenter: parent.verticalCenter
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: parent.opacity = 0.5
                    onExited: parent.opacity = 1.0
                    onClicked: {
                        parent.opacity = 1.0;
                        gamelistView.positionViewAtIndex(viewer.findIndex(searchTextInput.text, gamelistView.currentIndex), ListView.Beginning);
                        searchTextInput.focus = false;
                    }
                }
            }
            Rectangle {
                id: searchTextInputBox
                height: searchImage.height
                width: 200 / ToxicWaste.scaleFactorX()
                radius: height
                smooth: true
                anchors.left: searchImage.right
                anchors.leftMargin: 5 / ToxicWaste.scaleFactorX()
                anchors.verticalCenter: searchImage.verticalCenter
                TextInput {
                    id: searchTextInput
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 2
                    anchors.right: parent.right
                    anchors.rightMargin: 2
                    font.pointSize: parent.height
                    smooth: true
                    cursorDelegate: Rectangle {
                        id: searchTextCursorDelegate
                        color: "black"
                        width: 1
                        height: 10
                        anchors.verticalCenter: parent.verticalCenter
                        visible: parent.activeFocus
                        SequentialAnimation on opacity {
                            loops: Animation.Infinite;
                            PropertyAnimation { from: 0.0; to: 1.0; duration: 250 }
                            PauseAnimation { duration: 500 }
                            PropertyAnimation { from: 1.0; to: 0.0; duration: 250 }
                            PauseAnimation { duration: 500 }
                        }
                    }
                    onAccepted: gamelistView.positionViewAtIndex(viewer.findIndex(searchTextInput.text, gamelistView.currentIndex), ListView.Beginning);
                    onFocusChanged: {
                        if ( !focus )
                            toxicWasteMain.focus = true;
                    }
                }
            }
            Image {
                id: clearImage
                source: "images/clear.png"
                height: 28 / ToxicWaste.scaleFactorX()
                fillMode: Image.PreserveAspectFit
                smooth: true
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: searchTextInputBox.right
                anchors.leftMargin: 5 / ToxicWaste.scaleFactorX()
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: parent.opacity = 0.5
                    onExited: parent.opacity = 1.0
                    onClicked: {
                        parent.opacity = 1.0;
                        searchTextInput.text = "";
                        searchTextInput.focus = false;
                    }
                }
            }
        }
    }
    BackgroundAnimation {
        id: backgroundAnim
        opacity: parent.showBackgroundAnimation ? 1.0 : 0.0
        Behavior on opacity {
            NumberAnimation { properties: "opacity"; duration: 1000 }
        }
        z: toxicWasteMain.animateInForeground ? 5 : 2
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
            if ( searchTextInput.focus )
                searchTextInput.focus = false;
            else {
                if ( confirmQuitDialog.state == "hidden" )
                    confirmQuitDialog.state = "shown";
                else
                    confirmQuitDialog.state = "hidden";
            }
            event.accepted = true;
            break;
        case Qt.Key_F11:
            fullScreen = !fullScreen;
            event.accepted = true;
            break;
        case Qt.Key_Enter:
        case Qt.Key_Return:
            if ( event.modifiers & Qt.AltModifier ) {
                fullScreen = !fullScreen;
                event.accepted = true;
            }
            break;
        default:
            if ( ToxicWaste.validateKey(event.text) ) {
                searchTextInput.text += event.text;
                searchTextInput.focus = true;
            } else if ( ToxicWaste.validateSpecialKey(event.text) ) {
                searchTextInput.focus = true;
                switch ( event.text ) {
                case "\b":
                    if ( searchTextInput.text.length > 0)
                        searchTextInput.text = searchTextInput.text.substring(0, searchTextInput.text.length - 1);
                    break;
                }
            }
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
