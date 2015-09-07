import QtQuick 1.1
import QtMultimediaKit 1.1
import Qt.labs.shaders 1.0
import "ToxicWaste.js" as ToxicWaste
import "./animations"
import "./components"
import "./effects"
import Wheel 1.0

Rectangle {
    id: toxicWasteMain

    property int fps: 0
    property bool ignoreLaunch: false
    property bool invertFlip: false
    property bool horizontalFlip: true
    property bool flipDirectionChanged: false
    property bool iconsReady: false

    // restored properties
    property bool fpsVisible: false
    property bool showBackgroundAnimation: false
    property bool showShaderEffect: false
    property bool animateInForeground: false
    property bool fullScreen: false
    property string cabinetImageType: "preview"
    property string secondaryImageType: "preview"
    property bool cabinetFlipped: false
    property int lastIndex: 0
    property bool menuHidden: false
    property string version: ""
    property bool confirmQuit: true
    property int gameCardPage: 0
    property bool autoPositionOverlay: true
    property real overlayScale: 0.73
    property real overlayOffsetX: 0
    property real overlayOffsetY: 0
    property real overlayOpacity: 1
    property real backgroundOpacity: 0.7
    property real machineListOpacity: 1
    property bool autoStopAnimations: true
    property real videoPlayerVolume: 0.5

    // delayed init
    Timer {
        id: initTimer
        interval: 1
        running: false
        repeat: false
        onTriggered: {
            viewer.log("ToxicWaste: " + qsTr("Starting initialization"));
            ToxicWaste.init();
            restoreLastIndexTimer.start();
        }
    }

    Timer {
        id: restoreLastIndexTimer
        interval: 1
        running: false
        repeat: false
        onTriggered: ToxicWaste.restoreLastIndex()
    }

    Component.onCompleted: initTimer.start()

    Timer {
        id: launchButtonFlashTimer
        interval: 100
        running: false
        onTriggered: launchButton.opacity = 0.8
    }

    Timer {
        id: resetIgnoreLaunchTimer
        interval: 100
        running: false
        repeat: false
        onTriggered: toxicWasteMain.ignoreLaunch = false
    }

    Timer {
        id: flipDirectionResetTimer
        interval: 200
        running: false
        repeat: false
        onTriggered: toxicWasteMain.flipDirectionChanged = false
    }

    Connections {
        target: viewer
        onEmulatorStarted: ToxicWaste.emulatorStarted()
        onEmulatorFinished: ToxicWaste.emulatorStopped()
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
        opacity: toxicWasteMain.backgroundOpacity
    }
    Item {
        id: overlayItem
        z: 3
        anchors.top: parent.top
        anchors.topMargin: 10 * ToxicWaste.scaleFactorY()
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10 * ToxicWaste.scaleFactorY()
        anchors.left: parent.left
        anchors.leftMargin: 20 * ToxicWaste.scaleFactorX()
        anchors.right: parent.right
        anchors.rightMargin: 320 * ToxicWaste.scaleFactorX()
        Flipable {
            id: overlayFlip
            anchors.fill: parent
            front: Item {
                id: frontItem
                anchors.fill: parent
                scale: toxicWasteMain.autoPositionOverlay ? 1 : toxicWasteMain.overlayScale
                property bool shiftPressed: false
                property bool altPressed: false
                property bool ctrlPressed: false
                Image {
                    id: overlayImage
                    source: "images/overlay.png"
                    opacity: toxicWasteMain.overlayOpacity
                    fillMode: Image.PreserveAspectFit
                    width: 380
                    scale: ToxicWaste.scaleFactorX()
                    anchors.centerIn: parent
                    anchors.verticalCenterOffset: toxicWasteMain.autoPositionOverlay ? ToxicWaste.overlayOffset(height) : toxicWasteMain.overlayOffsetY
                    anchors.horizontalCenterOffset: toxicWasteMain.autoPositionOverlay ? 0 : (toxicWasteMain.overlayOffsetX + toxicWasteMain.mapToItem(parent, toxicWasteMain.width/2, 0).x - parent.width/2) * toxicWasteMain.overlayScale / toxicWasteMain.overlayScale // <= this is funny, I know, but seems to be the only way to trigger a recalculation when overlayScale changes
                    smooth: true
                }
                Rectangle {
                    id: previewRect
                    width: 239
                    height: 165
                    opacity: toxicWasteMain.overlayOpacity
                    anchors.centerIn: overlayImage
                    anchors.verticalCenterOffset: -124 * ToxicWaste.scaleFactorX()
                    scale: overlayImage.scale
                    color: "#202020"
                    smooth: true
                    z: -1
                    Image {
                        id: previewImage
                        cache: false
                        source: ToxicWaste.imageUrl(toxicWasteMain.cabinetImageType)
                        smooth: true
                        anchors.fill: parent
                        anchors.centerIn: parent
                        anchors.margins: 1
                        fillMode: Image.PreserveAspectFit
                        opacity: videoSnap.playing ? 0 : 1
                        Connections {
                            target: viewer
                            onImageDataUpdated: {
                                if ( cachePrefix === ToxicWaste.cachePrefix(toxicWasteMain.cabinetImageType) )
                                    previewImage.cache = true;
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if ( videoSnap.playing )
                                    videoSnap.stop();
                                else
                                    videoSnap.play();
                            }
                        }
                    }
                    Image {
                        id: videoIndicator
                        anchors.fill: parent
                        anchors.centerIn: parent
                        anchors.margins: 1
                        source: "images/movie.png"
                        fillMode: Image.PreserveAspectFit
                        scale: 0.33
                        smooth: true
                        opacity: 0
                    }
                    Video {
                        id: videoSnap
                        anchors.fill: parent
                        anchors.centerIn: parent
                        anchors.margins: 3
                        fillMode: Video.Stretch
                        autoLoad: false
                        opacity: playing ? 1 : 0
                        property string videoUrl: viewer.videoSnapUrl(machineListModel[machineListView.currentIndex].id)
                        source: videoUrl
                        volume: toxicWasteMain.videoPlayerVolume
                        onVideoUrlChanged: {
                            videoSnap.stop();
                            if ( videoSnap.videoUrl == "" )
                                videoIndicator.opacity = 0;
                            else
                                videoIndicator.opacity = 0.4;
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if ( videoSnap.playing )
                                    videoSnap.stop();
                                else
                                    videoSnap.play();
                            }
                        }
                    }
                }
                WheelArea {
                    anchors.fill: overlayImage
                    onWheel: {
                        if ( !toxicWasteMain.autoPositionOverlay ) {
                            frontItem.focus = true;
                            var wheelEventHandled = false;
                            if ( frontItem.altPressed ) {
                                wheelEventHandled = true;
                                toxicWasteMain.overlayOffsetX += (frontItem.shiftPressed ? 0.05 : 0.25) * delta
                            }
                            if ( frontItem.ctrlPressed ) {
                                wheelEventHandled = true;
                                toxicWasteMain.overlayOffsetY += (frontItem.shiftPressed ? 0.05 : 0.25) * delta
                            }
                            if ( !wheelEventHandled ) {
                                var newScale = toxicWasteMain.overlayScale * (1 + (frontItem.shiftPressed ? 0.01 : 0.1) * (delta / Math.abs(delta)));
                                if ( newScale > 10.0 )
                                    newScale = 10.0;
                                else if ( newScale < 0.01 )
                                    newScale = 0.01;
                                toxicWasteMain.overlayScale = newScale;
                                preferencesSlidersTab.cabinetZoomValue = toxicWasteMain.overlayScale;
                            }
                        }
                    }
                }
                focus: true
                Keys.onPressed: {
                    frontItem.shiftPressed = event.modifiers & Qt.ShiftModifier
                    frontItem.altPressed = event.modifiers & Qt.AltModifier
                    frontItem.ctrlPressed = event.modifiers & Qt.ControlModifier
                }
                Keys.onReleased: {
                    frontItem.shiftPressed = event.modifiers & Qt.ShiftModifier
                    frontItem.altPressed = event.modifiers & Qt.AltModifier
                    frontItem.ctrlPressed = event.modifiers & Qt.ControlModifier
                }
            }
            back: Rectangle {
                id: overlayRectBack
                anchors.fill: parent
                border.color: "black"
                border.width: 2 * ToxicWaste.scaleFactorX()
                radius: 5
                smooth: true
                scale: toxicWasteMain.flipDirectionChanged ? -1 : 1
                gradient: Gradient {
                    GradientStop { position: 0.00; color: "#ffffff" }
                    GradientStop { position: 0.50; color: "#007bff" }
                }
                Text {
                    id: itemDescription
                    text: ToxicWaste.gameCardHeader()
                    font.pixelSize: 12 * ToxicWaste.scaleFactorY()
                    anchors.top: parent.top
                    anchors.topMargin: 10 * ToxicWaste.scaleFactorX()
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 76 * ToxicWaste.scaleFactorX()
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.WordWrap
                }
                TabWidget {
                    id: gameCardTabWidget
                    anchors.fill: parent
                    anchors.leftMargin: 10 * ToxicWaste.scaleFactorX()
                    anchors.rightMargin: anchors.leftMargin
                    anchors.bottom: parent.bottom
                    anchors.topMargin: itemDescription.height + 25 * ToxicWaste.scaleFactorY()
                    anchors.horizontalCenter: parent.horizontalCenter
                    current: toxicWasteMain.gameCardPage
                    onCurrentChanged: toxicWasteMain.gameCardPage = current
                    baseColor: "#55a5ff"
                    activeBorderColor: "black"
                    inactiveBorderColor: "#202020"
                    activeTextColor: "black"
                    inactiveTextColor: "#202020"
                    fontSize: 12 * ToxicWaste.scaleFactorX()
                    smooth: true
                    scaleFactor: ToxicWaste.scaleFactorX()
                    Rectangle {
                        id: imageViewerRect
                        property string title: qsTr("Images")
                        anchors.fill: parent
                        anchors.topMargin: 5 * ToxicWaste.scaleFactorY()
                        anchors.bottomMargin: 30 * ToxicWaste.scaleFactorY()
                        smooth: true
                        border.color: "black"
                        border.width: ToxicWaste.scaleFactorX()
                        radius: 5
                        color: "#202020"
                        Image {
                            id: imageViewer
                            cache: false
                            source: ToxicWaste.imageUrl(toxicWasteMain.secondaryImageType)
                            smooth: true
                            anchors.fill: parent
                            anchors.centerIn: parent
                            anchors.margins: 2
                            fillMode: Image.PreserveAspectFit
                            Connections {
                                target: viewer
                                onImageDataUpdated: {
                                    if ( cachePrefix === ToxicWaste.cachePrefix(toxicWasteMain.secondaryImageType) )
                                        imageViewer.cache = true;
                                }
                            }
                        }
                        Rectangle {
                            id: imageTypeSelector
                            gradient: Gradient {
                                GradientStop { position: 0.00; color: "black" }
                                GradientStop { position: 0.75; color: "white" }
                                GradientStop { position: 1.00; color: "black" }
                            }
                            radius: height/2
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: -25 * ToxicWaste.scaleFactorY()
                            height: 20 * ToxicWaste.scaleFactorY()
                            width: 300 * ToxicWaste.scaleFactorX()
                            smooth: true
                            Text {
                                id: imageTypeText
                                text: ToxicWaste.gameImageType(toxicWasteMain.secondaryImageType)
                                color: "black"
                                font.bold: true
                                font.pixelSize: 12 * ToxicWaste.scaleFactorY()
                                anchors.fill: parent
                                anchors.centerIn: parent
                                anchors.leftMargin: nextImageButton.width
                                anchors.rightMargin: previousImageButton.width
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                smooth: true
                                elide: Text.ElideRight
                            }
                            Image {
                                id: nextImageButton
                                opacity: 0.5
                                source: "images/arrow.png"
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.horizontalCenterOffset: 150 * ToxicWaste.scaleFactorX() - width/2
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
                                        if ( toxicWasteMain.cabinetImageType == toxicWasteMain.secondaryImageType )
                                            miniCabinetImage.opacity = 1.0;
                                        else
                                            miniCabinetImage.opacity = 0.3;
                                    }
                                }
                            }
                            Image {
                                id: previousImageButton
                                opacity: 0.5
                                source: "images/arrow.png"
                                mirror: true
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.horizontalCenterOffset: -150 * ToxicWaste.scaleFactorX() + width/2
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
                                        if ( toxicWasteMain.cabinetImageType == toxicWasteMain.secondaryImageType )
                                            miniCabinetImage.opacity = 1.0;
                                        else
                                            miniCabinetImage.opacity = 0.3;
                                    }
                                }
                            }
                        }
                        Image {
                            id: miniCabinetImage
                            source: "images/cabinet_small.png"
                            anchors.left: parent.left
                            anchors.verticalCenter: imageTypeSelector.verticalCenter
                            smooth: true
                            fillMode: Image.PreserveAspectFit
                            height: imageTypeSelector.height
                            opacity: toxicWasteMain.cabinetImageType == toxicWasteMain.secondaryImageType ? 1.0 : 0.3
                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                onEntered: miniCabinetImage.opacity = 1.0
                                onExited: {
                                    if ( toxicWasteMain.cabinetImageType == toxicWasteMain.secondaryImageType )
                                        miniCabinetImage.opacity = 1.0;
                                    else
                                        miniCabinetImage.opacity = 0.3;
                                }
                                onClicked: toxicWasteMain.cabinetImageType = toxicWasteMain.secondaryImageType
                            }
                        }
                    }
                    Rectangle {
                        id: emuInfoViewer
                        property string title: qsTr("Emu info")
                        anchors.fill: parent
                        anchors.topMargin: 5 * ToxicWaste.scaleFactorY()
                        anchors.bottomMargin: 10 * ToxicWaste.scaleFactorY()
                        smooth: true
                        border.color: "black"
                        border.width: ToxicWaste.scaleFactorX()
                        radius: 5
                        color: "#202020"
                        TextArea {
                            anchors.fill: parent
                            anchors.margins: 5
                            color: "transparent"
                            fontSize: 12 * ToxicWaste.scaleFactorX()
                            fontColor: "white"
                            arrowIcon: "images/down_arrow_white.png"
                            displayText: viewer.requestInfo(machineListModel[machineListView.currentIndex].id, "emuinfo");
                        }
                    }
                    Rectangle {
                        id: gameInfoViewer
                        property string title: qsTr("Machine info")
                        anchors.fill: parent
                        anchors.topMargin: 5 * ToxicWaste.scaleFactorY()
                        anchors.bottomMargin: 10 * ToxicWaste.scaleFactorY()
                        smooth: true
                        border.color: "black"
                        border.width: ToxicWaste.scaleFactorX()
                        radius: 5
                        color: "#202020"
                        TextArea {
                            anchors.fill: parent
                            anchors.margins: 5
                            color: "transparent"
                            fontSize: 12 * ToxicWaste.scaleFactorX()
                            fontColor: "white"
                            arrowIcon: "images/down_arrow_white.png"
                            displayText: viewer.requestInfo(machineListModel[machineListView.currentIndex].id, "gameinfo");
                        }
                    }
                }
            }
            transform: Rotation {
                id: overlayRotation
                origin.x: overlayFlip.width/2
                origin.y: overlayFlip.height/2
                axis.x: toxicWasteMain.horizontalFlip ? 0 : (toxicWasteMain.invertFlip ? -1 : 1)
                axis.y: toxicWasteMain.horizontalFlip ? (toxicWasteMain.invertFlip ? -1 : 1) : 0
                axis.z: 0
                angle: 0
            }
            states: [
                State {
                    name: "back"
                    PropertyChanges { target: overlayRotation; angle: 180 }
                    PropertyChanges { target: toxicWasteMain; focus: true }
                    when: toxicWasteMain.cabinetFlipped
                },
                State {
                    name: "front"
                    PropertyChanges { target: overlayRotation; angle: 0 }
                    PropertyChanges { target: toxicWasteMain; focus: true }
                    when: !toxicWasteMain.cabinetFlipped
                }
            ]
            transitions: Transition {
                NumberAnimation { target: overlayRotation; property: "angle"; duration: 400 }
            }
            MouseArea { // right
                anchors.fill: parent
                anchors.leftMargin: parent.width/2
                anchors.bottomMargin: parent.height/4
                anchors.topMargin: parent.height/4
                onClicked: {
                    toxicWasteMain.flipDirectionChanged = !toxicWasteMain.horizontalFlip;
                    toxicWasteMain.invertFlip = flipDirectionChanged ? toxicWasteMain.cabinetFlipped : false;
                    toxicWasteMain.horizontalFlip = true;
                    toxicWasteMain.focus = true;
                    toxicWasteMain.cabinetFlipped = !toxicWasteMain.cabinetFlipped;
                    flipDirectionResetTimer.restart();
                }
                z: -1
            }
            MouseArea { // left
                anchors.fill: parent
                anchors.rightMargin: parent.width/2
                anchors.bottomMargin: parent.height/4
                anchors.topMargin: parent.height/4
                onClicked: {
                    toxicWasteMain.flipDirectionChanged = !toxicWasteMain.horizontalFlip;
                    toxicWasteMain.invertFlip = flipDirectionChanged ? !toxicWasteMain.cabinetFlipped : true;
                    toxicWasteMain.horizontalFlip = true;
                    toxicWasteMain.focus = true;
                    toxicWasteMain.cabinetFlipped = !toxicWasteMain.cabinetFlipped;
                    flipDirectionResetTimer.restart();
                }
                z: -1
            }
            MouseArea { // top
                anchors.fill: parent
                anchors.bottomMargin: parent.height - parent.height/4
                onClicked: {
                    toxicWasteMain.flipDirectionChanged = toxicWasteMain.horizontalFlip;
                    toxicWasteMain.invertFlip = flipDirectionChanged ? toxicWasteMain.cabinetFlipped : false;
                    toxicWasteMain.horizontalFlip = false;
                    toxicWasteMain.focus = true;
                    toxicWasteMain.cabinetFlipped = !toxicWasteMain.cabinetFlipped;
                    flipDirectionResetTimer.restart();
                }
                z: -1
            }
            MouseArea { // bottom
                anchors.fill: parent
                anchors.topMargin: parent.height - parent.height/4
                onClicked: {
                    toxicWasteMain.flipDirectionChanged = toxicWasteMain.horizontalFlip;
                    toxicWasteMain.invertFlip = flipDirectionChanged ? !toxicWasteMain.cabinetFlipped : true;
                    toxicWasteMain.horizontalFlip = false;
                    toxicWasteMain.focus = true;
                    toxicWasteMain.cabinetFlipped = !toxicWasteMain.cabinetFlipped;
                    flipDirectionResetTimer.restart();
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
                    viewer.launchEmulator(machineListModel[machineListView.currentIndex].id);
                    searchTextInput.focus = false;
                }
            }
        }
    }
    ListView {
        id: machineListView
        opacity: toxicWasteMain.machineListOpacity
        scale: ToxicWaste.scaleFactorX()
        flickDeceleration: 3500
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
        highlightMoveDuration: 500
        cacheBuffer: 72 * itemsPerPage() // 72 = machineListItemDelegate.height
        delegate: Item {
            property string gameId: id
            id: machineListItemDelegate
            width: 280
            height: 72
            Rectangle {
                id: machineListItemBackground
                smooth: true
                anchors.fill: machineListItemDelegate
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "lightgrey" }
                    GradientStop { position: 0.5; color: "white" }
                    GradientStop { position: 1.0; color: "lightgrey" }
                }
                opacity: 0.8
                radius: 10
                border.color: "black"
                border.width: 2
                Image {
                    id: machineListItemIcon
                    cache: viewer.isSevenZippedImageType("ico") ? toxicWasteMain.iconsReady : true
                    source: "image://qmc2/ico/" + model.modelData.id + "/" + model.modelData.parentId
                    anchors.left: machineListItemBackground.left
                    anchors.verticalCenter: machineListItemBackground.verticalCenter
                    anchors.margins: 10
                    smooth: true
                    fillMode: Image.PreserveAspectFit
                    height: machineListItemBackground.height / 3
                    asynchronous: !viewer.isSevenZippedImageType("ico")
                    Connections {
                        target: viewer
                        onImageDataUpdated: {
                            if ( cachePrefix === "ico" )
                                toxicWasteMain.iconsReady = true;
                        }
                    }
                }
                Text {
                    property bool fontResized: false
                    id: machineListItemText
                    text: model.modelData.description
                    color: "black"
                    font.bold: true
                    font.italic: true
                    font.pixelSize: machineListItemBackground.height / 3
                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                    anchors.centerIn: machineListItemBackground
                    anchors.horizontalCenterOffset: machineListItemIcon.width > 1 ? (paintedWidth + machineListItemIcon.width + 45 < machineListItemBackground.width ? 0 : machineListItemIcon.width - 10) : 0
                    width: machineListItemIcon.width > 1 ? machineListItemBackground.width - machineListItemIcon.width - 20 : machineListItemBackground.width - 20
                    smooth: true
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
                MouseArea {
                    id: machineListItemMouseArea
                    anchors.fill: machineListItemBackground
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton
                    onContainsMouseChanged: {
                        if ( mapToItem(menuAndStatusBar, mouseX, mouseY).y < 0 ) {
                            if ( containsMouse )
                                ToxicWaste.itemEntered(machineListItemText, machineListItemBackground, machineListItemIcon);
                            else
                                ToxicWaste.itemExited(machineListItemText, machineListItemBackground, machineListItemIcon);
                        }
                    }
                    onDoubleClicked: {
                        machineListView.currentIndex = index;
                        ToxicWaste.itemClicked(machineListItemText, machineListItemBackground, machineListItemIcon);
                        searchTextInput.focus = false;
                        launchButton.opacity = 1.0;
                        viewer.launchEmulator(machineListModel[machineListView.currentIndex].id);
                        launchButtonFlashTimer.start();
                    }
                    onClicked: {
                        machineListView.currentIndex = index;
                        ToxicWaste.itemClicked(machineListItemText, machineListItemBackground, machineListItemIcon);
                        searchTextInput.focus = false;
                    }
                }
            }
        }
        model: machineListModel
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
                if ( currentIndex + itemsPerPage() < machineListModelCount - 1 ) {
                    decrementCurrentIndex();
                    contentY = contentY + height - 70;
                    if ( currentIndex < machineListModelCount - 1 )
                        incrementCurrentIndex();
                    else {
                        contentY = contentHeight - 82;
                        currentIndex = machineListModelCount - 1;
                    }
                } else
                    currentIndex = machineListModelCount - 1;
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
                    viewer.launchEmulator(machineListModel[machineListView.currentIndex].id);
                    launchButtonFlashTimer.start();
                    event.accepted = true;
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
        Rectangle {
            anchors.fill: parent
            anchors.margins: 5
            color: "#55a5ff"
            border.color: "black"
            border.width: 1
            radius: 5
            smooth: true
        }
        Text {
            text: qsTr("Really quit?")
            anchors.top: parent.top
            anchors.topMargin: 15
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.horizontalCenterOffset: 0
            font.pixelSize: 12
            color: "black"
            smooth: true
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
        width: 300
        height: 250
        border.color: "black"
        border.width: 2
        color: "#007bff"
        opacity: 0.0
        state: "hidden"
        z: 4
        MouseArea {
            anchors.fill: parent
        }
        TabWidget {
            id: preferencesTabWidget
            anchors.fill: parent
            anchors.margins: 5
            baseColor: "#55a5ff"
            activeBorderColor: "black"
            inactiveBorderColor: "#202020"
            activeTextColor: "black"
            inactiveTextColor: "#202020"
            fontSize: 12
            smooth: true
            scaleFactor: ToxicWaste.scaleFactorX()
            Rectangle {
                id: preferencesSwitchesTab
                property string title: qsTr("Switches")
                property alias firstKeyNavItem: showBgAnimCheckBox
                property alias lastKeyNavItem: autoPositionOverlayCheckBox
                anchors.fill: parent
                color: preferencesTabWidget.baseColor
                radius: 5
                smooth: true
                border.color: preferencesTabWidget.activeBorderColor
                CheckBox {
                    id: showBgAnimCheckBox
                    anchors.top: preferencesSwitchesTab.top
                    anchors.topMargin: 10
                    anchors.bottom: preferencesSwitchesTab.top
                    anchors.bottomMargin: -26
                    anchors.left: parent.left
                    anchors.leftMargin: 5
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    checked: toxicWasteMain.showBackgroundAnimation
                    text: qsTr("Show floating-bubbles animation?")
                    textColor: "black"
                    onClicked: {
                        toxicWasteMain.showBackgroundAnimation = checked;
                        toxicWasteMain.ignoreLaunch = true;
                        resetIgnoreLaunchTimer.restart();
                        if ( checked )
                            backgroundAnim.opacity = 1.0;
                        else
                            backgroundAnim.opacity = 0.0;
                        focus = true;
                    }
                    onFocusChanged: {
                        if ( !focus )
                            toxicWasteMain.focus = true;
                    }
                    KeyNavigation.tab: preferencesTabWidget.nextKeyNavItem(animInFgCheckBox, preferencesSwitchesTab)
                    KeyNavigation.backtab: preferencesTabWidget.previousKeyNavItem(closeButton, preferencesSwitchesTab)
                    KeyNavigation.right: preferencesTabWidget.nextKeyNavItem(animInFgCheckBox, preferencesSwitchesTab)
                    KeyNavigation.left: preferencesTabWidget.previousKeyNavItem(closeButton, preferencesSwitchesTab)
                    smooth: true
                }
                CheckBox {
                    id: animInFgCheckBox
                    anchors.top: showBgAnimCheckBox.bottom
                    anchors.topMargin: 10
                    anchors.bottom: showBgAnimCheckBox.bottom
                    anchors.bottomMargin: -26
                    anchors.left: parent.left
                    anchors.leftMargin: 5
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    checked: toxicWasteMain.animateInForeground
                    text: qsTr("Draw animation in the foreground?")
                    textColor: "black"
                    onClicked: {
                        toxicWasteMain.animateInForeground = checked;
                        toxicWasteMain.ignoreLaunch = true;
                        resetIgnoreLaunchTimer.restart();
                        focus = true;
                    }
                    onFocusChanged: {
                        if ( !focus )
                            toxicWasteMain.focus = true;
                    }
                    KeyNavigation.tab: preferencesTabWidget.nextKeyNavItem(showShaderEffectCheckBox, preferencesSwitchesTab)
                    KeyNavigation.backtab: preferencesTabWidget.previousKeyNavItem(showBgAnimCheckBox, preferencesSwitchesTab)
                    KeyNavigation.right: preferencesTabWidget.nextKeyNavItem(showShaderEffectCheckBox, preferencesSwitchesTab)
                    KeyNavigation.left: preferencesTabWidget.previousKeyNavItem(showBgAnimCheckBox, preferencesSwitchesTab)
                    smooth: true
                }
                CheckBox {
                    id: showShaderEffectCheckBox
                    anchors.top: animInFgCheckBox.bottom
                    anchors.topMargin: 10
                    anchors.bottom: animInFgCheckBox.bottom
                    anchors.bottomMargin: -26
                    anchors.left: parent.left
                    anchors.leftMargin: 5
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    checked: toxicWasteMain.showShaderEffect
                    text: qsTr("Show radial wave effect on background?")
                    textColor: "black"
                    onClicked: {
                        toxicWasteMain.showShaderEffect = checked;
                        waveEffect.running = checked;
                        toxicWasteMain.ignoreLaunch = true;
                        resetIgnoreLaunchTimer.restart();
                        focus = true;
                    }
                    onFocusChanged: {
                        if ( !focus )
                            toxicWasteMain.focus = true;
                    }
                    KeyNavigation.tab: preferencesTabWidget.nextKeyNavItem(autoStopAnimationsCheckBox, preferencesSwitchesTab)
                    KeyNavigation.backtab: preferencesTabWidget.previousKeyNavItem(animInFgCheckBox, preferencesSwitchesTab)
                    KeyNavigation.right: preferencesTabWidget.nextKeyNavItem(autoStopAnimationsCheckBox, preferencesSwitchesTab)
                    KeyNavigation.left: preferencesTabWidget.previousKeyNavItem(animInFgCheckBox, preferencesSwitchesTab)
                    smooth: true
                }
                CheckBox {
                    id: autoStopAnimationsCheckBox
                    anchors.top: showShaderEffectCheckBox.bottom
                    anchors.topMargin: 10
                    anchors.bottom: showShaderEffectCheckBox.bottom
                    anchors.bottomMargin: -26
                    anchors.left: parent.left
                    anchors.leftMargin: 5
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    checked: toxicWasteMain.autoStopAnimations
                    text: qsTr("Auto-stop animation and wave effect?")
                    textColor: "black"
                    onClicked: {
                        toxicWasteMain.autoStopAnimations = checked;
                        toxicWasteMain.ignoreLaunch = true;
                        resetIgnoreLaunchTimer.restart();
                        focus = true;
                    }
                    onFocusChanged: {
                        if ( !focus )
                            toxicWasteMain.focus = true;
                    }
                    KeyNavigation.tab: preferencesTabWidget.nextKeyNavItem(showFpsCheckBox, preferencesSwitchesTab)
                    KeyNavigation.backtab: preferencesTabWidget.previousKeyNavItem(showShaderEffectCheckBox, preferencesSwitchesTab)
                    KeyNavigation.right: preferencesTabWidget.nextKeyNavItem(showFpsCheckBox, preferencesSwitchesTab)
                    KeyNavigation.left: preferencesTabWidget.previousKeyNavItem(showShaderEffectCheckBox, preferencesSwitchesTab)
                    smooth: true
                }
                CheckBox {
                    id: showFpsCheckBox
                    anchors.top: autoStopAnimationsCheckBox.bottom
                    anchors.topMargin: 10
                    anchors.bottom: autoStopAnimationsCheckBox.bottom
                    anchors.bottomMargin: -26
                    anchors.left: parent.left
                    anchors.leftMargin: 5
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    checked: toxicWasteMain.fpsVisible
                    text: qsTr("Show FPS counter in the menu-bar?")
                    textColor: "black"
                    onClicked: {
                        toxicWasteMain.fpsVisible = checked;
                        toxicWasteMain.ignoreLaunch = true;
                        resetIgnoreLaunchTimer.restart();
                        focus = true;
                    }
                    onFocusChanged: {
                        if ( !focus )
                            toxicWasteMain.focus = true;
                    }
                    KeyNavigation.tab: preferencesTabWidget.nextKeyNavItem(confirmQuitCheckBox, preferencesSwitchesTab)
                    KeyNavigation.backtab: preferencesTabWidget.previousKeyNavItem(autoStopAnimationsCheckBox, preferencesSwitchesTab)
                    KeyNavigation.right: preferencesTabWidget.nextKeyNavItem(confirmQuitCheckBox, preferencesSwitchesTab)
                    KeyNavigation.left: preferencesTabWidget.previousKeyNavItem(autoStopAnimationsCheckBox, preferencesSwitchesTab)
                    smooth: true
                }
                CheckBox {
                    id: confirmQuitCheckBox
                    anchors.top: showFpsCheckBox.bottom
                    anchors.topMargin: 10
                    anchors.bottom: showFpsCheckBox.bottom
                    anchors.bottomMargin: -26
                    anchors.left: parent.left
                    anchors.leftMargin: 5
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    checked: toxicWasteMain.confirmQuit
                    text: qsTr("Confirm when quitting the application?")
                    textColor: "black"
                    onClicked: {
                        toxicWasteMain.confirmQuit = checked;
                        toxicWasteMain.ignoreLaunch = true;
                        resetIgnoreLaunchTimer.restart();
                        focus = true;
                    }
                    onFocusChanged: {
                        if ( !focus )
                            toxicWasteMain.focus = true;
                    }
                    KeyNavigation.tab: preferencesTabWidget.nextKeyNavItem(autoPositionOverlayCheckBox, preferencesSwitchesTab)
                    KeyNavigation.backtab: preferencesTabWidget.previousKeyNavItem(showFpsCheckBox, preferencesSwitchesTab)
                    KeyNavigation.right: preferencesTabWidget.nextKeyNavItem(autoPositionOverlayCheckBox, preferencesSwitchesTab)
                    KeyNavigation.left: preferencesTabWidget.previousKeyNavItem(showFpsCheckBox, preferencesSwitchesTab)
                    smooth: true
                }
                CheckBox {
                    id: autoPositionOverlayCheckBox
                    anchors.top: confirmQuitCheckBox.bottom
                    anchors.topMargin: 10
                    anchors.bottom: confirmQuitCheckBox.bottom
                    anchors.bottomMargin: -26
                    anchors.left: parent.left
                    anchors.leftMargin: 5
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    checked: toxicWasteMain.autoPositionOverlay
                    text: qsTr("Scale & position cabinet automatically?")
                    textColor: "black"
                    onClicked: {
                        toxicWasteMain.autoPositionOverlay = checked;
                        toxicWasteMain.ignoreLaunch = true;
                        resetIgnoreLaunchTimer.restart();
                        focus = true;
                    }
                    onFocusChanged: {
                        if ( !focus )
                            toxicWasteMain.focus = true;
                    }
                    KeyNavigation.tab: preferencesTabWidget.nextKeyNavItem(closeButton, preferencesSwitchesTab)
                    KeyNavigation.backtab: preferencesTabWidget.previousKeyNavItem(confirmQuitCheckBox, preferencesSwitchesTab)
                    KeyNavigation.right: preferencesTabWidget.nextKeyNavItem(closeButton, preferencesSwitchesTab)
                    KeyNavigation.left: preferencesTabWidget.previousKeyNavItem(confirmQuitCheckBox, preferencesSwitchesTab)
                    smooth: true
                }
            }
            Rectangle {
                id: preferencesSlidersTab
                property string title: qsTr("Sliders")
                property alias firstKeyNavItem: cliOptionCombo
                property alias lastKeyNavItem: cliValueCombo
                property alias cabinetZoomValue: cabinetZoomSlider.value
                anchors.fill: parent
                color: preferencesTabWidget.baseColor
                border.color: preferencesTabWidget.activeBorderColor
                radius: 5
                smooth: true
                Slider {
                    id: cabinetZoomSlider
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 5
                    sliderText: qsTr("Cabinet zoom")
                    minimum: 0.01
                    maximum: 10
                    value: toxicWasteMain.overlayScale
                    defaultValue: 1
                    onValueChanged: toxicWasteMain.overlayScale = value
                }
                Slider {
                    id: cabinetOffsetXSlider
                    anchors.top: cabinetZoomSlider.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 5
                    sliderText: qsTr("Cabinet X center offset")
                    minimum: (-toxicWasteMain.width/2 - 514 * toxicWasteMain.overlayScale) / toxicWasteMain.overlayScale
                    maximum: (toxicWasteMain.width/2 + 514 * toxicWasteMain.overlayScale) / toxicWasteMain.overlayScale
                    showAsPercent: false
                    value: toxicWasteMain.overlayOffsetX
                    defaultValue: 0
                    onValueChanged: toxicWasteMain.overlayOffsetX = value
                }
                Slider {
                    id: cabinetOffsetYSlider
                    anchors.top: cabinetOffsetXSlider.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 5
                    sliderText: qsTr("Cabinet Y center offset")
                    minimum: (-toxicWasteMain.height/2 - 1104 * toxicWasteMain.overlayScale) / toxicWasteMain.overlayScale
                    maximum: (toxicWasteMain.height/2 + 1104 * toxicWasteMain.overlayScale) / toxicWasteMain.overlayScale
                    showAsPercent: false
                    value: toxicWasteMain.overlayOffsetY
                    defaultValue: 0
                    onValueChanged: toxicWasteMain.overlayOffsetY = value
                }
                Slider {
                    id: cabinetOpacitySlider
                    anchors.top: cabinetOffsetYSlider.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 5
                    sliderText: qsTr("Cabinet opacity")
                    minimum: 0
                    maximum: 1
                    value: toxicWasteMain.overlayOpacity
                    defaultValue: 1
                    onValueChanged: toxicWasteMain.overlayOpacity = value
                }
                Slider {
                    id: backgroundOpacitySlider
                    anchors.top: cabinetOpacitySlider.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 5
                    sliderText: qsTr("Background opacity")
                    minimum: 0
                    maximum: 1
                    value: toxicWasteMain.backgroundOpacity
                    defaultValue: 0.7
                    onValueChanged: toxicWasteMain.backgroundOpacity = value
                }
                Slider {
                    id: machineListOpacitySlider
                    anchors.top: backgroundOpacitySlider.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 5
                    sliderText: qsTr("Machine list opacity")
                    minimum: 0
                    maximum: 1
                    value: toxicWasteMain.machineListOpacity
                    defaultValue: 1
                    onValueChanged: toxicWasteMain.machineListOpacity = value
                }
                Slider {
                    id: videoPlayerVolumeSlider
                    anchors.top: machineListOpacitySlider.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 5
                    sliderText: qsTr("Video player volume")
                    minimum: 0
                    maximum: 1
                    value: toxicWasteMain.videoPlayerVolume
                    defaultValue: 0.5
                    onValueChanged: toxicWasteMain.videoPlayerVolume = value
                }
            }
            Rectangle {
                id: preferencesBackendTab
                property string title: qsTr("Backend")
                property alias firstKeyNavItem: cliOptionCombo
                property alias lastKeyNavItem: cliValueCombo
                anchors.fill: parent
                color: preferencesTabWidget.baseColor
                border.color: preferencesTabWidget.activeBorderColor
                radius: 5
                smooth: true
                Text {
                    id: cliOptionText
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 5
                    anchors.rightMargin: parent.width/2 + 2
                    smooth: true
                    font.pixelSize: 12
                    text: qsTr("Option")
                    horizontalAlignment: Text.Center
                }
                ComboBox {
                    id: cliOptionCombo
                    anchors.top: cliOptionText.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 5
                    anchors.rightMargin: parent.width/2 + 2
                    smooth: true
                    model: ListModel {}
                    arrowIcon: "images/down_arrow.png"
                    font.pixelSize: 12
                    z: +1
                    onFocusChanged: {
                        if ( !focus )
                            toxicWasteMain.focus = true;
                    }
                    onIndexChosen: {
                        cliValueCombo.ready = false;
                        cliValueCombo.model.clear();
                        var cliParamName = viewer.cliParamNames()[index];
                        var cliParamOptions = viewer.cliParamAllowedValues(cliParamName);
                        var cliParamValue = viewer.cliParamValue(cliParamName);
                        var indexToSelect = 0;
                        for (var i = 0; i < cliParamOptions.length; i++) {
                            var cliParamOption = cliParamOptions[i];
                            cliValueCombo.model.append({'name': cliParamOption});
                            if ( cliParamOption === cliParamValue )
                                indexToSelect = i;
                        }
                        cliValueCombo.close(true);
                        cliValueCombo.positionAtTop();
                        cliValueCombo.setCurrentIndex(indexToSelect);
                        cliValueCombo.ready = true;
                    }
                    KeyNavigation.tab: preferencesTabWidget.nextKeyNavItem(cliValueCombo, preferencesBackendTab);
                    KeyNavigation.backtab: preferencesTabWidget.previousKeyNavItem(closeButton, preferencesBackendTab);
                    KeyNavigation.right: preferencesTabWidget.nextKeyNavItem(cliValueCombo, preferencesBackendTab);
                    KeyNavigation.left: preferencesTabWidget.previousKeyNavItem(closeButton, preferencesBackendTab);
                }
                Text {
                    id: cliValueText
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 5
                    anchors.leftMargin: parent.width/2 + 2
                    smooth: true
                    font.pixelSize: 12
                    horizontalAlignment: Text.Center
                    text: qsTr("Value")
                }
                ComboBox {
                    id: cliValueCombo
                    property bool ready: false
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: cliValueText.bottom
                    anchors.margins: 5
                    anchors.leftMargin: parent.width/2 + 2
                    smooth: true
                    model: ListModel {}
                    arrowIcon: "images/down_arrow.png"
                    font.pixelSize: 12
                    z: +1
                    onFocusChanged: {
                        if ( !focus )
                            toxicWasteMain.focus = true;
                    }
                    onIndexChosen: {
                        if ( ready ) {
                            var cliParamName = viewer.cliParamNames()[cliOptionCombo.currentIndex()];
                            var cliParamValue = viewer.cliParamAllowedValues(cliParamName)[cliValueCombo.currentIndex()];
                            if ( cliParamName !== "" && cliParamValue !== "" )
                                viewer.setCliParamValue(cliParamName, cliParamValue);
                        }
                    }
                    KeyNavigation.tab: preferencesTabWidget.nextKeyNavItem(closeButton, preferencesBackendTab);
                    KeyNavigation.backtab: preferencesTabWidget.previousKeyNavItem(cliOptionCombo, preferencesBackendTab);
                    KeyNavigation.right: preferencesTabWidget.nextKeyNavItem(closeButton, preferencesBackendTab);
                    KeyNavigation.left: preferencesTabWidget.previousKeyNavItem(cliOptionCombo, preferencesBackendTab);
                }
                Text {
                    id: cliInfoText
                    anchors.top: cliValueCombo.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 5
                    anchors.topMargin: 20
                    smooth: true
                    font.pixelSize: 12
                    horizontalAlignment: Text.Center
                    text: qsTr("For customized default backend options to\ntake effect, please restart QMC2 Arcade!")
                }
                Component.onCompleted: {
                    var cliParamNames = viewer.cliParamNames();
                    var cliParamOptions = viewer.cliParamAllowedValues(cliParamNames[0]);
                    var cliParamValue = viewer.cliParamValue(cliParamNames[0]);
                    var indexToSelect = 0;
                    for (var i = 0; i < cliParamOptions.length; i++) {
                        var cliParamOption = cliParamOptions[i];
                        cliValueCombo.model.append({'name': cliParamOption});
                        if ( cliParamOption === cliParamValue )
                            indexToSelect = i;
                    }
                    for (var i = 0; i < cliParamNames.length; i++)
                        cliOptionCombo.model.append({'name': viewer.cliParamDescription(cliParamNames[i])});
                    cliValueCombo.setCurrentIndex(indexToSelect);
                    cliValueCombo.ready = true;
                }
            }
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
                toxicWasteMain.focus = true;
                resetIgnoreLaunchTimer.restart();
            }
            onFocusChanged: {
                if ( !focus )
                    toxicWasteMain.focus = true;
            }
            KeyNavigation.tab: preferencesTabWidget.firstKeyNavItem()
            KeyNavigation.backtab: preferencesTabWidget.lastKeyNavItem()
            KeyNavigation.right: preferencesTabWidget.firstKeyNavItem()
            KeyNavigation.left: preferencesTabWidget.lastKeyNavItem()
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
        height: menuAndStatusBar.height
        anchors.left: parent.left
        anchors.leftMargin: 2 * ToxicWaste.scaleFactorX()
        anchors.bottom: parent.bottom
        fillMode: Image.PreserveAspectFit
        opacity: 0.5
        rotation: toxicWasteMain.menuHidden ? 0 : 180
        smooth: true
        z: 5
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: parent.opacity = 1.0
            onMouseXChanged: parent.opacity = 1.0
            onMouseYChanged: parent.opacity = 1.0
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
        if ( menuHidden ) {
            menuAndStatusBar.anchors.bottomMargin -= 64;
            searchTextInput.focus = false;
        } else
            menuAndStatusBar.anchors.bottomMargin += 64;
    }
    Rectangle {
        id: menuAndStatusBar
        x: 0
        z: 4
        width: parent.width
        height: ToxicWaste.scaleFactorY() > 1 ? 24 : (ToxicWaste.scaleFactorY() < 0.5 ? 12 : 24 * ToxicWaste.scaleFactorY())
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        transformOrigin: Rectangle.Bottom
        opacity: 1.0
        smooth: true
        gradient: Gradient {
            GradientStop { position: 0.00; color: "#40AAAAAA" }
            GradientStop { position: 0.33; color: "#60AAAAAA" }
            GradientStop { position: 0.66; color: "#60AAAAAA" }
            GradientStop { position: 1.00; color: "#40AAAAAA" }
        }
        Text {
            id: fpsText
            anchors.left: parent.left
            anchors.leftMargin: showHideMenuBarButton.width + 10 * ToxicWaste.scaleFactorX()
            anchors.verticalCenter: menuAndStatusBar.verticalCenter
            opacity: 0.8
            smooth: true
            color: "white"
            text: qsTr("FPS") + ": " + toxicWasteMain.fps.toString()
            font.pixelSize: parent.height - 4 * ToxicWaste.scaleFactorY()
            visible: toxicWasteMain.fpsVisible
        }
        Image {
            id: exitButton
            anchors.top: parent.top
            anchors.topMargin: 2 * ToxicWaste.scaleFactorY()
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 2 * ToxicWaste.scaleFactorY()
            anchors.right: parent.right
            anchors.rightMargin: 2 * ToxicWaste.scaleFactorX()
            source: "images/exit.png"
            smooth: true
            fillMode: Image.PreserveAspectFit
            opacity: 0.5
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: parent.opacity = 1.0
                onMouseXChanged: parent.opacity = 1.0
                onMouseYChanged: parent.opacity = 1.0
                onExited: parent.opacity = 0.5
                onClicked: {
                    if ( toxicWasteMain.confirmQuit ) {
                        parent.opacity = 1.0;
                        confirmQuitDialog.state = "shown";
                        searchTextInput.focus = false;
                    } else
                        Qt.quit();
                }
            }
        }
        Image {
            id: fullScreenToggleButton
            anchors.top: parent.top
            anchors.topMargin: 2 * ToxicWaste.scaleFactorY()
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 2 * ToxicWaste.scaleFactorY()
            anchors.right: exitButton.left
            anchors.rightMargin: 5 * ToxicWaste.scaleFactorX()
            source: "images/fullscreen.png"
            state: toxicWasteMain.fullScreen ? "fullscreen" : "windowed"
            smooth: true
            fillMode: Image.PreserveAspectFit
            opacity: 0.5
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: parent.opacity = 1.0
                onMouseXChanged: parent.opacity = 1.0
                onMouseYChanged: parent.opacity = 1.0
                onExited: parent.opacity = 0.5
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
            anchors.topMargin: 2 * ToxicWaste.scaleFactorY()
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 2 * ToxicWaste.scaleFactorY()
            anchors.right: fullScreenToggleButton.left
            anchors.rightMargin: 5 * ToxicWaste.scaleFactorX()
            source: "images/preferences.png"
            smooth: true
            fillMode: Image.PreserveAspectFit
            opacity: 0.5
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: parent.opacity = 1.0
                onMouseXChanged: parent.opacity = 1.0
                onMouseYChanged: parent.opacity = 1.0
                onExited: parent.opacity = 0.5
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
            width: 230 * ToxicWaste.scaleFactorX()
            height: menuAndStatusBar.height - 2 * ToxicWaste.scaleFactorY()
            Image {
                id: searchImage
                source: "images/find.png"
                height: parent.height - 4 * ToxicWaste.scaleFactorY()
                fillMode: Image.PreserveAspectFit
                smooth: true
                anchors.verticalCenter: parent.verticalCenter
                opacity: 0.5
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: parent.opacity = 1.0
                    onMouseXChanged: parent.opacity = 1.0
                    onMouseYChanged: parent.opacity = 1.0
                    onExited: parent.opacity = 0.5
                    onClicked: {
                        parent.opacity = 1.0;
                        machineListView.positionViewAtIndex(viewer.findIndex(searchTextInput.text, machineListView.currentIndex), ListView.Beginning);
                        searchTextInput.focus = false;
                    }
                }
            }
            Rectangle {
                id: searchTextInputBox
                height: parent.height - 2 * ToxicWaste.scaleFactorY()
                width: 200 * ToxicWaste.scaleFactorX()
                radius: height/2
                smooth: true
                anchors.left: searchImage.right
                anchors.leftMargin: 5 * ToxicWaste.scaleFactorX()
                anchors.verticalCenter: searchImage.verticalCenter
                TextInput {
                    id: searchTextInput
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 2 * ToxicWaste.scaleFactorX()
                    anchors.right: parent.right
                    anchors.rightMargin: 2 * ToxicWaste.scaleFactorX()
                    font.pointSize: parent.height - 2
                    smooth: true
                    focus: false
                    autoScroll: true
                    clip: true
                    selectByMouse: true
                    opacity: 0.8
                    cursorDelegate: Rectangle {
                        id: searchTextCursorDelegate
                        color: "black"
                        width: 1
                        height: parent.height
                        anchors.verticalCenter: parent.verticalCenter
                        visible: parent.activeFocus
                        SequentialAnimation on opacity {
                            loops: Animation.Infinite
                            running: searchTextCursorDelegate.visible
                            PropertyAction { target: searchTextCursorDelegate; property: "opacity"; value: 1.0 }
                            PauseAnimation { duration: 500 }
                            PropertyAction { target: searchTextCursorDelegate; property: "opacity"; value: 0.0 }
                            PauseAnimation { duration: 500 }
                        }
                    }
                    onAccepted: machineListView.positionViewAtIndex(viewer.findIndex(searchTextInput.text, machineListView.currentIndex), ListView.Beginning);
                    onFocusChanged: {
                        if ( !focus )
                            toxicWasteMain.focus = true;
                    }
                }
            }
            Image {
                id: clearImage
                source: "images/clear.png"
                height: parent.height + 2 * ToxicWaste.scaleFactorY()
                fillMode: Image.PreserveAspectFit
                smooth: true
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: searchTextInputBox.right
                anchors.leftMargin: 5 * ToxicWaste.scaleFactorY()
                opacity: 0.5
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: parent.opacity = 1.0
                    onMouseXChanged: parent.opacity = 1.0
                    onMouseYChanged: parent.opacity = 1.0
                    onExited: parent.opacity = 0.5
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
        opacity: toxicWasteMain.showBackgroundAnimation ? 1.0 : 0.0
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
        id: waveEffect
        anchors.fill: parent
        source: effectSource
        wave: 0.0
        waveOriginX: 0.5
        waveOriginY: 0.5
        waveWidth: 0.01
        z: 1
        property alias running: waveAnim.running
        NumberAnimation on wave {
            id: waveAnim
            running: toxicWasteMain.showShaderEffect
            loops: Animation.Infinite
            easing.type: Easing.Linear
            from: 0.0000;
            to: 2.0000;
            duration: 3000
            onRunningChanged: {
                if ( running )
                    viewer.log("ToxicWaste: " + qsTr("Starting shader effect"));
                else
                    viewer.log("ToxicWaste: " + qsTr("Shader effect stopped"));
            }
        }
    }
    focus: true
    Keys.onPressed: {
        switch ( event.key ) {
        case Qt.Key_Escape:
            if ( searchTextInput.focus )
                searchTextInput.focus = false;
            else {
                if ( toxicWasteMain.confirmQuit ) {
                    if ( confirmQuitDialog.state == "hidden" )
                        confirmQuitDialog.state = "shown";
                    else
                        confirmQuitDialog.state = "hidden";
                } else
                    Qt.quit();
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
            if ( event.modifiers & Qt.ControlModifier) {
                switch ( event.key ) {
                case Qt.Key_P:
                    if ( !toxicWasteMain.ignoreLaunch ) {
                        launchButton.opacity = 1.0;
                        viewer.launchEmulator(machineListModel[machineListView.currentIndex].id);
                        launchButtonFlashTimer.start();
                    }
                    event.accepted = true;
                    break;
                case Qt.Key_O:
                    preferencesDialog.state = preferencesDialog.state == "shown" ? "hidden" : "shown";
                    event.accepted = true;
                    break;
                case Qt.Key_M:
                    toxicWasteMain.menuHidden = !toxicWasteMain.menuHidden;
                    event.accepted = true;
                    break;
                case Qt.Key_X:
                    if ( toxicWasteMain.confirmQuit ) {
                        if ( confirmQuitDialog.state == "hidden" )
                            confirmQuitDialog.state = "shown";
                        else
                            confirmQuitDialog.state = "hidden";
                    } else
                        Qt.quit();
                    break;
                case Qt.Key_F:
                    if ( !toxicWasteMain.menuHidden ) {
                        searchTextInput.text = "";
                        searchTextInput.focus = true;
                        event.accepted = true;
                    }
                    break;
                case Qt.Key_Backspace:
                    toxicWasteMain.cabinetFlipped = !toxicWasteMain.cabinetFlipped;
                    event.accepted = true;
                    break;
                }
            } else if ( !toxicWasteMain.menuHidden ) {
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
    }
    Keys.forwardTo: [machineListView]
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
