import QtQuick 1.1;
import QtMultimediaKit 1.1
import "./components";
import Wheel 1.0;
import Pointer 1.0;
import "darkone.js" as DarkoneJS;

FocusScope {
    id: darkoneFocusScope
    focus: true

    width: darkone.width
    height: darkone.height

    // global properties
    property alias debug: darkone.debug
    property alias debug2: darkone.debug2
    property alias debug3: darkone.debug3
    property alias version: darkone.version
    property alias qtVersion: darkone.qtVersion
    property alias fps: darkone.fps

    // restored properties
    property alias lastIndex: darkone.lastIndex
    property alias toolbarHidden: darkone.toolbarHidden
    property alias listHidden: darkone.listHidden
    property alias fullScreen: darkone.fullScreen
    property alias fpsVisible: darkone.fpsVisible
    property alias sortByName: darkone.sortByName
    property alias launchFlash: darkone.launchFlash
    property alias launchZoom: darkone.launchZoom
    property alias dataTypePrimary: darkone.dataTypePrimary
    property alias dataTypeSecondary: darkone.dataTypeSecondary
    property alias lightTimeout: darkone.lightTimeout
    property alias backLight: darkone.backLight
    property alias backLightOpacity: darkone.backLightOpacity
    property alias screenLight: darkone.screenLight
    property alias screenLightOpacity: darkone.screenLightOpacity
    property alias toolbarAutoHide: darkone.toolbarAutoHide
    property alias overlayScale: darkone.overlayScale
    property alias colourScheme: darkone.colourScheme
    property alias videoPlayerVolume: darkone.videoPlayerVolume
    property alias videoAutoPlayTimeout: darkone.videoAutoPlayTimeout

    Rectangle {
        id: darkone
        focus: true
        z: 0
        anchors.fill: parent
        width: DarkoneJS.baseWidth
        height: DarkoneJS.baseHeight

        // global properties
        property bool debug: false
        property bool debug2: false
        property bool debug3: false
        property string version: ""
        property string qtVersion: ""
        property int fps: 0

        // restored properties
        property int lastIndex: -1
        property bool toolbarHidden: false
        property bool listHidden: false
        property bool fullScreen: false
        property bool fpsVisible: false
        property bool sortByName: false
        property bool launchFlash: true
        property bool launchZoom: true
        property string dataTypePrimary: ""
        property string dataTypeSecondary: ""
        property real lightTimeout: 60
        property bool screenLight: true
        property real screenLightOpacity: 0
        property bool backLight: true
        property real backLightOpacity: 0
        property bool toolbarAutoHide: true
        property real overlayScale: 1
        property string colourScheme: "dark"
        property real videoPlayerVolume: 0.5
        property int videoAutoPlayTimeout: -1000

        property bool initialised: false
        property bool activeBorders: false
        property bool ignoreLaunch: false
        property bool dataHidden: true
        property bool keepLightOn: false
        property bool lightOut: true
        property bool lightOutScreen: true
        property string dataTypeCurrent: "title"
        property int zoomDuration: 250
        property int listDuration: 750
        property int overlayDuration: 0
        property int flashes: 4
        property int activeBorderSize: 2
        property bool preferencesLaunchLock: false
        property bool toolbarShowMenuLock: false
        property bool toolbarShowFpsLock: false
        property bool infoMissing: true
        property string colour1: "#000000"
        property string colour2: "#000000"
        property string colour3: "#000000"
        property string colour4: "#000000"
        property string colour5: "#000000"
        property string textColour1: "#000000"
        property string textColour2: "#000000"
        property int itemLeftMargin: 15
        property int itemRightMargin: 15

        color: "black"
        opacity: 0
        state: "off"

        Component.onCompleted: { initTimer.start(); }
        Connections {
            target: viewer;
            onEmulatorStarted: DarkoneJS.gameOn();
            onEmulatorFinished: DarkoneJS.gameOver();
        }

        onToolbarAutoHideChanged: { debug && console.log("toolbarAutoHide: '" + darkone.toolbarAutoHide + "'"); }
        onLastIndexChanged: { debug && console.log("lastIndex: '" + darkone.lastIndex + "'"); }
        onColourSchemeChanged: { DarkoneJS.colourScheme(colourScheme); }
        onStateChanged: { state == "off" ? lightOffAnimation.start() : lightOnAnimation.start(); }
        onLightOutChanged: { debug && console.log("[darkone] darkone.lightOut: '" + darkone.lightOut + ", " +
                                                            "state before: '" + darkone.state + "'");
                             darkone.lightOut ? darkone.state = "off" : darkone.state = "on"; }
        onLightOutScreenChanged: { debug && console.log("[darkone] darkone.lightOutScreen: '" + darkone.lightOutScreen + ", " +
                                                                  "overlayScreen.state orig: '" + overlayScreen.state + "', " +
                                                                  "screenLightOpacity: '" + darkone.screenLightOpacity + "'");
                                  if (darkone.lightOutScreen) {
                                      overlayScreenLight.visible = false;
                                      overlayBackLight.visible = false;
                                      overlayScreen.state = "off"
                                   } else {
                                      if (screenLight)
                                         overlayScreenLight.visible = true;
                                      if (backLight)
                                         overlayBackLight.visible = true;
                                      overlayScreen.state = "on";
                                   } }
        onListHiddenChanged: { darkone.listHidden ? machineListView.state = "hidden" : machineListView.state = "shown"; }
        onToolbarHiddenChanged: { darkone.toolbarHidden ? toolbar.state = "hidden" : toolbar.state = "shown"; }
        onDataHiddenChanged: { darkone.dataHidden ? overlayData.state = "hidden" : overlayData.state = "shown"; }
        onInfoMissingChanged: { darkone.infoMissing ? overlayText.state = "missing" : overlayText.state = "found"; }
        onOverlayScaleChanged: { overlayScaleSliderItem.value = darkone.overlayScale; }
        onFocusChanged: {
            debug2 && console.log("[focus] darkone: '" + focus + "'" );
            debug2 && focus && DarkoneJS.inFocus();
        }
        onActiveFocusChanged: {
            debug2 && console.log("[activeFocus] darkone: '" + activeFocus + "'" );
            debug2 && activeFocus && DarkoneJS.inFocus();
        }

        PropertyAnimation { id: fadeIn; target: darkone; property: "opacity"; duration: 2000; from: 0; to: 1.0; easing.type: Easing.InExpo; }
        SequentialAnimation {
            id: lightOnAnimation
            PropertyAnimation { target: overlayCabinet; property: "opacity"; duration: 150; from: 0; to: 1.0; easing.type: Easing.InExpo; }
            PropertyAnimation { target: overlayCabinet; property: "opacity"; duration: 75; from: 1.0; to: 0.25; easing.type: Easing.InExpo; }
            PropertyAnimation { target: overlayCabinet; property: "opacity"; duration: 75; from: 0.25; to: 1.0; easing.type: Easing.InExpo; }
            PropertyAnimation { target: overlayCabinet; property: "opacity"; duration: 75; from: 1.0; to: 0.25; easing.type: Easing.InExpo; }
            PropertyAnimation { target: overlayCabinet; property: "opacity"; duration: 105; from: 0.25; to: 1.0; easing.type: Easing.InExpo; }
            PropertyAnimation { target: overlayCabinet; property: "opacity"; duration: 250; from: 1.0; to: 0.75; easing.type: Easing.InExpo; }
            PropertyAnimation { target: overlayCabinet; property: "opacity"; duration: 0; from: 0.75; to: 1.0; easing.type: Easing.InExpo; }
            PropertyAnimation { target: overlayStateBlock; property: "opacity"; duration: 5; from: 0.0; to: 1.0; easing.type: Easing.InExpo; }
            PropertyAnimation { target: overlayStateBlock; property: "opacity"; duration: 5; from: 0; to: 0.5; easing.type: Easing.InExpo; }
            PropertyAnimation { target: overlayTextFlick; property: "opacity"; duration: 5; from: 0; to: 1.0; easing.type: Easing.InExpo; }
        }
        SequentialAnimation {
            id: lightOffAnimation
            PropertyAnimation { target: overlayStateBlock; property: "opacity"; duration: 0; from: overlayStateBlock.opacity; to: 0; easing.type: Easing.InExpo; }
            PropertyAnimation { target: overlayCabinet; property: "opacity"; duration: 5; from: 1.0; to: 0.1; easing.type: Easing.InExpo; }
            PropertyAnimation { target: overlayTextFlick; property: "opacity"; duration: 5; from: 1.0; to: 0; easing.type: Easing.InExpo; }
        }

        function lights() {
            if (!darkone.keepLightOn) {
                lightOutTimer.restart();
                lightOutScreenTimer.restart();
            }
            if (darkone.lightOut)
                DarkoneJS.lightToggle(1);
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onPositionChanged: {
                // lights
                darkone.lights();
            }
        }
        // global key events
        Keys.onPressed: {
            debug2 && console.log("[keys] darkone: '" + DarkoneJS.keyEvent2String(event) + "'")

            // lights
            darkone.lights();

            if ( event.modifiers & Qt.AltModifier) {
                switch ( event.key ) {
                    case Qt.Key_Enter:
                    case Qt.Key_Return: {
                        DarkoneJS.fullScreenToggle()
                        event.accepted = true;
                        break;
                    }
                }
            } else if ( event.modifiers & Qt.ControlModifier) {
                if ( event.modifiers & Qt.ShiftModifier ) {
                } else {
                    switch ( event.key ) {
                        case Qt.Key_B: {
                            darkone.activeBorders = !darkone.activeBorders;
                            event.accepted = true;
                            break;
                        }
                        case Qt.Key_1: {
                            debug = !debug;
                            event.accepted = true;
                            break;
                        }
                        case Qt.Key_2: {
                            debug2 = !debug2;
                            event.accepted = true;
                            break;
                        }
                        case Qt.Key_3: {
                            debug3 = !debug3;
                            event.accepted = true;
                            break;
                        }
                        case Qt.Key_L: {
                            if (preferences.state == "hidden")
                                DarkoneJS.listToggle();
                            event.accepted = true;
                            break;
                        }
                        case Qt.Key_Q: {
                            Qt.quit();
                            break;
                        }
                        case Qt.Key_P: {
                            !darkone.ignoreLaunch && DarkoneJS.launch();
                            event.accepted = true;
                            break;
                        }
                        case Qt.Key_O: {
                            preferences.state = preferences.state == "shown" ? "hidden" : "shown";
                            event.accepted = true;
                            break;
                        }
                        case Qt.Key_T: {
                            DarkoneJS.toolbarToggle();
                            event.accepted = true;
                            break;
                        }
                        case Qt.Key_S: {
                            if ( darkone.toolbarHidden )
                                DarkoneJS.toolbarToggle(1);
                            searchTextInput.focus = true;
                            searchTextInput.forceActiveFocus();
                            event.accepted = true;
                            break;
                        }
                        case Qt.Key_V: {
                            if ( videoSnap.playing )
                                videoSnap.stop();
                            else
                                videoSnap.play();
                            break;
                        }
                    }
                }
            } else {
                switch ( event.key ) {
                    case Qt.Key_Backtab:
                    case Qt.Key_Tab: {
                        debug2 && console.log("[keys] tab: \"let me fall through your cracks again\"");
                        overlay.focus = true;
                        event.accepted = true;
                        break;
                    }
                    case Qt.Key_Left: {
                        if (!darkone.listHidden)
                            DarkoneJS.listToggle();
                        event.accepted = true;
                        break;
                    }
                    case Qt.Key_Right: {
                        if (darkone.listHidden && preferences.state == "hidden")
                            DarkoneJS.listToggle();
                        event.accepted = true;
                        break;
                    }
                    case Qt.Key_Up: {
                        if (!darkone.toolbarHidden)
                            DarkoneJS.toolbarToggle();
                        event.accepted = true;
                        break;
                    }
                    case Qt.Key_Down: {
                        if (darkone.toolbarHidden)
                            DarkoneJS.toolbarToggle();
                        event.accepted = true;
                        break;
                    }
                    case Qt.Key_Escape: {
                        if ( preferences.state == "shown" )
                            preferences.state = "hidden";
                        else if ( launchFlashTimer.running ) {
                            launchFlashTimer.stop();
                            DarkoneJS.flashCounter = 0;
                            DarkoneJS.inGame = true; // fake game over
                            DarkoneJS.gameOver();
                        }
                        event.accepted = true;
                        break;
                    }
                    case Qt.Key_F1:
                        break;
                    case Qt.Key_F11: {
                        DarkoneJS.fullScreenToggle()
                        event.accepted = true;
                        break;
                    }
                    case Qt.Key_Plus: {
                        DarkoneJS.zoom(1.1);
                        event.accepted = true;
                        break;
                    }
                    case Qt.Key_Minus: {
                        DarkoneJS.zoom(0.9);
                        event.accepted = true;
                        break;
                    }
                }
            }
        }
        Timer {
            id: initTimer
            interval: 5
            running: false
            repeat: true
            onTriggered: { DarkoneJS.init(); }
        }
        Timer {
            id: resetIgnoreLaunchTimer
            interval: 100
            running: false
            repeat: false
            onTriggered: darkone.ignoreLaunch = false
        }
        Timer {
            //reset overlay width to snap instantantly
            id: resetOverlaySnapTimer
            interval: 1000
            repeat: false
            onTriggered: { darkone.overlayDuration = 0; }
        }
        Timer {
            id: launchFlashTimer
            interval: 750
            running: false
            repeat: true
            onTriggered: DarkoneJS.launchDelay()
        }
        Timer {
            id: launchTimer
            interval: 250
            running: false
            repeat: true
            onTriggered: { if (!overlayScreenScaleAnimation.running) {
                              launchTimer.stop();
                              viewer.launchEmulator(machineListModel[machineListView.currentIndex].id);
                           } } }
        Timer {
            id: hideToolbarTimer
            interval: 4000
            running: false
            repeat: false
            onTriggered: { darkone.toolbarAutoHide && DarkoneJS.toolbarToggle(-1); }
        }
        Timer {
            id: lightOutTimer
            interval: darkone.lightTimeout * 1000
            running: false
            repeat: true
            onTriggered: DarkoneJS.lightToggle(-1);
        }
        Timer {
            id: lightOutScreenTimer
            interval: darkone.lightTimeout * 1000 + 2500
            running: false
            repeat: true
            onTriggered: { darkone.lightOutScreen = true;
                           lightOutScreenTimer.stop(); }
        }
        Timer {
            id: videoAutoPlayTimer
            interval: darkone.videoAutoPlayTimeout
            running: false
            repeat: false
            onTriggered: videoSnap.play()
        }


/***
 * overlay
 */
        Rectangle {
            id: overlay
            focus: true // darkoneFocusScope
            z: 0
            opacity: debug ? 0.25 : 1.0
            width: (DarkoneJS.overlayWidth() - 15 - 15)
            border.color: debug ? "red" : "transparent"
            border.width: debug ? 2 : 0
            color: debug ? "white" : "transparent"
            anchors.top: parent.top
            anchors.topMargin: 10
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 15

            Behavior on width { PropertyAnimation { duration: darkone.overlayDuration; easing.type: Easing.InOutQuad } }

            onFocusChanged: {
                if ( darkone.initialised ) {
                    (debug2 || debug3) && console.log("[focus] overlay: '" + focus + "'" );
                    (debug2 || debug3) && focus && DarkoneJS.inFocus();
                }
                if ( focus ) {
                    DarkoneJS.focus("overlay");
                    overlayScreen.focus = true;
                }
            }
            onActiveFocusChanged: {
                if ( darkone.initialised ) {
                    debug2 && console.log("[activeFocus] overlay: '" + activeFocus + "'" );
                    debug2 && activeFocus && DarkoneJS.inFocus();
                }
                if ( activeFocus )
                    overlayScreen.focus = true;
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    debug && console.log("[overlay] clicked");
                    overlay.focus = true;
                }
            }
            WheelArea {
                anchors.fill: parent
                onWheel: {
                           DarkoneJS.zoom(1 + (0.1) * (delta / Math.abs(delta)));
                           debug && console.log("[overlay] wheel event: darkone.overlayScale: '" + darkone.overlayScale + "', " +
                                                                       "zoom: '" + (1 + (0.1) * (delta / Math.abs(delta))) + "'");
                }
            }
            // overlay key events
            Keys.onPressed: {
                debug2 && console.log("[keys] overlay: '" + DarkoneJS.keyEvent2String(event) + "'")

                // lights
                darkone.lights();

                switch( event.key ) {
                    case Qt.Key_1: {
                        if ( ! darkone.dataHidden )
                            darkone.dataTypePrimary = darkone.dataTypeCurrent
                        event.accepted = true;
                        break;
                    }
                    case Qt.Key_2: {
                        if ( ! darkone.dataHidden )
                            darkone.dataTypeSecondary = darkone.dataTypeCurrent
                        event.accepted = true;
                        break;
                    }
                    case Qt.Key_F: {
                        if ( event.modifiers & Qt.AltModifier ) {
                            DarkoneJS.fullScreenToggle()
                            event.accepted = true;
                        }
                        break;
                    }
                    case Qt.Key_Enter:
                    case Qt.Key_Return: {
                        darkone.dataHidden = !darkone.dataHidden
                        if ( ! darkone.dataHidden )
                           overlayDataTypeCycleItem.focus = true;
                        else
                           overlay.focus = true;
                        event.accepted = true;
                        break;
                    }
                    case Qt.Key_P: {
                        !darkone.ignoreLaunch && DarkoneJS.launch();
                        event.accepted = true;
                        break;
                    }
                    case Qt.Key_Backtab: {
                        if ( !darkone.toolbarHidden ) {
                            toolbarFocusScope.focus = true;
                            event.accepted = true;
                        } else if ( !darkone.listHidden ) {
                            machineListView.focus = true;
                            event.accepted = true;
                        }
                        break;
                    }
                    case Qt.Key_Tab: {
                        if ( !darkone.listHidden ) {
                            machineListView.focus = true;
                            event.accepted = true;
                        } else if ( !darkone.toolbarHidden ) {
                            toolbarFocusScope.focus = true;
                            event.accepted = true;
                        }
                        break;
                    }
                    case Qt.Key_PageUp: {
                        if ( event.modifiers & Qt.ControlModifier) {
                            if ( overlayText.text != "" ) {
                                // scroll page
                                overlayTextFlick.contentY += overlayTextFlick.height / 10 * 9
                                overlayTextFlick.returnToBounds();
                                event.accepted = true;
                            }
                        }
                        break;
                    }
                    case Qt.Key_PageDown: {
                        if ( event.modifiers & Qt.ControlModifier) {
                            if ( overlayText.text != "" ) {
                                // scroll page
                                overlayTextFlick.contentY -= overlayTextFlick.height / 10 * 9
                                overlayTextFlick.returnToBounds();
                                event.accepted = true;
                            }
                        }
                        break;
                    }
                    case Qt.Key_Up: {
                        if ( event.modifiers & Qt.ControlModifier) {
                            if ( event.modifiers & Qt.ShiftModifier ) {
                                DarkoneJS.zoom(1.1);
                                event.accepted = true;
                                break;
                            } else {
                                if ( overlayText.text != "" ) {
                                    // scroll page
                                    overlayTextFlick.contentY += overlayTextFlick.height / 10 * 9
                                    overlayTextFlick.returnToBounds();
                                    event.accepted = true;
                                } else {
                                    if ( !darkone.toolbarHidden ) {
                                        toolbarFocusScope.focus = true;
                                        event.accepted = true;
                                    } else if ( !darkone.listHidden ) {
                                        machineListView.focus = true;
                                        event.accepted = true;
                                    }
                                }
                                break;
                            }
                        } else {
                            if ( overlayText.text != "" ) {
                                // scroll line
                                overlayTextFlick.contentY += overlayTextFlick.height / 10
                                overlayTextFlick.returnToBounds();
                                event.accepted = true;
                            }
                            break;
                        }
                    }
                    case Qt.Key_Down: {
                        if ( event.modifiers & Qt.ControlModifier) {
                            if ( event.modifiers & Qt.ShiftModifier ) {
                                DarkoneJS.zoom(0.9);
                                event.accepted = true;
                                break;
                            } else {
                                if ( overlayText.text != "" ) {
                                    // scroll page
                                    overlayTextFlick.contentY -= overlayTextFlick.height / 10 * 9
                                    overlayTextFlick.returnToBounds();
                                    event.accepted = true;
                                } else {
                                    if ( !darkone.listHidden ) {
                                        machineListView.focus = true;
                                        event.accepted = true;
                                    } else if ( !darkone.toolbarHidden ) {
                                        toolbarFocusScope.focus = true;
                                        event.accepted = true;
                                    }
                                }
                                break;
                            }
                        } else {
                            if ( overlayText.text != "" ) {
                                // scroll line
                                overlayTextFlick.contentY -= overlayTextFlick.height / 10
                                overlayTextFlick.returnToBounds();
                                event.accepted = true;
                            }
                            break;
                        }
                    }
                    case Qt.Key_Left:
                        if ( event.modifiers & Qt.ControlModifier ) {
                            if ( !darkone.toolbarHidden ) {
                                toolbarFocusScope.focus = true;
                                event.accepted = true
                            } else if ( !darkone.listHidden ) {
                                machineListView.focus = true;
                                event.accepted = true
                            }
                        } else {
                            if ( ! darkone.dataHidden )
                                event.accepted = true
                        }
                        break;
                    case Qt.Key_Right: {
                        if ( event.modifiers & Qt.ControlModifier ) {
                            if ( !darkone.listHidden ) {
                                machineListView.focus = true;
                                event.accepted = true
                            } else if ( !darkone.toolbarHidden ) {
                                toolbarFocusScope.focus = true;
                                event.accepted = true
                            }
                        } else {
                            if ( ! darkone.dataHidden )
                                event.accepted = true
                        }
                        break;
                    }
                    default: {
                    }
                }
            }

/***
 * screen
 */
            Rectangle {
                id: overlayScreen
                z: 1
                width: 354
                height: 259 - 2 * (darkone.activeBorderSize + 1)
                scale: DarkoneJS.scaleFactorY() * darkone.overlayScale
                // keep the screen still under scaling. then ensure margin of 30% of surrounding space. adjusted to account for screen border
                property real scaleOffsetReal: ((scale * (height + 2 * (darkone.activeBorderSize + 1))) - (height + 2 * (darkone.activeBorderSize + 1))) / 2
                property real surroundOffsetReal: darkone.height - 20 - (height * scale)
                anchors.top: parent.top
                anchors.topMargin: scaleOffsetReal + 0.3 * surroundOffsetReal - ((darkone.activeBorderSize + 1) * scale)
                anchors.horizontalCenter: parent.horizontalCenter
                smooth: true
                opacity: 1.0
                border.color: debug ? "blue" : "black"
                border.width: debug ? 2 : 1
                color: debug ? "white" : "#181818"
                state: "on"

                Behavior on scale { PropertyAnimation { id: "overlayScreenScaleAnimation"; properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear } }
                transitions: [
                    Transition {
                        from: "on"
                        to: "off"
                        ParallelAnimation {
                            PropertyAnimation { target: overlayScreen; property: "opacity"; from: 1.0; to: 0.1; duration: 500; easing.type: Easing.OutExpo }
                            PropertyAnimation { target: overlayBackLight; property: "opacity"; from: darkone.screenLightOpacity; to: 0; duration: 500; easing.type: Easing.OutExpo }

                        }
                    },
                    Transition {
                        from: "off"
                        to: "on"
                        ParallelAnimation {
                            PropertyAnimation { target: overlayScreen; property: "opacity"; from: 0.1; to: 1.0; duration: 500; easing.type: Easing.OutExpo }
                            PropertyAnimation { target: overlayBackLight; property: "opacity"; from: 0; to: darkone.screenLightOpacity; duration: 500; easing.type: Easing.OutExpo }
                        }
                    }
                ]

                onFocusChanged: {
                    if ( darkone.initialised ) {
                        debug2 && console.log("[focus] overlayScreen: '" + focus + "'" );
                        debug2 && focus && DarkoneJS.inFocus();
                    }
                    if ( !darkone.dataHidden )
                        overlayDataTypeCycleItem.focus = true;
                }
                onActiveFocusChanged: {
                    if ( darkone.initialised ) {
                        debug2 && console.log("[activeFocus] overlayScreen: '" + activeFocus + "'" );
                        debug2 && activeFocus && DarkoneJS.inFocus();
                    }
                    if ( !darkone.dataHidden )
                        overlayDataTypeCycleItem.focus = true;
                }
                onStateChanged: { debug && console.log("[overlayScreen] state changed, state: '" + state + "', " +
                                                                       "screenLightOpacity: '" + darkone.screenLightOpacity + "'"); }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        parent.forceActiveFocus();
                    }
                    onDoubleClicked: {
                        debug && console.log("[overlayScreen] double-clicked");
                        darkone.dataHidden = !darkone.dataHidden;
                        if ( ! darkone.dataHidden )
                            overlayDataTypeCycleItem.focus = true;
                        else
                            overlay.focus = true;
                    }
                    onEntered: { debug && console.log("[overlayScreen] entered"); }
                    onPositionChanged: {
                        // lights
                        darkone.lights();
                    }
                }
                Keys.onPressed: {
                    debug2 && console.log("[keys] overlayScreen: '" + DarkoneJS.keyEvent2String(event) + "'")
                }

                Rectangle {
                    id: overlayScreenBorderTop
                    z: parent.z + 15
                    anchors.top: parent.top
                    anchors.topMargin: -(darkone.activeBorderSize + 1)
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width
                    height: (darkone.activeBorderSize + 1)
                    visible: (parent.focus || overlayDataTypeCycleItem.focus) && darkone.activeBorders
                    color: darkone.textColour2
                }
                Rectangle {
                    id: overlayScreenBorderBottom
                    z: parent.z + 15
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: -(darkone.activeBorderSize + 1)
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width
                    height: (darkone.activeBorderSize + 1)
                    visible: (parent.focus || overlayDataTypeCycleItem.focus) && darkone.activeBorders
                    color: darkone.textColour2
                }

/***
 * display
 */
                Rectangle {
                    id: overlayDisplay
                    z: 2 // must be above other layers (in the same hierarchy!) for flickable actions
                    width: parent.width
                    height: parent.height
                    anchors.fill: parent
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.horizontalCenterOffset: 0
                    opacity: 1.0
                    smooth: true
                    state: "shown"
                    color: "transparent"

                    Behavior on anchors.topMargin { PropertyAnimation { duration: darkone.zoomDuration; easing.type: Easing.Linear } }
                    Behavior on anchors.bottomMargin { PropertyAnimation { duration: darkone.zoomDuration; easing.type: Easing.Linear } }
                    CursorShapeArea {
                        anchors.fill: parent
                        cursorShape: videoSnap.playing ? Qt.ArrowCursor : Qt.CrossCursor
                    }
                    MouseArea {
                        anchors.fill: parent
                        onDoubleClicked: { debug && console.log("[overlayDisplay] double-clicked");
                                           darkone.dataHidden = !darkone.dataHidden; }
                    }

                    Image {
                        id: overlayImage
                        z: 0
                        source: DarkoneJS.data("image")
                        cache: false
                        smooth: true
                        anchors.fill: parent
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectFit
                        opacity: videoSnap.playing ? 0 : 1
                        MouseArea {
                            anchors.fill: parent
                            onDoubleClicked: { debug && console.log("[overlayImage] double-clicked");
                                               darkone.dataHidden = !darkone.dataHidden; }
                        }
                        WheelArea {
                            anchors.fill: parent
                            onWheel: {
                                DarkoneJS.zoom(1 + (0.1) * (delta / Math.abs(delta)));
                            }
                        }
                        Connections {
                            target: viewer
                            onImageDataUpdated: {
                                if ( cachePrefix === DarkoneJS.dataTypes[darkone.dataTypeCurrent].path )
                                    overlayImage.cache = true;
                            }
                        }
                    }
                    Rectangle {
                        id: overlayTextWrap
                        z: 1
                        color: debug ? "blue" : "transparent"
                        anchors.fill: parent
                        height: parent.height
                        width: parent.width

                        Flickable {
                            id: overlayTextFlick
                            interactive: true
                            contentHeight: Math.max(parent.height, overlayText.paintedHeight)  // no flickable without this set. vertical alignment dependent on this also
                            anchors.fill: parent
                            anchors.topMargin: 5
                            anchors.bottomMargin: 5
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            boundsBehavior: Flickable.StopAtBounds
                            flickableDirection: Flickable.VerticalFlick
                            clip: true
                            maximumFlickVelocity : 100

                            MouseArea {
                                anchors.fill: parent
                                onDoubleClicked: {
                                    debug && console.log("[overlayTextFlick] double-clicked");
                                    darkone.dataHidden = !darkone.dataHidden
                                    if ( ! darkone.dataHidden )
                                        overlayDataTypeCycleItem.focus = true;
                                    else
                                        overlay.focus = true;
                                }
                            }
                            WheelArea {
                                anchors.fill: parent
                                onWheel: {
                                    DarkoneJS.zoom(1 + (0.1) * (delta / Math.abs(delta)));
                                }
                            }

                            Text {
                                id: overlayText
                                anchors.fill: parent
                                text: DarkoneJS.data("text")
                                textFormat: Text.RichText
                                color: "white"
                                font.bold: false
                                font.pixelSize: 8
                                verticalAlignment: Text.AlignTop
                                horizontalAlignment: Text.AlignLeft
                                wrapMode: Text.WordWrap
                                smooth: true
                                state: "found"

                                states: [
                                    State {
                                        name: "found"
                                        PropertyChanges { target: overlayText; verticalAlignment: Text.AlignTop }
                                        PropertyChanges { target: overlayText; horizontalAlignment: Text.AlignLeft }
                                    },
                                    State {
                                        name: "missing"
                                        PropertyChanges { target: overlayText; verticalAlignment: Text.AlignVCenter }
                                        PropertyChanges { target: overlayText; horizontalAlignment: Text.AlignHCenter }
                                    }
                                ]
                            }
                        }
                    }
                    Image {
                        id: videoIndicator
                        anchors.fill: parent
                        anchors.centerIn: parent
                        anchors.margins: 1
                        z: 3
                        source: "images/movie.png"
                        fillMode: Image.PreserveAspectFit
                        scale: 0.33
                        smooth: true
                        opacity: 0
                        CursorShapeArea {
                            anchors.fill: parent
                            cursorShape: videoIndicator.opacity > 0 ? Qt.ArrowCursor: Qt.CrossCursor
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                videoAutoPlayTimer.stop();
                                if ( videoSnap.playing )
                                    videoSnap.stop();
                                else
                                    videoSnap.play();
                            }
                        }
                    }
                    Video {
                        id: videoSnap
                        anchors.fill: parent
                        anchors.centerIn: parent
                        anchors.margins: 1
                        z: 3
                        fillMode: Video.Stretch
                        autoLoad: false
                        opacity: playing ? 1 : 0
                        property string videoUrl: darkone.initialised ? viewer.videoSnapUrl(machineListModel[machineListView.currentIndex].id) : ""
                        source: videoUrl
                        volume: darkone.videoPlayerVolume
                        onVideoUrlChanged: {
                            videoAutoPlayTimer.stop();
                            videoSnap.stop();
                            if ( videoSnap.videoUrl == "" )
                                videoIndicator.opacity = 0;
                            else {
                                videoIndicator.opacity = 0.4;
                                if ( darkone.videoAutoPlayTimeout >= 0 )
                                    videoAutoPlayTimer.start();
                            }
                        }
                        MouseArea {
                            enabled: videoSnap.playing
                            anchors.fill: parent
                            onClicked: {
                                videoAutoPlayTimer.stop();
                                if ( videoSnap.playing )
                                    videoSnap.stop();
                            }
                        }
                    }
                } // overlayDisplay

                Rectangle {
                    id: overlayData
                    z: 1
                    width: parent.width
                    height: parent.height
                    anchors.fill: parent
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.horizontalCenterOffset: 0
                    opacity: 1.0
                    state: "hidden"
                    color: "transparent"

                    Behavior on scale { PropertyAnimation { properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear } }
                    transitions: [
                        Transition {
                            from: "hidden"
                            to: "shown"
                            SequentialAnimation {
                                ParallelAnimation {
                                    PropertyAnimation { target: overlayDisplay; property: "anchors.topMargin"; from: 0; to: overlayDataHeader.height; duration: 450; easing.type: Easing.InExpo }
                                    PropertyAnimation { target: overlayDataHeader; property: "anchors.topMargin"; from: -overlayDataHeader.height - 5; to: 0; duration: 450; easing.type: Easing.InExpo }
                                    PropertyAnimation { target: overlayDataHeader; property: "opacity"; from: 0; to: 1.0; duration: 450; easing.type: Easing.InExpo }

                                    PropertyAnimation { target: overlayDisplay; property: "anchors.bottomMargin"; from: 0; to: overlayDataNav.height; duration: 450; easing.type: Easing.InExpo }
                                    PropertyAnimation { target: overlayDataNav; property: "anchors.bottomMargin"; from: -overlayDataNav.height - 5; to: 0; duration: 450; easing.type: Easing.InExpo }
                                    PropertyAnimation { target: overlayDataNav; property: "opacity"; from: 0; to: 1.0; duration: 450; easing.type: Easing.OutExpo }
                                }
                            }
                        },
                        Transition {
                            from: "shown"
                            to: "hidden"
                            SequentialAnimation {
                                ParallelAnimation {
                                    PropertyAnimation { target: overlayDisplay; property: "anchors.topMargin"; from: overlayDataHeader.height; to: 0; duration: 450; easing.type: Easing.OutExpo }
                                    PropertyAnimation { target: overlayDataHeader; property: "anchors.topMargin"; from: 0; to: -overlayDataHeader.height - 5; duration: 450; easing.type: Easing.OutExpo }
                                    PropertyAnimation { target: overlayDataHeader; property: "opacity"; from: 1.0; to: 0; duration: 450; easing.type: Easing.OutExpo }

                                    PropertyAnimation { target: overlayDisplay; property: "anchors.bottomMargin"; from: overlayDataNav.height; to: 0; duration: 450; easing.type: Easing.OutExpo }
                                    PropertyAnimation { target: overlayDataNav; property: "anchors.bottomMargin"; from: 0; to: -overlayDataNav.height - 5; duration: 450; easing.type: Easing.OutExpo }
                                    PropertyAnimation { target: overlayDataNav; property: "opacity"; from: 1.0; to: 0; duration: 450; easing.type: Easing.OutExpo }
                                }
                            }
                        }
                    ]

                    CursorShapeArea {
                        anchors.fill: parent
                        cursorShape: Qt.CrossCursor
                    }

                    Rectangle {
                        id: overlayDataHeader
                        height: overlayDataTitle.paintedHeight + 10
                        width: parent.width
                        anchors.top: parent.top
                        anchors.horizontalCenter: parent.horizontalCenter
                        opacity: 0
                        smooth: true
                        color: debug ? "yellow" : "transparent"

                        MouseArea {
                            anchors.fill: parent
                            onClicked: { darkone.dataHidden = !darkone.dataHidden; }
                        }

                        Text {
                              id: overlayDataTitle
                              text: DarkoneJS.gameCardHeader()
                              textFormat: Text.RichText
                              font.pixelSize: 10
                              anchors.top: parent.top
                              anchors.topMargin: 5
                              anchors.horizontalCenter: parent.horizontalCenter
                              width: parent.width - 20
                              horizontalAlignment: Text.AlignHCenter
                              verticalAlignment: Text.AlignVCenter
                              color: "white"
                              wrapMode: Text.WordWrap

                              onTextChanged: { debug &&  console.log("[overlayDataHeader] changed");
                                               parent.height = paintedHeight + 5;  // force update
                                               if (!darkone.dataHidden)
                                                   overlayDisplay.anchors.topMargin = paintedHeight + 5
                                             } // force update
                        }
                    }
                    Rectangle {
                        id: overlayDataNav
                        height: 30
                        width: parent.width
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottom: parent.bottom
                        color: debug ? "red" : parent.color
                        opacity: 0
                        smooth: true

                        MouseArea {
                            anchors.fill: parent
                            onClicked: { debug && console.log("[overlayDataNav] clicked"); }
                        }

                        Rectangle {
                            id: overlayDataNavSeparator
                            height: 1
                            width: parent.width * 0.5
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.top: parent.top
                            anchors.topMargin: 5
                            color: "white"
                            opacity: 0.75
                            smooth: true
                        }
                        Rectangle {
                            id: overlayDataTypeSetPrimaryButton
                            height: 14
                            width: 10
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 5
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            color: "white"
                            border.width: 2
                            border.color: "black"
                            opacity: 0.75

                            CursorShapeArea {
                                anchors.fill: parent
                                cursorShape: Qt.CrossCursor
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: { darkone.dataTypePrimary = darkone.dataTypeCurrent;
                                             debug && console.log("[overlayDataTypeSetPrimaryButton clicked]"); }
                            }

                            Text {
                                id: overlayDataTypeSetPrimaryText
                                text: "1"
                                color: "black"
                                font.bold: true
                                font.pixelSize: 8
                                anchors.fill: parent
                                anchors.centerIn: parent
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                smooth: true
                            }
                        }
                        Rectangle {
                            id: overlayDataTypeSetSecondaryButton
                            height: 14
                            width: 10
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 5
                            anchors.left: overlayDataTypeSetPrimaryButton.right
                            anchors.leftMargin: 3
                            color: "white"
                            border.width: 2
                            border.color: "black"
                            opacity: 0.75

                            CursorShapeArea {
                                anchors.fill: parent
                                cursorShape: Qt.CrossCursor
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: { darkone.dataTypeSecondary = darkone.dataTypeCurrent;
                                             debug && console.log("[overlayDataTypeSetSecondaryButton clicked]"); }
                            }

                            Text {
                                id: overlayDataTypeSetSecondaryText
                                text: "2"
                                color: "black"
                                font.bold: true
                                font.pixelSize: 8
                                anchors.fill: parent
                                anchors.centerIn: parent
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                smooth: true
                            }
                        }
                        CycleItem {
                            id: overlayDataTypeCycleItem
                            property int index: prefsBackendText.index + 1
                            anchors.fill: parent
                            anchors.topMargin: 5 + 1
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.leftMargin: parent.width / 4
                            anchors.rightMargin: parent.width / 4
                            height: parent.itemHeight
                            width: parent.width / 2
                            textSize: 12
                            textColour: "white"
                            items: DarkoneJS.datatypeKeys();
                            image: "../images/arrow.png"
                            imageWidth: 16
                            imageRotation: 0
                            activeColour: "transparent"
                            passKeyEvents: true

                            Component.onCompleted: {
                                value = darkone.dataTypeCurrent;
                                text = DarkoneJS.data("name");
                            }

                            onValueChanged: {
                                darkone.dataTypeCurrent = value;
                                text = DarkoneJS.data("name");
                            }
                            onFocusChanged: {
                                debug2 && console.log("[focus] overlayDataCycleItem: '" + focus + "'" );
                                debug2 && focus && DarkoneJS.inFocus();
                            }
                            onActiveFocusChanged: {
                                debug2 && console.log("[activeFocus] overlayDataCycleItem: '" + activeFocus + "'" );
                                debug2 && activeFocus && DarkoneJS.inFocus();
                            }

                            Keys.onPressed: {
                                debug2 && console.log("[keys] overlayDataTypeCycleItem: '" + DarkoneJS.keyEvent2String(event) + "'")
                            }
                        }
                    }
                } // overlayData
            } // overlayScreen

/***
 * cabinet
 */
            Image {
                id: overlayCabinet
                z: 2
                source: "images/cabinet.png"
                fillMode: Image.PreserveAspectFit
                scale: DarkoneJS.scaleFactorY() * darkone.overlayScale
                // keep the item still under scaling relative to screen, shift it by the same amount as the screen is shifted in its surrounding space, then offet the image to match the screen position. adjusted to account for screen border
                property real scaleOffsetReal: ((scale * height) - height) / 2
                property real imageOffsetReal: 554
                anchors.top: parent.top
                anchors.topMargin: scaleOffsetReal + 0.3 * (overlayScreen.surroundOffsetReal - 2 * (darkone.activeBorderSize + 1)) - (imageOffsetReal * scale) - ((darkone.activeBorderSize + 1) * scale)
                anchors.horizontalCenter: overlayScreen.horizontalCenter
                anchors.horizontalCenterOffset: 0 - 1.5 * darkone.overlayScale // manual screen/cabinet alignment tweaks
                smooth: true
                opacity: 1.0

                Behavior on scale { ParallelAnimation {
                                      PropertyAnimation { properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear }
                                      PropertyAnimation { target: overlayBackLight; properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear } } }
            }
            Image {
                z: 5
                id: overlayScreenLight
                source: "images/screenlight.png"
                fillMode: Image.PreserveAspectFit
                scale: overlayCabinet.scale
                // keep the item still under scaling relative to screen, shift it by the same amount as the screen is shifted in its surrounding space , then offet the image to match the screen position. adjusted to account for screen border
                property real scaleOffsetReal: ((scale * height) - height) / 2
                property real imageOffsetReal: 554
                anchors.top: parent.top
                anchors.topMargin: scaleOffsetReal + 0.3 * (overlayScreen.surroundOffsetReal -2 * (darkone.activeBorderSize + 1)) - (imageOffsetReal * scale) - ((darkone.activeBorderSize + 1) * scale)
                anchors.horizontalCenter: overlayScreen.horizontalCenter
                anchors.horizontalCenterOffset: overlayCabinet.anchors.horizontalCenterOffset
                smooth: true
                opacity: darkone.screenLight ? darkone.screenLightOpacity : 0
            }
            Image {
                z: 0
                id: overlayBackLight
                source: "images/backlight.png"
                fillMode: Image.PreserveAspectFit
                scale: overlayCabinet.scale
                // keep the item still under scaling relative to screen, shift it by the same amount as the screen is shifted in its surrounding space , then offet the image to match the screen position. adjusted to account for screen border
                property real scaleOffsetReal: ((scale * height) - height) / 2
                property real imageOffsetReal: 554
                anchors.top: parent.top
                anchors.topMargin: scaleOffsetReal + 0.3 * (overlayScreen.surroundOffsetReal -2 * (darkone.activeBorderSize + 1)) - (imageOffsetReal * scale) - ((darkone.activeBorderSize + 1) * scale)
                anchors.horizontalCenter: overlayScreen.horizontalCenter
                anchors.horizontalCenterOffset: overlayCabinet.anchors.horizontalCenterOffset
                smooth: true
                opacity: darkone.backLight ? darkone.backLightOpacity : 0
            }
            Rectangle {
                id: overlayStateBlock
                z: 0
                width: 150
                height: 275
                scale: DarkoneJS.scaleFactorY() * darkone.overlayScale
                anchors.top: parent.top
                anchors.topMargin: (((scale * height) - height) / 2) + 0.3 * (darkone.height - 20 - (overlayScreen.height * overlayScreen.scale)) + (300 * scale)
                anchors.horizontalCenter: overlayCabinet.horizontalCenter
                anchors.horizontalCenterOffset: 0
                color: DarkoneJS.gameStatusColour()
                smooth: true
                opacity: 0.0

                Behavior on scale { PropertyAnimation { properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear } }
            }
            Rectangle {
                id: overlayButtonBlock
                z: 2
                width: 25
                height: 35
                scale: DarkoneJS.scaleFactorY() * darkone.overlayScale
                anchors.top: parent.top
                anchors.topMargin: (((scale * height) - height) / 2) + 0.3 * (darkone.height - 20 - (overlayScreen.height * overlayScreen.scale)) + (319 * scale)
                anchors.horizontalCenter: overlayCabinet.horizontalCenter
                anchors.horizontalCenterOffset: 2 * darkone.overlayScale
                color: debug ? "white" : "transparent"

                Behavior on scale { PropertyAnimation { properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear } }

                CursorShapeArea {
                    anchors.fill: parent
                    cursorShape: Qt.CrossCursor
                }
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: { overlayStateBlock.opacity = (lightOnAnimation.running || darkone.lightOut) ? 0 : 1.0; }
                    onExited: { overlayStateBlock.opacity = (lightOnAnimation.running || darkone.lightOut) ? 0 : 0.5; }
                    onClicked: { if (!darkone.ignoreLaunch) {
                                     machineListView.positionViewAtIndex(darkone.lastIndex, ListView.Center);
                                     DarkoneJS.launch(); }
                    }
                }
            }
            Rectangle {
                id: overlayGridBlock
                z: 2
                width: 120
                height: 40
                scale: DarkoneJS.scaleFactorY() * darkone.overlayScale
                anchors.top: parent.top
                anchors.topMargin: (((scale * height) - height) / 2) + 0.3 * (darkone.height - 20 - (overlayScreen.height * overlayScreen.scale)) + (500 * scale)
                anchors.horizontalCenter: overlayCabinet.horizontalCenter
                anchors.horizontalCenterOffset: 1 * darkone.overlayScale
                color: debug ? "white" : "transparent"

                Behavior on scale { PropertyAnimation { properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear } }
                CursorShapeArea {
                    anchors.fill: parent
                    cursorShape: Qt.CrossCursor
                }
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: { overlayStateBlock.opacity = (lightOnAnimation.running || darkone.lightOut) ? 0 : 1.0; }
                    onExited: { overlayStateBlock.opacity = (lightOnAnimation.running || darkone.lightOut) ? 0 : 0.5; }
                    onClicked: { if (!darkone.ignoreLaunch) {
                                     machineListView.positionViewAtIndex(darkone.lastIndex, ListView.Center);
                                     DarkoneJS.launch(); }
                    }
                }
            }
        } // overlay


/***
* list
*/
        Rectangle {
            id: machineListViewBorder
            z: machineListView.z + 15
            anchors.left: darkone.left
            anchors.leftMargin: 8
            anchors.top: darkone.top
            anchors.topMargin: machineListView.itemHeight + 1
            width: 2
            height: machineListView.height - 2
            color: darkone.textColour2
            visible: machineListView.activeFocus && darkone.activeBorders
        }

        ListView {
            id: machineListView
            focus: false // darkoneFocusScope
            property int itemHeight: 24
            z: 3
            height: parent.height - (darkone.toolbarHidden ? 2 : toolbar.height) - machineListView.itemHeight - machineListView.itemHeight
            width: DarkoneJS.listWidth()
            anchors.top: parent.top
            anchors.topMargin: machineListView.itemHeight
            anchors.left: parent.left
            anchors.leftMargin: 15
            model: machineListModel
            state: "shown"
            spacing: 10
            clip: true
            orientation: ListView.Vertical
            flickableDirection: Flickable.VerticalFlick
            flickDeceleration: 1000
            maximumFlickVelocity: 5000
            interactive: true
            keyNavigationWraps: false
            preferredHighlightBegin: (height / 2) - (machineListView.itemHeight / 2)
            preferredHighlightEnd: (height / 2) + (machineListView.itemHeight / 2)

            states: [
                State {
                    name: "hidden"
                },
                State {
                    name: "shown"
                }
            ]

            transitions: [
                //note: here we jump through hoops for an issue where by if a list is hidden in non-fullscreen, and
                //then fullscreen is enabled, the leftMargin isn't updated and so the (hidden) list is partially viewable.
                //the clean way is to switch anchors from 'left against parent.left' to 'right against parent.left',
                //but setting left/right anchors to 'undefined' in a state doesn't seem to work. the setting doesn't
                //take effect, and hence the list is squashed out of existence instead of moved which looks bad
                //a massive negitive margin would work fine but that's a very bad hack. so as it stands here, opacity
                //comes to the rescue. it seems natural to set hidden elements to transparent, but just remember the above
                //issue means that the hidden list can become technically unhidden (unhidden but invisible)
                Transition {
                    from: "hidden"
                    to: "shown"
                    SequentialAnimation {
                        // ensure correct initial position
                        PropertyAnimation { target: machineListView; property: "anchors.leftMargin"; from: anchors.leftMargin; to: -5 - DarkoneJS.listWidth(); duration: 0; easing.type: Easing.Linear }
                        PropertyAnimation { target: searchBox; property: "anchors.leftMargin"; from: machineListView.anchors.leftMargin; to: -5 - DarkoneJS.listWidth(); duration: 0; easing.type: Easing.Linear }
                        // show list / search box
                        PropertyAnimation { target: machineListView; property: "opacity"; from: 0; to: 1.0; duration: 0; }
                        PropertyAnimation { target: searchBox; property: "opacity"; from: 0; to: 1.0; duration: 0; }
                        // animate
                        ParallelAnimation {
                            PropertyAnimation { target: machineListView; property: "anchors.leftMargin"; from: -5 - DarkoneJS.listWidth(); to: darkone.itemLeftMargin; duration: darkone.listDuration; easing.type: Easing.InOutQuad }
                            PropertyAnimation { target: searchBox; property: "anchors.leftMargin"; from: -5 - DarkoneJS.listWidth(); to: darkone.itemLeftMargin; duration: darkone.listDuration; easing.type: Easing.InOutQuad }
                            PropertyAnimation { target: showListButton; property: "anchors.leftMargin"; from: darkone.itemLeftMargin; to: darkone.itemLeftMargin + DarkoneJS.listWidth() + 15; duration: darkone.listDuration; easing.type: Easing.InOutQuad; } }
                        // show borders
                        PropertyAnimation { target: machineListViewBorder; property: "opacity"; from: 0; to: 1.0; duration: 0; }
                    } },
                Transition {
                    from: "shown"
                    to: "hidden"
                    SequentialAnimation {
                        // hide border
                        PropertyAnimation { target: machineListViewBorder; property: "opacity"; from: 1.0; to: 0; duration: 0; }
                        // ensure correct initial position
                        PropertyAnimation { target: machineListView; property: "anchors.leftMargin"; from: anchors.leftMargin; to: darkone.itemLeftMargin + DarkoneJS.listWidth(); duration: 0; easing.type: Easing.Linear }
                        PropertyAnimation { target: searchBox; property: "anchors.leftMargin"; from: machineListView.anchors.leftMargin; to: darkone.itemLeftMargin + DarkoneJS.listWidth(); duration: 0; easing.type: Easing.Linear }
                        // animate
                        ParallelAnimation {
                            PropertyAnimation { target: machineListView; property: "anchors.leftMargin"; from: darkone.itemLeftMargin; to: -5 - DarkoneJS.listWidth(); duration: darkone.listDuration; easing.type: Easing.InOutQuad }
                            PropertyAnimation { target: searchBox; property: "anchors.leftMargin"; from: darkone.itemLeftMargin; to: -5 - DarkoneJS.listWidth(); duration: darkone.listDuration; easing.type: Easing.InOutQuad }
                            PropertyAnimation { target: showListButton; property: "anchors.leftMargin"; from: darkone.itemLeftMargin + DarkoneJS.listWidth() + 15; to: darkone.itemLeftMargin; duration: darkone.listDuration; easing.type: Easing.InOutQuad; } }
                        // hide list / search box
                        PropertyAnimation { target: machineListView; property: "opacity"; from: 1.0; to: 0; duration: 0; }
                        PropertyAnimation { target: searchBox; property: "opacity"; from: 1.0; to: 0; duration: 0; }
                } }
            ]

            function firstVisibleItem() { return - Math.floor(((height / 2) / (machineListView.itemHeight + 10))); } // relatives 'work'
            function lastVisibleItem() { return + Math.floor(((height / 2) / (machineListView.itemHeight + 10))); } // relatives 'work'
            function itemsPerPage() { debug && console.log("contentX: '" + contentX + "', " +
                                                           "contentY: '" + contentY + "', " +
                                                           "firstVisibleItem: '" + firstVisibleItem() + "', " +
                                                           "lastVisibleItem: '" + lastVisibleItem() + "', " +
                                                           "itemsPerPage: '" + height / (machineListView.itemHeight + 10) + "'");
                                          return lastVisibleItem() - firstVisibleItem() + 1 }
            function listUp() {
                if ( currentIndex - (itemsPerPage() - 1) > 0 ) {
                    currentIndex -= (itemsPerPage() - 1)
                    machineListView.positionViewAtIndex(currentIndex, ListView.Contain);
                } else {
                    machineListView.positionViewAtBeginning();
                    currentIndex = 0;
                }
            }
            function listDown() {
                if ( currentIndex + (itemsPerPage() - 1) < machineListModelCount ) {
                    currentIndex += (itemsPerPage() - 1)
                    machineListView.positionViewAtIndex(currentIndex, ListView.Contain);
                } else {
                    machineListView.positionViewAtEnd();
                    currentIndex = machineListModelCount - 1;
                }
            }

            onStateChanged: {
                if (state == "hidden") {
                    if (DarkoneJS.resetFocused == "" && !darkone.toolbarHidden) {
                        if (toolbar.activeItem.parent == searchBox ||
                            toolbar.activeItem.parent.parent == searchBox)
                            showListButton.focus = true;
                    }
                } 
            }
            onFocusChanged: {
                (debug2 || debug3) && console.log("[focus] machineListView: '" + focus + "'" );
                (debug2 || debug3) && focus && DarkoneJS.inFocus();
                if ( focus )
                    DarkoneJS.focus("machineListView");
            }
            onActiveFocusChanged: {
                debug2 && console.log("[activeFocus] machineListView: '" + activeFocus + "'" );
                debug2 && activeFocus && DarkoneJS.inFocus();
            }
            onCurrentIndexChanged: {
                if ( darkone.initialised )
                    darkone.lastIndex = currentIndex;
            }

            CursorShapeArea {
                anchors.fill: parent
                cursorShape: Qt.ArrowCursor
            }
            // machineListView key events
            Keys.onPressed: {
                debug2 && console.log("[keys] machineListView: '" + DarkoneJS.keyEvent2String(event) + "'")

                // lights
                darkone.lights();

                switch ( event.key ) {
                    case Qt.Key_Backtab: {
                        overlay.focus = true;
                        event.accepted = true;
                        break;
                    }
                    case Qt.Key_Tab: {
                        if ( !darkone.toolbarHidden )
                            toolbarFocusScope.focus = true;
                        else
                            overlay.focus = true;
                        event.accepted = true;
                        break;
                    }
                    case Qt.Key_PageUp: {
                        listUp();
                        event.accepted = true;
                        break;
                    }
                    case Qt.Key_PageDown: {
                        listDown();
                        event.accepted = true;
                        break;
                    }
                    case Qt.Key_Home: {
                        positionViewAtBeginning();
                        currentIndex = 0;
                        event.accepted = true;
                        break;
                    }
                    case Qt.Key_End: {
                        positionViewAtEnd();
                        currentIndex = machineListModelCount - 1;
                        event.accepted = true;
                        break;
                    }
                    case Qt.Key_Enter:
                    case Qt.Key_Return: {
                        if ( !(event.modifiers & Qt.AltModifier) && !darkone.ignoreLaunch ) {
                            machineListView.positionViewAtIndex(darkone.lastIndex, ListView.Center);
                            DarkoneJS.launch();
                        }
                        break;
                    }
                    default: {
                        if ( event.modifiers & Qt.ControlModifier ) {
                            if ( event.modifiers & Qt.ShiftModifier ) {
                                switch ( event.key ) {
                                    case Qt.Key_Up: {
                                        positionViewAtBeginning();
                                        currentIndex = 0;
                                        event.accepted = true;
                                        break;
                                    }
                                    case Qt.Key_Down: {
                                        positionViewAtEnd();
                                        currentIndex = machineListModelCount - 1;
                                        event.accepted = true;
                                        break;
                                    }
                                }
                            } else {
                                switch ( event.key ) {
                                    case Qt.Key_Up: {
                                        listUp();
                                        event.accepted = true;
                                        break;
                                    }
                                    case Qt.Key_Down: {
                                        listDown();
                                        event.accepted = true;
                                        break;
                                    }
                                    case Qt.Key_Left: {
                                        overlay.focus = true;
                                        event.accepted = true;
                                        break;
                                    }
                                    case Qt.Key_Right: {
                                        if ( !darkone.toolbarHidden )
                                            toolbarFocusScope.focus = true;
                                        else
                                            overlay.focus = true;
                                        event.accepted = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // item
            delegate: Component {
                id: machineListItemDelegate
                Rectangle {
                    width: parent.width
                    height: machineListView.itemHeight
                    id: machineListItemBackground
                    smooth: true
                    border.color: "#333333"
                    border.width: 1
                    gradient: Gradient {
                                  GradientStop { position: 0.0; color: darkone.colour1 }
                                  GradientStop { position: 0.2; color: darkone.colour2 }
                                  GradientStop { position: 0.7; color: darkone.colour3 }
                                  GradientStop { position: 1.0; color: darkone.colour4 } }
                    opacity: 0.75

                    MouseArea {
                        id: machineListItemMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton
                        onContainsMouseChanged: {
                            if ( mapToItem(toolbar, mouseX, mouseY).y < 0 ) {
                                if ( containsMouse )
                                    DarkoneJS.itemEntered(machineListItemText, machineListItemBackground, mapToItem(toolbar, mouseX, mouseY));
                                else
                                    DarkoneJS.itemExited(machineListItemText, machineListItemBackground, mapToItem(toolbar, mouseX, mouseY));
                            }
                        }
                        onDoubleClicked: { if (!darkone.ignoreLaunch) {
                                              machineListView.currentIndex = index;
                                              machineListView.positionViewAtIndex(darkone.lastIndex, ListView.Center) ;
                                              DarkoneJS.launch(); }
                        }
                        onClicked: {
                            machineListView.currentIndex = index;
                            debug && console.log("[machineListView] setting index: '" + index + "'");
                            machineListView.focus = true;
                            machineListView.forceActiveFocus();
                        }
                    }

                    Text {
                        property bool fontResized: false
                        id: machineListItemText
                        anchors.fill: parent
                        anchors.margins: 10
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: model.modelData.description
                        color: machineListItemBackground.ListView.isCurrentItem ? darkone.textColour2: darkone.textColour1
                        font.bold: true
                        font.pixelSize: 13
                        elide: Text.ElideRight
                        smooth: true
                    }
                }
            }
        }


/***
 * preferences menu
 */
        FocusScope {
            id: preferencesFocusScope
            focus: false // darkoneFocusScope

            Rectangle {
                id: preferences
                z: 4
                y: darkone.height - toolbar.height - 5 - height
                x: preferencesButton.x - 10
                property int itemHeight: 12
                property int itemSpacing: 6
                property int itemTextSize: 9
                property string activeColour: darkone.textColour2
                height: (itemHeight + itemSpacing) * 25 + 10
                width: 175
                smooth: true
                border.color: parent.focus ? activeColour : "transparent";
                border.width: parent.focus ? 1 : 0;
                color: darkone.colour5
                opacity: 1.0
                state: "hidden"

                states: [
                    State {
                        name: "hidden"
                        PropertyChanges { target: preferences; visible: false; }
                    },
                    State {
                        name: "shown"
                        PropertyChanges { target: preferences; visible: true; }
                    }
                ]
                transitions: Transition {
                    from: "hidden"
                    to: "shown"
                    reversible: true
                    PropertyAnimation { property: "opacity"; duration: 100 }
                }

                onStateChanged: {
                    if ( state == "shown" ) {
                        darkone.ignoreLaunch = true;
                        darkone.preferencesLaunchLock = true;
                        darkone.toolbarShowMenuLock = true;
                        overlayScaleSliderItem.maximum = DarkoneJS.overlayScaleMax * 1.5;
                        overlayScaleSliderItem.minimum = DarkoneJS.overlayScaleMin;
                        overlayScaleSliderItem.value = darkone.overlayScale;
                        preferencesFocusScope.focus = true;
                    } else {
                        darkone.preferencesLaunchLock = false;
                        darkone.ignoreLaunch = false;
                        darkone.toolbarShowMenuLock = false;
                        DarkoneJS.focus(1);
                    }
                }
                onFocusChanged: {
                    if (darkone.initialised) {
                        debug2 && console.log("[focus] preferences: '" + focus + "'" );
                        debug2 && focus && DarkoneJS.inFocus();
                    }
                }
                onActiveFocusChanged: {
                    if (darkone.initialised) {
                        debug2 && console.log("[activeFocus] preferences: '" + activeFocus + "'" );
                        debug2 && activeFocus && DarkoneJS.inFocus();
                    }
                }

                CursorShapeArea {
                    anchors.fill: parent
                    cursorShape: Qt.ArrowCursor
                }
                MouseArea {
                    anchors.fill: parent;
                    hoverEnabled: true
                    onClicked: {
                        debug && console.log("[preferences onClick 1] focus: '" + focus + "'");
                        preferencesFocusScope.focus = true;
                        debug && console.log("[preferences onClick 2] focus: '" + focus + "'");
                   }
                }
                // preferences key events
                Keys.onPressed: {
                    debug2 && console.log("[keys] preferences: '" + DarkoneJS.keyEvent2String(event) + "'")

                    // lights
                    darkone.lights();

                    switch( event.key ) {
                        case Qt.Key_Tab: {
                            if ( event.modifiers & Qt.ShiftModifier ) {
                                backendParamValuesCycleItem.focus = true;
                                backendParamValuesCycleItem.forceActiveFocus()
                            } else {
                                sortByNameCheckBox.focus = true;
                                sortByNameCheckBox.forceActiveFocus();
                            }
                            event.accepted = true;
                            break;
                        }
                    }
                }

                Text {
                    id: headerText
                    property int index: 1
                    text: qsTr("Preferences")
                    height: parent.itemHeight
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing) - parent.itemSpacing
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    font.pixelSize: preferences.itemTextSize + 3
                    font.bold: true
                    color: darkone.textColour1
                    smooth: true
                }

                /***
                * behaviour
                */
                Text {
                    id: prefsText
                    property int index: 2
                    opacity: 1.0
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    text: qsTr("behaviour")
                    font.pixelSize: preferences.itemTextSize + 2
                    font.bold: true
                    color: darkone.textColour1
                    smooth: true
                }
                Rectangle {
                    id: prefsSeparator
                    height: 1
                    anchors.verticalCenter: prefsText.verticalCenter
                    anchors.left: prefsText.right
                    anchors.leftMargin: 7
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    color: darkone.textColour1
                    opacity: 0.5
                    smooth: true
                }
                CheckBox {
                    id: sortByNameCheckBox
                    focus: true // preferencesFocusScope
                    property int index: prefsText.index + 1
                    height: parent.itemHeight
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    checked: sortByName
                    text: qsTr("sort by name?")
                    textSize: preferences.itemTextSize
                    textColour: darkone.textColour1
                    activeColour: darkone.textColour2
                    smooth: true

                    onCheckedChanged: {
                        if (darkone.initialised) {
                            sortByName = checked;
                            var desc = machineListModel[machineListView.currentIndex].description
                            viewer.saveSettings();
                            viewer.loadMachineList();
                            machineListView.currentIndex = viewer.findIndex(desc, machineListView.currentIndex);
                            machineListView.positionViewAtIndex(darkone.lastIndex, ListView.Center);
                            debug && console.log("[sortByName] desc: '" + desc + "', " +
                                                 "result: '" + viewer.findIndex(desc, machineListView.currentIndex) + "'");
                        }
                    }

                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: backendParamValuesCycleItem
                    KeyNavigation.tab: autoHideToolbarCheckBox
                }

                CheckBox {
                    id: autoHideToolbarCheckBox
                    property int index: prefsText.index + 2
                    height: parent.itemHeight
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    checked: darkone.toolbarAutoHide
                    text: qsTr("auto-hide toolbar")
                    textSize: preferences.itemTextSize
                    textColour: darkone.textColour1
                    activeColour: darkone.textColour2
                    smooth: true

                    onCheckedChanged: { darkone.toolbarAutoHide = checked; }

                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: sortByNameCheckBox
                    KeyNavigation.tab: fpsCheckBox
                }
                CheckBox {
                    id: fpsCheckBox
                    property int index: prefsText.index + 3
                    height: parent.itemHeight
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    checked: darkone.fpsVisible
                    text: qsTr("FPS counter")
                    textSize: preferences.itemTextSize
                    textColour: darkone.textColour1
                    activeColour: darkone.textColour2
                    smooth: true

                    onCheckedChanged: {
                        darkone.fpsVisible = checked;
                        resetIgnoreLaunchTimer.restart();
                        darkone.toolbarShowFpsLock = checked ? darkone.toolbarAutoHide : false;
                    }

                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: autoHideToolbarCheckBox
                    KeyNavigation.tab: lightOutInputItem
                }
                InputItem {
                    id: lightOutInputItem
                    property int index: prefsText.index + 4
                    height: parent.itemHeight
                    inputWidth: 25
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    textPrefix: qsTr("lights out in")
                    textSuffix: qsTr("secs")
                    textSize: preferences.itemTextSize
                    textColour: darkone.textColour1
                    activeColour: darkone.textColour2
                    inputColour: "black"
                    text: darkone.lightTimeout

                    onAccepted: {
                        text = text.replace(/([^0-9.])/g, '');
                        var valid = text.match(/^(|\d+|\d+\.*\d+)$/g) &&
                                      parseFloat(text) >= 5 ? true : false
                        if (valid) {
                            inputColour = "black"
                            darkone.lightTimeout = parseFloat(text);
                            darkone.keepLightOn = darkone.lightTimeout == 0 ? true : false;
                            if (darkone.keepLightOn) {
                                lightOutTimer.stop();
                                lightOutScreenTimer.stop();
                            } else {
                                lightOutTimer.start();
                                lightOutScreenTimer.start();
                            }
                            focus = false;
                            overlayScaleSliderItem.focus = true;
                        } else
                            inputColour = "red"
                    }
                    onFocusChanged: { if (focus)
                                          text = text.replace(/([^0-9.])/g, ''); }

                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: fpsCheckBox
                    KeyNavigation.tab: overlayScaleSliderItem
                }
                SliderItem {
                    id: overlayScaleSliderItem
                    property int index: prefsText.index + 5
                    height: parent.itemHeight
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    textSize: preferences.itemTextSize
                    textColour: darkone.textColour1
                    activeColour: darkone.textColour2
                    fgColour1: darkone.colour3
                    fgColour2: darkone.colour4
                    bgColour1: "white"
                    bgColour2: "white"
                    textPrefix: qsTr("scale")
                    textSuffix: DarkoneJS.round(100 * darkone.overlayScale / DarkoneJS.overlayScaleMax, 0) + "%"
                    textSuffixWidth: 25
                    slidePercentage: 4

                    onValueChanged: darkone.overlayScale = DarkoneJS.round(value, 2);

                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: lightOutInputItem
                    KeyNavigation.tab: videoPlayerVolumeSlider
                }

                /***
                * video snaps
                */
                Text {
                    id: videoSnapsText
                    property int index: 8
                    opacity: 1.0
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    text: qsTr("video snaps")
                    font.pixelSize: preferences.itemTextSize + 2
                    font.bold: true
                    color: darkone.textColour1
                    smooth: true
                }
                Rectangle {
                    id: videoSnapsSeparator
                    height: 1
                    anchors.verticalCenter: videoSnapsText.verticalCenter
                    anchors.left: videoSnapsText.right
                    anchors.leftMargin: 7
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    color: darkone.textColour1
                    opacity: 0.5
                    smooth: true
                }
                SliderItem {
                    id: videoPlayerVolumeSlider
                    property int index: videoSnapsText.index + 1
                    height: parent.itemHeight
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    textSize: preferences.itemTextSize
                    textColour: darkone.textColour1
                    activeColour: darkone.textColour2
                    fgColour1: darkone.colour3
                    fgColour2: darkone.colour4
                    bgColour1: "white"
                    bgColour2: "white"
                    textPrefix: qsTr("volume")
                    textSuffix: DarkoneJS.round(100 * darkone.videoPlayerVolume, 0) + "%"
                    textSuffixWidth: 25
                    slidePercentage: 2
                    minimum: 0
                    maximum: 1
                    value: darkone.videoPlayerVolume;

                    onValueChanged: darkone.videoPlayerVolume = DarkoneJS.round(value, 2);

                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: overlayScaleSliderItem
                    KeyNavigation.tab: videoAutoPlaySlider
                }
                SliderItem {
                    id: videoAutoPlaySlider
                    property int index: videoSnapsText.index + 2
                    height: parent.itemHeight
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    textSize: preferences.itemTextSize
                    textColour: darkone.textColour1
                    activeColour: darkone.textColour2
                    fgColour1: darkone.colour3
                    fgColour2: darkone.colour4
                    bgColour1: "white"
                    bgColour2: "white"
                    textPrefix: qsTr("auto-play")
                    textSuffix: value === minimum ? qsTr("off") : DarkoneJS.round(value, 0) + qsTr("s")
                    textSuffixWidth: 25
                    slidePercentage: 2
                    minimum: -1
                    maximum: 60
                    value: darkone.videoAutoPlayTimeout / 1000;

                    onValueChanged: darkone.videoAutoPlayTimeout = value * 1000;

                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: videoPlayerVolumeSlider
                    KeyNavigation.tab: screenLightCheckBox
                }

                /***
                * effects
                */
                Text {
                    id: prefsEffectsText
                    property int index: 11
                    opacity: 1.0
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    text: qsTr("effects")
                    font.pixelSize: preferences.itemTextSize + 2
                    font.bold: true
                    color: darkone.textColour1
                    smooth: true
                }
                Rectangle {
                    id: prefsEffectsSeparator
                    height: 1
                    anchors.verticalCenter: prefsEffectsText.verticalCenter
                    anchors.left: prefsEffectsText.right
                    anchors.leftMargin: 7
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    color: darkone.textColour1
                    opacity: 0.5
                    smooth: true
                }
                CheckBox {
                    id: screenLightCheckBox
                    property int index: prefsEffectsText.index + 1
                    height: parent.itemHeight
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    checked: darkone.screenLight
                    text: qsTr("screen lighting")
                    textSize: preferences.itemTextSize
                    textColour: darkone.textColour1
                    activeColour: darkone.textColour2
                    smooth: true

                    onCheckedChanged: {
                        overlayScreenLight.visible = darkone.opacity == 0 ? false : true
                        darkone.screenLight = checked;
                    }

                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: videoAutoPlaySlider
                    KeyNavigation.tab: screenLightOpacitySliderItem
                }
                SliderItem {
                    id: screenLightOpacitySliderItem
                    property int index: prefsEffectsText.index + 2
                    height: parent.itemHeight
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    textSize: preferences.itemTextSize
                    textColour: darkone.textColour1
                    activeColour: darkone.textColour2
                    fgColour1: darkone.colour3
                    fgColour2: darkone.colour4
                    bgColour1: "white"
                    bgColour2: "white"
                    textPrefix: qsTr("opacity")
                    textSuffix: DarkoneJS.round(100 * darkone.screenLightOpacity, 0) + "%"
                    textSuffixWidth: 25
                    slidePercentage: 5
                    minimum: 0
                    maximum: 1
                    value: darkone.screenLightOpacity;

                    onValueChanged: darkone.screenLightOpacity = DarkoneJS.round(value, 2);

                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: screenLightCheckBox
                    KeyNavigation.tab: backLightCheckBox
                }
                CheckBox {
                    id: backLightCheckBox
                    property int index: prefsEffectsText.index + 3
                    height: parent.itemHeight
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    checked: darkone.backLight
                    text: qsTr("back lighting")
                    textSize: preferences.itemTextSize
                    textColour: darkone.textColour1
                    activeColour: darkone.textColour2
                    smooth: true

                    onCheckedChanged: {
                        overlayBackLight.visible = darkone.opacity == 0 ? false : true
                        darkone.backLight = checked;
                    }

                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: screenLightOpacitySliderItem
                    KeyNavigation.tab: backLightOpacitySliderItem
                }
                SliderItem {
                    id: backLightOpacitySliderItem
                    property int index: prefsEffectsText.index + 4
                    height: parent.itemHeight
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    textSize: preferences.itemTextSize
                    textColour: darkone.textColour1
                    activeColour: darkone.textColour2
                    fgColour1: darkone.colour3
                    fgColour2: darkone.colour4
                    bgColour1: "white"
                    bgColour2: "white"
                    textPrefix: qsTr("opacity")
                    textSuffix: DarkoneJS.round(100 * darkone.backLightOpacity, 0) + "%"
                    textSuffixWidth: 25
                    slidePercentage: 4
                    minimum: 0
                    maximum: 1
                    value: darkone.backLightOpacity;

                    onValueChanged: darkone.backLightOpacity = DarkoneJS.round(value, 2);

                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: backLightCheckBox
                    KeyNavigation.tab: launchFlashCheckBox
                }
                CheckBox {
                    id: launchFlashCheckBox
                    property int index: prefsEffectsText.index + 5
                    height: parent.itemHeight
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    checked: darkone.launchFlash
                    text: qsTr("launch flash?")
                    textSize: preferences.itemTextSize
                    textColour: darkone.textColour1
                    activeColour: darkone.textColour2
                    smooth: true

                    onCheckedChanged: { darkone.launchFlash = checked; }

                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: backLightOpacitySliderItem
                    KeyNavigation.tab: launchZoomCheckBox
                }
                CheckBox {
                    id: launchZoomCheckBox
                    property int index: prefsEffectsText.index + 6
                    height: parent.itemHeight
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    checked: darkone.launchZoom
                    text: qsTr("launch zoom?")
                    textSize: preferences.itemTextSize
                    textColour: darkone.textColour1
                    activeColour: darkone.textColour2
                    smooth: true

                    onCheckedChanged: { darkone.launchZoom = checked; }

                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: launchFlashCheckBox
                    KeyNavigation.tab: colourScheme1Button
                }

                /***
                * colour schemes
                */
                Text {
                    id: prefsColourSchemeText
                    property int index: 18
                    opacity: 1.0
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    text: qsTr("colour scheme")
                    font.pixelSize: preferences.itemTextSize + 2
                    font.bold: true
                    color: darkone.textColour1
                    smooth: true
                }
                Rectangle {
                    id: prefsColourSchemeSeparator
                    height: 1
                    anchors.verticalCenter: prefsColourSchemeText.verticalCenter
                    anchors.left: prefsColourSchemeText.right
                    anchors.leftMargin: 7
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    color: darkone.textColour1
                    opacity: 0.5
                    smooth: true
                }
                CheckableGroup { id: checkGroup }
                CheckItem {
                    id: colourScheme1Button
                    property int index: prefsColourSchemeText.index + 1
                    exclusiveGroup: checkGroup
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing) + 2
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    height: parent.itemHeight - 2
                    text: qsTr("dark")
                    textSize: preferences.itemTextSize
                    textColour: darkone.textColour1
                    activeColour: darkone.textColour2

                    onCheckedChanged: {
                        if ( checked ) { darkone.colourScheme = "dark"; }
                        debug && console.log("[colourScheme1Button] checked: '" + checked + "'");
                    }

                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: launchZoomCheckBox
                    KeyNavigation.tab: colourScheme2Button
                }
                CheckItem {
                    id: colourScheme2Button
                    property int index: prefsColourSchemeText.index + 2
                    exclusiveGroup: checkGroup
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing) - 2
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    height: parent.itemHeight - 2
                    text: qsTr("metal")
                    textSize: preferences.itemTextSize
                    textColour: darkone.textColour1
                    activeColour: darkone.textColour2

                    onCheckedChanged: {
                        if ( checked ) { darkone.colourScheme = "metal"; }
                        debug && console.log("[colourScheme2Button] checked: '" + checked + "'");
                    }

                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: colourScheme1Button
                    KeyNavigation.tab: backendParamNamesCycleItem
                }

                /***
                * backend
                */
                Text {
                    id: prefsBackendText
                    property int index: 21
                    opacity: 1.0
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    text: qsTr("backend")
                    font.pixelSize: preferences.itemTextSize + 2
                    font.bold: true
                    color: darkone.textColour1
                    smooth: true
                }
                Rectangle {
                    id: prefsBackendSeparator
                    height: 1
                    anchors.verticalCenter: prefsBackendText.verticalCenter
                    anchors.left: prefsBackendText.right
                    anchors.leftMargin: 7
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    color: darkone.textColour1
                    opacity: 0.5
                    smooth: true
                }
                CycleItem {
                    id: backendParamNamesCycleItem
                    property int index: prefsBackendText.index + 1
                    height: parent.itemHeight
                    width: parent.width - 25
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    textPrefix: "param:"
                    textSize: preferences.itemTextSize
                    textColour: darkone.textColour1
                    activeColour: darkone.textColour2
                    items: viewer.cliParamNames()
                    image: "../images/arrow.png"
                    imageWidth: 14
                    imageRotation: 0

                    onValueChanged: {
                        debug && console.log("[preferences params names 1] param " +
                                                 "values: '" + viewer.cliParamAllowedValues(backendParamNamesCycleItem.value) + "', " +
                                                 "values set: '" + backendParamValuesCycleItem.items + "', " +
                                                 "default: '" + viewer.cliParamValue(backendParamNamesCycleItem.value) + "', " +
                                                 "default set: '" + backendParamValuesCycleItem.selectedItem + "'");

                        backendParamValuesCycleItem.items = viewer.cliParamAllowedValues(value)
                        backendParamValuesCycleItem.selectedItem = viewer.cliParamValue(backendParamNamesCycleItem.value)
                        backendParamValuesCycleItem.value = viewer.cliParamValue(backendParamNamesCycleItem.value)

                        debug && console.log("[preferences param names 2] param " +
                                                 "values: '" + viewer.cliParamAllowedValues(backendParamNamesCycleItem.value) + "', " +
                                                 "values set: '" + backendParamValuesCycleItem.items + "', " +
                                                 "default: '" + viewer.cliParamValue(backendParamNamesCycleItem.value) + "', " +
                                                 "default set: '" + backendParamValuesCycleItem.selectedItem + "'");

                    }

                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: colourScheme2Button
                    KeyNavigation.tab: backendParamValuesCycleItem
                }
                Text {
                    id: backendParamDescText
                    property int index: prefsBackendText.index + 2
                    opacity: 0.8
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    text: "desc:   "
                    font.pixelSize: preferences.itemTextSize
                    font.bold: false
                    color: darkone.textColour1
                    smooth: true

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: { parent.opacity += 0.2; }
                        onExited: { parent.opacity -= 0.2; }
                    }
                }
                Text {
                    id: backendParamDescText2
                    opacity: 1.0
                    anchors.verticalCenter: backendParamDescText.verticalCenter
                    anchors.left: backendParamDescText.right
                    anchors.leftMargin: 5
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    text: viewer.cliParamDescription(backendParamNamesCycleItem.value)
                    font.pixelSize: preferences.itemTextSize + 1
                    color: darkone.textColour1
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    smooth: true
                }
                CycleItem {
                    id: backendParamValuesCycleItem
                    property int index: prefsBackendText.index + 3
                    height: parent.itemHeight
                    width: parent.width - 25
                    anchors.top: parent.top
                    anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    textSize: preferences.itemTextSize
                    textColour: darkone.textColour1
                    activeColour: darkone.textColour2
                    textPrefix: "value: "
                    image: "../images/arrow.png"
                    imageWidth: 14
                    imageRotation: 0

                    onSelect: {
                        debug && console.log("[preferences] param values: '" +
                                                  viewer.cliParamAllowedValues(backendParamNamesCycleItem.value) + "', " +
                                                 "default: '" + viewer.cliParamValue(backendParamNamesCycleItem.value) + "'");
                        if (value != "" && selectedItem != value) {
                            debug && console.log("[preferences] setting param: '" + backendParamNamesCycleItem.value + ", " +
                                                                       "value: '" + selectedItem + "' -> '" + value + "'");
                            selectedItem = value;
                            viewer.setCliParamValue(backendParamNamesCycleItem.value, value);
                        }
                    }

                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: backendParamNamesCycleItem
                    KeyNavigation.tab: sortByNameCheckBox
                }
            }
        } // focusScope


/***
 * toolbar
 */
        Rectangle {
            id: toolbarBorder
            z: toolbar.z + 15
            anchors.bottom: parent.bottom
            anchors.bottomMargin: toolbar.height + 1
            anchors.left: parent.left
            anchors.leftMargin: 1
            height: 2
            width: darkone.width - 2
            color: darkone.textColour2
            visible: toolbarFocusScope.focus && darkone.activeBorders
        }
        Rectangle {
            id: toolbarItemBorderBottom
            z: parent.z + 15
            anchors.left: parent.left
            anchors.leftMargin: toolbar.activeItem ? (toolbar.activeItem.parent == toolbar ? toolbar.activeItem.x : toolbar.activeItem.mapToItem(null, 0, 0).x) : 0
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 2 + darkone.activeBorderSize
            width: toolbar.activeItem ? toolbar.activeItem.width : 0
            height: darkone.activeBorderSize - 1
            visible: toolbarFocusScope.focus && darkone.activeBorders && toolbar.activeItemBorders
            color: darkone.textColour2
        }
        Rectangle {
            id: toolbarItemBorderTop
            z: parent.z + 15
            anchors.left: parent.left
            anchors.leftMargin: toolbarItemBorderBottom.anchors.leftMargin
            anchors.bottom: parent.bottom
            anchors.bottomMargin: toolbar.height - 1 - (2 + darkone.activeBorderSize)
            width: toolbarItemBorderBottom.width
            height: toolbarItemBorderBottom.height
            opacity: toolbarItemBorderBottom.opacity
            visible: toolbarItemBorderBottom.visible
            color: darkone.textColour2
        }

        FocusScope {
            id: toolbarFocusScope
            anchors.fill: parent
            focus: false // darkoneFocusScope

            onFocusChanged: {
                (debug2 || debug3) && console.log("[focus] toolbarFocusScope: '" + focus + "'" );
                (debug2 || debug3) && focus && DarkoneJS.inFocus();
                if ( focus ) {
                    DarkoneJS.focus("toolbarFocusScope");
                    hideToolbarTimer.stop();
                } else
                    hideToolbarTimer.restart();
            }
            onActiveFocusChanged: {
                debug2 && console.log("[activeFocus] toolbarFocusScope: '" + activeFocus + "'" );
                debug2 && activeFocus && DarkoneJS.inFocus();
            }

            Keys.onPressed: {
                debug2 && console.log("[keys] toolbarFocusScope: '" + DarkoneJS.keyEvent2String(event) + "'")
            }

            Rectangle {
                id: toolbar
                z: 4
                property bool cycling: false
                property bool activeItemBorders: true
                property Item activeItem
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                width: DarkoneJS.baseWidth * DarkoneJS.scaleFactorX()
                height: 36
                opacity: 0.75
                smooth: true
                state: "shown"
                gradient: Gradient {
                    GradientStop { position: 0.0; color: darkone.colour1 }
                    GradientStop { position: 0.2; color: darkone.colour2 }
                    GradientStop { position: 0.7; color: darkone.colour3 }
                    GradientStop { position: 1.0; color: darkone.colour4 }
                }

                transitions: [
                    Transition {
                        from: "hidden"
                        to: "shown"
                        SequentialAnimation {
                            ParallelAnimation {
                                PropertyAnimation { target: toolbar; property: "anchors.bottomMargin"; from: -(toolbar.height - 2); to: 0; duration: 500; easing.type: Easing.OutCubic }
                                PropertyAnimation { target: machineListView; property: "anchors.bottomMargin"; from: 2 + machineListView.itemHeight; to: toolbar.height + machineListView.itemHeight; duration: 500; easing.type: Easing.OutCubic }
                            }
                            PropertyAnimation { target: toolbarBorder; property: "opacity"; from: 0; to: 1.0; duration: 0; }
                            PropertyAnimation { target: toolbarItemBorderBottom; property: "opacity"; from: 0; to: 1.0; duration: 0; }
                        }
                    },
                    Transition {
                        from: "shown"
                        to: "hidden"
                        SequentialAnimation {
                            PropertyAnimation { target: toolbarItemBorderBottom; property: "opacity"; from: 1.0; to: 0; duration: 0; }
                            PropertyAnimation { target: toolbarBorder; property: "opacity"; from: 1.0; to: 0; duration: 0; }
                            ParallelAnimation {
                                PropertyAnimation { target: toolbar; property: "anchors.bottomMargin"; from: 0; to: -(toolbar.height - 2); duration: 500; easing.type: Easing.OutCubic }
                                PropertyAnimation { target: machineListView; property: "anchors.bottomMargin"; from: toolbar.height + machineListView.itemHeight; to: 2 + machineListView.itemHeight; duration: 500; easing.type: Easing.OutCubic }
                            }
                        }
                    }
                ]

                onFocusChanged: {
                    if (darkone.initialised) {
                        debug2 && console.log("[focus] toolbar: '" + focus + "'" );
                        debug2 && focus && DarkoneJS.inFocus();
                    }
                    if (!toolbar.cycling)
                        toolbarFocusScope.focus = true;
                }
                onActiveFocusChanged: {
                    if (darkone.initialised) {
                        debug2 && console.log("[activeFocus] toolbar: '" + activeFocus + "'" );
                        debug2 && activeFocus && DarkoneJS.inFocus();
                    }
                    if (activeFocus) 
                        toolbar.activeItemBorders = false;
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: {
                        darkone.toolbarHidden && !DarkoneJS.inGame && DarkoneJS.toolbarToggle();
                        hideToolbarTimer.stop();
                    }
                    onExited: { hideToolbarTimer.restart(); }
                    onPositionChanged: {
                        // lights
                        darkone.lights();
                    }
                    onClicked: { toolbarFocusScope.focus = true; }
                }
                // toolbar key events
                Keys.onPressed: {
                    debug2 && console.log("[keys] toolbar: '" + DarkoneJS.keyEvent2String(event) + "'")

                    // lights
                    darkone.lights();
                    // hide timer  
                    hideToolbarTimer.stop();
                    // searchTextInput hack
                    searchTextInput.Keys.forwardTo = [];

                    if ( darkone.toolbarHidden )
                        debug2 && console.log("[toolbar] error: key press in hidden state")
                    else {
                        if ( event.modifiers & Qt.ControlModifier ) {
                            switch ( event.key ) {
                                case Qt.Key_Left:
                                case Qt.Key_Up: {
                                    if ( !darkone.listHidden )
                                        machineListView.forceActiveFocus();
                                    else
                                        overlay.forceActiveFocus();
                                    event.accepted = true;
                                    break;
                                }
                                case Qt.Key_Right:
                                case Qt.Key_Down: {
                                    overlay.forceActiveFocus();
                                    event.accepted = true;
                                    break;
                                }
                            }
                        } else {
                            switch ( event.key ) {
                                case Qt.Key_Backtab: {
                                    if ( toolbar.cycling ) {
                                        toolbar.cycling = false;
                                        if ( !darkone.listHidden )
                                            machineListView.forceActiveFocus();
                                        else
                                            overlay.forceActiveFocus();
                                    } else {
                                        exitButton.focus = true;
                                    }
                                    event.accepted = true;
                                    break;
                                }
                                case Qt.Key_Tab: {
                                    if ( toolbar.cycling ) {
                                        toolbar.cycling = false;
                                        overlay.forceActiveFocus();
                                    } else {
                                        if ( !darkone.listHidden )
                                            searchButton.focus = true;
                                        else
                                            showListButton.focus = true;
                                    }
                                    event.accepted = true;
                                    break;
                                }
                                default: {
                                    if ( !darkone.listHidden ) {
                                        switch ( event.key ) {
                                            case Qt.Key_Left:
                                            case Qt.Key_Up:
                                            case Qt.Key_Right:
                                            case Qt.Key_Down: {
                                                event.accepted = true;
                                                break;
                                            }
                                        }
                                    }
                                }
                                break;
                            } // switch
                        }
                    }
                }

                Item {
                    id: searchBox
                    width: DarkoneJS.listWidth()
                    height: 24
                    anchors.left: toolbar.left
                    anchors.leftMargin: darkone.itemLeftMargin
                    anchors.verticalCenter: parent.verticalCenter
                    opacity: 1.0

                    Image {
                        id: searchButton
                        source: "images/find.png"
                        height: 16
                        anchors.right: searchTextInputBox.left
                        anchors.rightMargin: 5
                        anchors.verticalCenter: parent.verticalCenter
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                        opacity: 0.75

                        onFocusChanged: {
                            debug2 && console.log("[focus] searchButton: '" + focus + "'" );
                        }
                        onActiveFocusChanged: {
                            debug2 && console.log("[activeFocus] searchButton: '" + activeFocus + "'" );
                            if (activeFocus) {
                                toolbar.activeItem = searchButton;
                                toolbar.activeItemBorders = true;
                            } 
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: { darkone.toolbarAutoHide && darkone.toolbarHidden && !DarkoneJS.inGame && DarkoneJS.toolbarToggle(); hideToolbarTimer.stop(); parent.opacity = 1.0 }
                            onExited: { hideToolbarTimer.restart(); parent.opacity = 0.75 }
                            onClicked: {
                                parent.opacity = 1.0;
                                machineListView.currentIndex = viewer.findIndex(searchTextInput.text, machineListView.currentIndex)
                                machineListView.positionViewAtIndex(machineListView.currentIndex, ListView.Center);
                                focus = true;
                            }
                        }
                        Keys.onUpPressed: {
                            toolbar.cycling = true;
                            toolbar.focus = true;
                        }
                        Keys.onBacktabPressed: {
                            toolbar.cycling = true;
                            toolbar.focus = true;
                        }
                        KeyNavigation.down: KeyNavigation.tab
                        KeyNavigation.tab: searchTextInput
                        Keys.onPressed: {
                            darkone.lights();
                            switch ( event.key ) {
                                case Qt.Key_Space:
                                case Qt.Key_Enter:
                                case Qt.Key_Return: {
                                    machineListView.currentIndex = viewer.findIndex(searchTextInput.text, machineListView.currentIndex)
                                    machineListView.positionViewAtIndex(machineListView.currentIndex, ListView.Center);
                                    focus = true;
                                    event.accepted = true;
                                    break;
                                }
                            }
                        }
                    }
                    Rectangle {
                        id: searchTextInputBox
                        height: 18
                        width: DarkoneJS.listWidth() - searchButton.width - 5 - clearButton.width - 5
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        radius: height - 2
                        smooth: true

                        onFocusChanged: { debug2 && console.log("[focus] searchTextInputBox: '" + focus + "'" ); }
                        onActiveFocusChanged: {
                            debug2 && console.log("[activeFocus] searchTextInputBox: '" + activeFocus + "'" );
                        }

                        TextInput {
                            id: searchTextInput
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.topMargin: 2
                            anchors.bottomMargin: 2
                            anchors.leftMargin: 8
                            anchors.right: parent.right
                            anchors.rightMargin: 8
                            font.pointSize: parent.height - 6
                            smooth: true
                            autoScroll: true
                            clip: true
                            selectByMouse: true
                            focus: true // toolbarFocusScope

                            cursorDelegate: Rectangle {
                                id: searchTextCursorDelegate
                                color: "black"
                                width: 1
                                height: 0.5
                                anchors.verticalCenter: parent.verticalCenter
                                visible: parent.activeFocus
                            }

                            onAccepted: { machineListView.currentIndex = viewer.findIndex(searchTextInput.text, machineListView.currentIndex)
                                          machineListView.positionViewAtIndex(machineListView.currentIndex, ListView.Center);
                            }
                            onFocusChanged: {
                                if ( darkone.initialised ) {
                                    debug2 && console.log("[focus] searchTextInput: '" + focus + "'" );
                                    debug2 && focus && DarkoneJS.inFocus();
                                }
                            }
                            onActiveFocusChanged: {
                                if ( darkone.initialised ) {
                                    debug2 && console.log("[activeFocus] searchTextInput: '" + activeFocus + "'" );
                                    debug2 && activeFocus && DarkoneJS.inFocus();
                                }
                                if (activeFocus) {
                                    if (darkone.listHidden)
                                        showListButton.focus = true;
                                    else {
                                        toolbar.activeItem = searchTextInputBox;
                                        toolbar.activeItemBorders = true;
                                    }
                                }
                            }

                            KeyNavigation.up: KeyNavigation.backtab
                            KeyNavigation.down: KeyNavigation.tab
                            KeyNavigation.backtab: searchButton
                            KeyNavigation.tab: clearButton
                            Keys.priority: Keys.BeforeItem
                            Keys.onPressed: {
                                debug2 && console.log("[keys] searchTextInput: '" + DarkoneJS.keyEvent2String(event) + "'");
                                darkone.lights();
                                Keys.forwardTo = []; // reset forwarding
                                if ( darkone.listHidden ) {
                                    debug2 && console.log("[searchInputText] error: key press in hidden state");
                                    showListButton.focus = true;
                                    event.accepted = true;
                                } else {
                                    if ( event.modifiers & Qt.ControlModifier ) {
                                        Keys.forwardTo = [toolbar]; // forwarding to intercept global key sequences
                                    }
                                }
                            }
                        }
                    }
                    Image {
                        id: clearButton
                        source: "images/clear.png"
                        height: 18
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: searchTextInputBox.right
                        anchors.leftMargin: 5
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                        opacity: 0.75

                        onFocusChanged: {
                            debug2 && console.log("[focus] clearButton: '" + focus + "'" );
                        }
                        onActiveFocusChanged: {
                            debug2 && console.log("[activeFocus] clearButton: '" + activeFocus + "'" );
                            if (activeFocus) {
                                toolbar.activeItem = clearButton;
                                toolbar.activeItemBorders = true;
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: parent.opacity = 1.0
                            onExited: parent.opacity = 0.75
                            onClicked: {
                                parent.opacity = 1.0;
                                searchTextInput.text = "";
                                searchTextInput.focus = true;
                            }
                        }
                        KeyNavigation.up: KeyNavigation.backtab
                        KeyNavigation.down: KeyNavigation.tab
                        KeyNavigation.backtab: searchTextInput
                        KeyNavigation.tab: showListButton
                        Keys.onPressed: {
                            darkone.lights();
                            switch ( event.key ) {
                                case Qt.Key_Space:
                                case Qt.Key_Enter:
                                case Qt.Key_Return: {
                                    searchTextInput.text = "";
                                    searchTextInput.focus = true;
                                    event.accepted = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                Image {
                    id: showListButton
                    source: "images/list_toggle.png"
                    height: 18
                    width: height
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: (parent.height - height) / 2
                    anchors.left: toolbar.left
                    // important: ensure behaviour is based on actual state and not 'secondary/wrapper' boolean darkone.listHidden
                    anchors.leftMargin: darkone.itemLeftMargin + (machineListView.state == "hidden" ? 0 : DarkoneJS.listWidth() + 15);
                    fillMode: Image.PreserveAspectFit
                    opacity: 0.75
                    rotation: darkone.listHidden ? 90 : 270
                    smooth: true
                    z: 5

                    onFocusChanged: {
                        debug2 && console.log("[focus] showListButton: '" + focus + "'" );
                    }
                    onActiveFocusChanged: {
                        debug2 && console.log("[activeFocus] showListButton: '" + activeFocus + "'" );
                        if (activeFocus) {
                            toolbar.activeItem = showListButton;
                            toolbar.activeItemBorders = true;
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: parent.opacity = 1.0
                        onExited: parent.opacity = 0.75
                        onClicked: {
                            DarkoneJS.listToggle();
                            toolbarFocusScope.focus = true;
                        }
                    }
                    Keys.onUpPressed: {
                        if ( ! darkone.listHidden )
                            clearButton.focus = true;
                        else {
                            toolbar.cycling = true;
                            toolbar.focus = true;
                        }
                    }
                    Keys.onBacktabPressed: {
                        if ( ! darkone.listHidden )
                            clearButton.focus = true;
                        else {
                            toolbar.cycling = true;
                            toolbar.focus = true;
                        }
                    }
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.tab: preferencesButton
                    Keys.onPressed: {
                        darkone.lights();
                        switch ( event.key ) {
                            case Qt.Key_Space:
                            case Qt.Key_Enter:
                            case Qt.Key_Return: {
                                DarkoneJS.listToggle();
                                toolbarFocusScope.focus = true;
                                event.accepted = true;
                                break;
                            }
                        }
                    }
                }
                Image {
                    id: preferencesButton
                    height: 16
                    anchors.left: toolbar.left
                    anchors.leftMargin: showListButton.anchors.leftMargin + showListButton.width + 10
                    anchors.verticalCenter: parent.verticalCenter
                    source: "images/preferences.png"
                    smooth: true
                    opacity: 0.75
                    fillMode: Image.PreserveAspectFit

                    onFocusChanged: {
                        debug2 && console.log("[focus] preferencesButton: '" + focus + "'" );
                    }
                    onActiveFocusChanged: {
                        debug2 && console.log("[activeFocus] preferencesButton: '" + activeFocus + "'" );
                        if (activeFocus) {
                            toolbar.activeItem = preferencesButton;
                            toolbar.activeItemBorders = true;
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: parent.opacity = 1.0
                        onExited: parent.opacity = 0.6
                        onClicked: {
                            debug && console.log("[preferencesButton clicked] state: '" + preferences.state + "'")
                            debug && DarkoneJS.info("[preferencesButton clicked]", preferences)
                            parent.opacity = 1.0;
                            if (preferences.state == "shown")
                                preferences.state = "hidden";
                            else
                                preferences.state = "shown";
                        }
                    }
                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: showListButton
                    KeyNavigation.tab: fullScreenButton
                    Keys.onPressed: {
                        darkone.lights();
                        switch ( event.key ) {
                            case Qt.Key_Space:
                            case Qt.Key_Enter:
                            case Qt.Key_Return: {
                                if (preferences.state == "shown")
                                    preferences.state = "hidden";
                                else
                                    preferences.state = "shown";
                                event.accepted = true;
                                break;
                            }
                        }
                    }
                }
                Image {
                    id: fullScreenButton
                    height: 16
                    anchors.left: toolbar.left
                    anchors.leftMargin: preferencesButton.anchors.leftMargin + preferencesButton.width + 10
                    anchors.verticalCenter: parent.verticalCenter
                    source: "images/fullscreen.png"
                    state: darkone.fullScreen ? "fullscreen" : "windowed"
                    smooth: true
                    opacity: 0.75
                    fillMode: Image.PreserveAspectFit

                    states: [
                        State {
                            name: "fullscreen"
                            PropertyChanges { target: fullScreenButton; source: "images/windowed.png" }
                        },
                        State {
                            name: "windowed"
                            PropertyChanges { target: fullScreenButton; source: "images/fullscreen.png" }
                        }
                    ]

                    onFocusChanged: {
                        debug2 && console.log("[focus] fullScreenButton: '" + focus + "'" );
                    }
                    onActiveFocusChanged: {
                        debug2 && console.log("[activeFocus] fullScreenButton: '" + activeFocus + "'" );
                        if (activeFocus) {
                            toolbar.activeItem = fullScreenButton;
                            toolbar.activeItemBorders = true;
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: parent.opacity = 1.0
                        onExited: parent.opacity = 0.6
                        onClicked: {
                            DarkoneJS.fullScreenToggle();
                            parent.opacity = 1.0;
                        }
                    }
                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: preferencesButton
                    KeyNavigation.tab: launchButton
                    Keys.onPressed: {
                        darkone.lights();
                        switch ( event.key ) {
                            case Qt.Key_Space:
                            case Qt.Key_Enter:
                            case Qt.Key_Return: {
                                DarkoneJS.fullScreenToggle();
                                event.accepted = true;
                                break;
                            }
                        }
                    }
                }
                Text {
                    id: fpsText
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: fullScreenButton.right
                    anchors.leftMargin: 15
                    color: darkone.textColour1
                    text: qsTr("FPS") + ": " + darkone.fps.toString()
                    visible: darkone.fpsVisible
                }
                Rectangle {
                    id: launchBox
                    height: 20
                    width: 40
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.rightMargin: overlay.anchors.rightMargin + overlay.width - overlayButtonBlock.x - (overlayButtonBlock.width / 2) - (width / 2);
                    anchors.right: parent.right
                    opacity: 0.5
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "transparent" }
                        GradientStop { position: 0.25; color: DarkoneJS.gameStatusColour() }
                        GradientStop { position: 0.75; color: DarkoneJS.gameStatusColour() }
                        GradientStop { position: 1.0; color: "transparent" }
                    }

                    onFocusChanged: { debug2 && console.log("[focus] launchBox: '" + focus + "'" ); }
                    onActiveFocusChanged: { debug2 && console.log("[activeFocus] launchBox: '" + activeFocus + "'" ); }

                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.down: KeyNavigation.tab
                    KeyNavigation.backtab: fullScreenButton
                    KeyNavigation.tab: exitButton
                    Keys.onPressed: {
                        darkone.lights();
                        switch ( event.key ) {
                            case Qt.Key_Space:
                            case Qt.Key_Enter:
                            case Qt.Key_Return: {
                                if (!darkone.ignoreLaunch) {
                                    machineListView.positionViewAtIndex(darkone.lastIndex, ListView.Center);
                                    DarkoneJS.launch();
                                }
                                event.accepted = true;
                                break;
                            }
                        }
                    }

                    Image {
                        id: launchButton
                        source: "images/launch.png"
                        height: 16
                        width: 40
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter

                        onFocusChanged: {
                            debug2 && console.log("[focus] launchButton: '" + activeFocus + "'" );
                        }
                        onActiveFocusChanged: {
                            debug2 && console.log("[activeFocus] launchButton: '" + activeFocus + "'" );
                            if (activeFocus) {
                                toolbar.activeItem = launchBox;
                                toolbar.activeItemBorders = true;
                            }
                        }

                        CursorShapeArea {
                            anchors.fill: parent
                            cursorShape: Qt.CrossCursor
                        }
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: { parent.opacity = 1.0
                                         parent.parent.opacity = 1.0
                                         overlayStateBlock.opacity = (lightOnAnimation.running || darkone.lightOut) ? 0 : 1.0;
                            }
                            onExited: { parent.opacity = 0.75
                                        parent.parent.opacity = 0.5
                                        overlayStateBlock.opacity = (lightOnAnimation.running || darkone.lightOut) ? 0 : 0.5;
                            }
                            onClicked: { if (!darkone.ignoreLaunch) {
                                             machineListView.positionViewAtIndex(darkone.lastIndex, ListView.Center);
                                             DarkoneJS.launch(); }
                            }
                        }
                    }
                }
                Image {
                    id: exitButton
                    height: 18
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: 15
                    source: "images/exit.png"
                    smooth: true
                    opacity: 0.25
                    fillMode: Image.PreserveAspectFit

                    onFocusChanged: {
                        debug2 && console.log("[focus] exitButton: '" + focus + "'" );
                    }
                    onActiveFocusChanged: {
                        debug2 && console.log("[activeFocus] exitButton: '" + activeFocus + "'" );
                        if (activeFocus) {
                            toolbar.activeItem = exitButton;
                            toolbar.activeItemBorders = true;
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: { parent.opacity = 1.0; }
                        onExited: { parent.opacity = 0.6; }
                        onClicked: {
                            parent.opacity = 1.0;
                            darkone.ignoreLaunch = true;
                            Qt.quit();
                        }
                    }
                    Keys.onDownPressed: {
                        toolbar.cycling = true;
                        toolbar.focus = true;
                    }
                    Keys.onTabPressed: {
                        toolbar.cycling = true;
                        toolbar.focus = true;
                    }
                    KeyNavigation.up: KeyNavigation.backtab
                    KeyNavigation.backtab: launchButton
                    Keys.onPressed: {
                        darkone.lights();
                        switch ( event.key ) {
                            case Qt.Key_Space:
                            case Qt.Key_Enter:
                            case Qt.Key_Return: {
                                darkone.ignoreLaunch = true;
                                Qt.quit();
                                break;
                            }
                        }
                    }
                }
            } // toolbar
        } // toolbarFocusScope
    } // darkone
} // darkoneFocusScope
