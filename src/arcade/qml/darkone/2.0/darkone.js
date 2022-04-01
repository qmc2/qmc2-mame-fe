var baseWidth = 800;
var baseHeight = 600;
var baseListWidth = 250;
var lastItemText;
var lastitemBackground;
var resetScale;
var resetListHidden;
var resetToolbarHidden;
var resetFocused;
var flashCounter = 0;
var inGame = false;
var overlayScaleMin = 0;
var overlayScaleMax = 0;
var dataTypes = { "title": { key: "title", name: "title image", type: "image", path: "ttl" },
    "preview": { key: "preview", name: "preview image", type: "image", path: "prv" },
    "flyer": { key: "flyer", name: "flyer image", type: "image", path: "fly" },
    "cabinet": { key: "cabinet", name: "cabinet image", type: "image", path: "cab" },
    "controller": { key: "controller", name: "controller image", type: "image", path: "ctl" },
    "marquee": { key: "marquee", name: "marquee image", type: "image", path: "mrq" },
    "pcb": { key: "pcb", name: "pcb image", type: "image", path: "pcb" },
    "sysinfo": { key: "sysinfo", name: "machine info", type: "text", text: "[machine info]" },
    "emuinfo": { key: "emuinfo", name: "emulator info", type: "text", text: "[emulator info]" } }
var focused = [];
var borders = false;

function init() {
    if (!darkone.initialised && darkone.lastIndex == -1) {
        overlayScaleMax = baseHeight / overlayScreen.height;
        overlayScaleMin = overlayScaleMax * 0.10;
        debug && console.log("[init] overlayScreen.height: '" + overlayScreen.height + "', " +
                             "overlayScaleMax: '" + overlayScaleMax + "'");
        viewer.loadSettings();
        colourScheme(darkone.colourScheme);
        viewer.loadMachineList();
        debug && console.log("[init] lastIndex: '" + darkone.lastIndex + "', " +
                             "machineListModelCount: '" + machineListModelCount + "'");
        darkone.lastIndex = (darkone.lastIndex < machineListModelCount && darkone.lastIndex > -1) ? darkone.lastIndex : 0;
        machineListView.currentIndex = darkone.lastIndex
        machineListView.positionViewAtIndex(machineListView.currentIndex, ListView.Center);
        debug && console.log("[init] lastIndex: '" + darkone.lastIndex + "', " +
                             "machineListModelCount: '" + machineListModelCount + "'");
        darkone.dataTypeCurrent = darkone.dataTypePrimary;
        debug && console.log("[init 1] resetScale: '" + resetScale + "', " +
                             "overlayScale: '" + darkone.overlayScale + "'");
        resetScale = darkone.overlayScale;
        darkone.overlayScale = Math.min(darkone.overlayScale, overlayScaleMax) / 2;
        debug && console.log("[init 2] resetScale: '" + resetScale + "', " +
                             "overlayScale: '" + darkone.overlayScale + "'");

        listToggle(1 - (darkone.listHidden * 2));
        machineListViewBorder.opacity = darkone.listHidden ? 0 : 1;
        if (darkone.fpsVisible)
            darkone.toolbarShowFpsLock = true;
        if (!darkone.keepLightOn) {
            lightOutTimer.start();
            lightOutScreenTimer.start();
        }
        darkone.keepLightOn = (darkone.lightTimeout == 0) ? true : false;
        toolbarToggle(darkone.toolbarAutoHide ? -1 : 1);
        toolbarBorder.opacity = darkone.toolbarHidden ? 0 : 1;
        fadeIn.start();
    } else if(!fadeIn.running) {
        darkone.initialised = true;
        initTimer.stop();
        //power
        darkone.lightOut = false;
        darkone.lightOutScreen = false;
        //immediate user interaction may have caused a light-on event which means the zoom may
        //have already been reset. check before we call zoom with a negative argument!
        if (resetScale != -1) {
            zoom(resetScale / darkone.overlayScale);
            resetScale = -1;
        }
        focused = ["overlay"];
        resetFocused = ""
        focus(1);
    }
}

function info(context, object) {
    console.log("[" + context + "] " +
                //                "opacity: '" + object.opacity + "'" +
                //                "x: '" + object.x + "', " +
                //                "width: '" + round(object.width, 2) + "', " +
                //                "scaleFactorX: '" + round(scaleFactorX(), 2) + "', " +
                //                "leftMargin: '" + round(object.anchors.leftMargin, 2) + "', " +
                //                "rightMargin: '" + round(object.anchors.rightMargin, 2) + "', " +
                "y: '" + object.y + "', " +
                "height: '" + round(object.height, 2) + "', " +
                "scaleFactorY: '" + round(scaleFactorY(), 2) + "', " +
                "topMargin: '" + round(object.anchors.topMargin, 2) + "', " +
                "bottomMargin: '" + round(object.anchors.bottomMargin, 2) + "', " +
                "")
}

function scaleFactorX() {
    return darkone.width / baseWidth;
}

function scaleFactorY() {
    return darkone.height / baseHeight;
}

function listWidth() {
    return Math.floor(Math.max(baseListWidth, baseListWidth * scaleFactorX()));
}

function overlayWidth() {
    return baseWidth * scaleFactorX() - (darkone.listHidden ? 0 : listWidth());
}

function itemEntered(itemText, itemBackground, itemCurrent) {
    debug && console.log("[itemEntered] " +
                         "listWidth: '" + listWidth() + "', " +
                         "overlayWidth: '" + overlayWidth() + "'");
    if ( !itemText.fontResized ) {
        if ( lastItemText != undefined )
            itemExited(lastItemText, lastitemBackground);
        lastItemText = itemText;
        itemText.fontResized = true;
        itemText.font.pixelSize += 1;
    }
}

function itemExited(itemText, itemBackground, itemCurrent) {
    if ( itemText.fontResized ) {
        itemText.fontResized = false;
        itemText.font.pixelSize -= 1;
    }
}

function toolbarToggle(force) {
    debug && console.log("[toolbarToggle 1] " +
                         "toolbarHidden: '" + darkone.toolbarHidden + "', " +
                         "toolbarAutoHide: '" + darkone.toolbarAutoHide + "', " +
                         "force: '" + force + "'");
    if (force > 0 || (darkone.toolbarHidden && !force)) {
        darkone.toolbarHidden = false;
        toolbarFocusScope.focus = true;
    } else if ((force < 0 || (!darkone.toolbarHidden && !force)) &&
               !darkone.toolbarShowMenuLock && !darkone.toolbarShowFpsLock) {
        darkone.toolbarHidden = true;
        toolbarFocusScope.focus && focus(1);
    }
    debug && console.log("[toolbarToggle 2] " +
                         "toolbarHidden: '" + darkone.toolbarHidden + "', " +
                         "toolbarAutoHide: '" + darkone.toolbarAutoHide + "', " +
                         "force: '" + force + "'");
}

function listToggle(force) {
    debug && console.log("[listToggle 1] " +
                         "listHidden: '" + darkone.listHidden + "', " +
                         "force: '" + force + "'");
    if (darkone.toolbarShowMenuLock)
        return;
    darkone.overlayDuration = 1000;
    resetOverlaySnapTimer.start();
    if (force > 0 || (darkone.listHidden && !force)) {
        darkone.listHidden = false;
        machineListView.focus = true;
    } else if (force < 0 || (!darkone.listHidden && !force)) {
        darkone.listHidden = true;
        machineListView.focus && focus(1);
    }
    debug && console.log("[listToggle 2] " +
                         "listHidden: '" + darkone.listHidden + "', " +
                         "force: '" + force + "'");
}

function fullScreenToggle() {
    if ( darkone.initialised ) {
        if ( darkone.fullScreen )
            viewer.switchToWindowed();
        else
            viewer.switchToFullScreen();
        darkone.fullScreen = !darkone.fullScreen;
    }
}

function lightToggle(force) {
    if (!darkone.initialised || fadeIn.running)
        return;
    if (force > 0 || (darkone.lightOff && !force)) {
        darkone.ignoreLaunch = darkone.preferencesLaunchLock ? true : false
        darkone.lightOut = false;
        darkone.lightOutScreen = false;
        resetListHidden && listToggle(1);
        resetToolbarHidden && toolbarToggle(1);
        debug && console.log("[lightToggle on 1] resetScale: '" + resetScale + "', " +
                             "overlayScale: '" + darkone.overlayScale + "'");
        if (resetScale != -1) {
            zoom(resetScale / darkone.overlayScale);
            resetScale = -1;
        }
        debug && console.log("[lightToggle on 2] resetScale: '" + resetScale + "', " +
                             "overlayScale: '" + darkone.overlayScale + "'");
        if (!darkone.keepLightOn) {
            lightOutTimer.restart();
            lightOutScreenTimer.restart();
        }
        resetFocused = "";
        focus(1);
    } else if (force < 0 || (!darkone.lightOff && !force)) {
        focus(-1);
        darkone.ignoreLaunch = true
        debug && console.log("[lightToggle off 1] resetScale: '" + resetScale + "', " +
                             "overlayScale: '" + darkone.overlayScale + "'");
        if (resetListHidden)
            //do NOT interfere with whatever else is expecting to 'reset listHidden (aka show the list)'
            true // no-op
        else
            resetListHidden = !darkone.listHidden;
        listToggle(-1);
        if (resetToolbarHidden)
            //do NOT interfere with whatever else is expecting to 'reset toolbarHidden (aka show the toolbar)'
            true // no-op
        else
            resetToolbarHidden = !darkone.toolbarHidden;
        toolbarToggle(-1);
        overlayStateBlock.opacity = 0
        if (resetScale == -1) {
            resetScale = darkone.overlayScale;
            zoom(0.5);
        }
        debug && console.log("[lightToggle off 2] resetScale: '" + resetScale + "', " +
                             "overlayScale: '" + darkone.overlayScale + "'");
        lightOutTimer.stop();
        darkone.lightOut = true;
    }
}

function round(number, dp) {
    return Math.round(number * Math.pow(10, dp)) / Math.pow(10, dp);
}

function zoom(vzoom) {
    debug && console.log(
                "[zoom] " +
                //        "scale: window (x,y): ' " + round(scaleFactorX(), 2) + "," + round(scaleFactorY(), 2) + "', " +
                //        "scale: screen: '" + round(darkone.overlayScale, 2) + "', " +
                //        "scale: combined:' " + round(scaleFactorX() * darkone.overlayScale, 2) + "', " +
                //        "darkone.height: '" + round(darkone.height, 2) + "', " +
                //        "overlayScreen.anchors.topMargin: '" + round(overlayScreen.anchors.topMargin, 2) + "', " +
                //        "overlayScreen.y: '" + round(overlayScreen.y, 2) + "', " +
                //        "overlayScreen.parent.y: '" + round(overlayScreen.parent.y, 2) + "', " +
                //        "overlayScreen.height: '" + round(overlayScreen.height, 2) + "', " +
                //        "darkone.height: '" + round(baseHeight, 2) + "', " +
                //        "overlayScreen.height scaled: '" + round(overlayScreen.height * overlayScreen.scale, 2) + "', " +
                //        "overlayScreen.height scaled comp: '" + round(overlayScreen.height * scaleFactorY() * darkone.overlayScale, 2) + "', " +
                //        "darkone.height scaled: '" + round(darkone.height, 2) + "'");
                "overlayScreen.height: '" + overlayScreen.height + "', " +
                "overlayScaleMax: '" + overlayScaleMax + "'");

    switch(typeof(vzoom)) {
    case "string":
        debug && console.log("[zoom] extreme hit: '" + vzoom + "'");
        if (vzoom == "max") {
            debug && console.log("[zoom max 1] darkone.overlayScale: '" + darkone.overlayScale + "', " +
                                 "overlayScreen.scale: '" + overlayScreen.scale + "'");
            vzoom = round(baseHeight / overlayScreen.height, 2);
            darkone.zoomDuration = Math.max(200, Math.abs(vzoom - darkone.overlayScale) * 100 * 8);
            debug && console.log("[zoom max] vzoom: '" + vzoom + "', " +
                                 "duration: '" + darkone.zoomDuration + "'");
            darkone.overlayScale = vzoom;
            debug && console.log("[zoom max 2] darkone.overlayScale: '" + darkone.overlayScale + "', " +
                                 "overlayScreen.scale: '" + overlayScreen.scale + "'");
        } else if (vzoom == "min") {
            vzoom = round(overlayScaleMin, 2);
            darkone.zoomDuration = Math.max(200, Math.abs(darkone.overlayScale - vzoom) * 100 * 8);
            darkone.overlayScale = vzoom;
        }
        break;
    default:
        darkone.zoomDuration = Math.max(200, Math.abs(vzoom - darkone.overlayScale) * 100 * 8);
        debug && console.log("[zoom] vzoom: " + vzoom + ", " +
                             "duration: " + darkone.zoomDuration + ",");
        if (vzoom > 1) {
            debug && console.log("[zoom > 1 1] darkone.overlayScale: '" + darkone.overlayScale + "', " +
                                 "overlayScreen.scale: '" + overlayScreen.scale + "'");
            debug && console.log("[zoom] testing on: '" +
                                 round(((overlayScreen.height * darkone.overlayScale * vzoom) / baseHeight), 2) + " > 0.95'");
            if (((overlayScreen.height * darkone.overlayScale * vzoom) / baseHeight ) > 0.95)
                zoom("max");
            else
                darkone.overlayScale = round(darkone.overlayScale * vzoom, 2);
            debug && console.log("[zoom > 1 2] darkone.overlayScale: '" + darkone.overlayScale + "', " +
                                 "overlayScreen.scale: '" + overlayScreen.scale + "'");
        } else if (vzoom < 1) {
            debug && console.log("[zoom < 1 1] darkone.overlayScale: '" + darkone.overlayScale + "', " +
                                 "overlayScreen.scale: '" + overlayScreen.scale + "'");
            if (darkone.overlayScale * vzoom < overlayScaleMin)
                zoom("min");
            else
                darkone.overlayScale = round(darkone.overlayScale * vzoom, 2);
            debug && console.log("[zoom < 1 2] darkone.overlayScale: '" + darkone.overlayScale + "', " +
                                 "overlayScreen.scale: '" + overlayScreen.scale + "'");
        }
        break;
    }
}

function launchDelay() {
    if (flashCounter == darkone.flashes) {
        launchFlashTimer.stop();
        flashCounter = 0;
        darkone.launchZoom && zoom("max");
        launchTimer.start()
    } else {
        if (launchButton.opacity < 1) {
            launchButton.opacity = 1.0;
            overlayStateBlock.opacity = 1.0;
        } else {
            launchButton.opacity = 0.5;
            overlayStateBlock.opacity = 0.5;
        }
        flashCounter += 1;
    }
}

function launch() {
    focus(-1);
    darkone.forceActiveFocus();
    resetListHidden = !darkone.listHidden;
    listToggle(-1);
    resetToolbarHidden = !darkone.toolbarHidden;
    toolbarToggle(-1);
    resetScale = darkone.overlayScale;
    if (!darkone.launchFlash) {
        darkone.launchZoom && zoom("max");
        launchTimer.start();
    } else {
        launchFlashTimer.start();
    }
}

function gameOn() {
    debug && console.log("[gameOn]");
    inGame = true;
}

function gameOver() {
    if (inGame) {
        debug && console.log("[gameOver] resetScale: '" + resetScale + "', " +
                             "resetToolbarHidden: '" + resetToolbarHidden + "', " +
                             "resetListHidden: '" + resetListHidden + "'");
        inGame = false;
        launchButton.opacity = 0.5;
        overlayStateBlock.opacity = (lightOnAnimation.running || darkone.lightOut) ? 0 : 0.5;
        resetListHidden && listToggle(1);
        resetToolbarHidden && toolbarToggle(1);
        if (darkone.launchZoom && resetScale != -1) {
            zoom(resetScale / darkone.overlayScale);
            resetScale = -1;
        }
        resetFocused = ""
        focus(1);
    }
}

function gameCardHeader() {

    if (!darkone.initialised)
        return "";

    var gameObject = machineListModel[machineListView.currentIndex];
    return "<html><head><style type='text/css'>p, h2 { margin: 0px; }</style></head><h2>" + gameObject.description + "</h2><p>" + qsTr("ID") + ": " + gameObject.id + " / " + qsTr("ROM state") + ": " + viewer.romStateText(gameObject.romState) + "</p></html>";
}

// find datatype (object) by (property) name in (object) dataTypes, and return an 'adjacent' datatype's name
// type: key (property) of datatype to find in datatypes
// offset: integer, positive for next adjacent, negative for previous adjacent
function adjDataType(type, offset) {
    var first;
    var current;
    var previous;
    var target;
    var set = false;
    var dataType;
    for (dataType in dataTypes) {
        current = dataTypes[dataType].key;
        if (set) {
            if (offset > 0) {
                target = current;
                break;
            }
        } else if (current == type) {
            set = true;
            if (offset < 0 && previous) {
                target = previous;
                break;
            }
        } else if (!first)
            first=current
        previous = current
    }
    if (set && !target) {
        if (offset > 0)
            target = first
        else if (offset < 0)
            target = current
    }
    debug && console.log("[adjDataType] type: '" + type + "', " +
                         "offset: '" + offset + "', " +
                         "target: '" + target + "'");
    return target;
}

function datatypeKeys() {
    var items = [];
    var l;
    for (l in dataTypes)
    {
        debug && console.log("[datatypeKeys] key: '" + dataTypes[l].key + "'");
        items.push(dataTypes[l].key);
    }
    return items;
}

function data(type) {

    debug && console.log("[data] type: '" + type + "', " +
                         "dataTypeCurrent: '" + darkone.dataTypeCurrent + "', " +
                         "dataTypePrimary: '" + darkone.dataTypePrimary + "', " +
                         "dataTypeSecondary: '" + darkone.dataTypeSecondary + "'");
    switch (type) {
    case "name":
        return dataTypes[darkone.dataTypeCurrent].name;
        break;

    case "image":
        if (!darkone.initialised)
            return "image://qmc2/ghost";

        var image = ""
        if (darkone.dataHidden) {
            if (dataTypes[darkone.dataTypePrimary].type == "image") {
                var glItem = machineListModel[machineListView.currentIndex];
                if ( viewer.loadImage(dataTypes[darkone.dataTypePrimary].path + "/" + glItem.id + "/" + glItem.parentId) != "" ) {
                    image = "image://qmc2/" + dataTypes[darkone.dataTypePrimary].path + "/" + glItem.id + "/" + glItem.parentId;
                    debug && console.log("[data] success using image path: '" + image + "'");
                }
            }
            if (image == "" && dataTypes[darkone.dataTypeSecondary].type == "image") {
                var glItem = machineListModel[machineListView.currentIndex];
                if ( viewer.loadImage(dataTypes[darkone.dataTypeSecondary].path + "/" + glItem.id + "/" + glItem.parentId) != "") {
                    image = "image://qmc2/" + dataTypes[darkone.dataTypeSecondary].path + "/" + glItem.id + "/" + glItem.parentId;
                    debug && console.log("[data] success using image path: '" + image + "'");
                }
            }
            if (image == "" && ( dataTypes[darkone.dataTypePrimary].type == "image" || dataTypes[darkone.dataTypeSecondary].type == "image"))
                image = "image://qmc2/ttl/default" // default
        } else {
            if (dataTypes[darkone.dataTypeCurrent].type == "image") {
                var glItem = machineListModel[machineListView.currentIndex];
                if ( viewer.loadImage(dataTypes[darkone.dataTypeCurrent].path + "/" + glItem.id + "/" + glItem.parentId) != "") {
                    image = "image://qmc2/" + dataTypes[darkone.dataTypeCurrent].path + "/" + glItem.id + "/" + glItem.parentId;
                    debug && console.log("[data] success using image path: '" + image + "'");
                }
            }
            if (image == "" && dataTypes[darkone.dataTypeCurrent].type == "image")
                image = "image://qmc2/ttl/default" // default
        }
        debug && console.log("[data] using image path: '" + image + "'");
        return image;
        break;

    case "text":
        if (!darkone.initialised)
            return "";

        var info = ""
        var type = ""
        if (darkone.dataHidden) {
            if (dataTypes[darkone.dataTypePrimary].type == "text") {
                type = dataTypes[darkone.dataTypePrimary].text
                info = viewer.requestInfo(machineListModel[machineListView.currentIndex].id, dataTypes[darkone.dataTypePrimary].key);
                if (!info.match(qsTr("no info available"))) {
                    debug && console.log("[data] using text type: '" + type + "'");
                }
            }
            if (type == "" && dataTypes[darkone.dataTypeSecondary].type == "text") {
                type = dataTypes[darkone.dataTypeSecondary].text
                info = viewer.requestInfo(machineListModel[machineListView.currentIndex].id, dataTypes[darkone.dataTypeSecondary].key);
                if (!info.match(qsTr("no info available"))) {
                    debug && console.log("[data] using text type: '" + type + "'");
                }
            }
        }
        else {
            if (dataTypes[darkone.dataTypeCurrent].type == "text") {
                type = dataTypes[darkone.dataTypeCurrent].text
                info = viewer.requestInfo(machineListModel[machineListView.currentIndex].id, dataTypes[darkone.dataTypeCurrent].key);
            }
        }
        info.match(qsTr("no info available")) ? darkone.infoMissing = true : darkone.infoMissing = false;
        debug && console.log("[data] infoMissing: '" + darkone.infoMissing + "', " +
                             "info: '" + "info" + "'")
        return type == "" ? "" : "<html><head><style type='text/css'>p, h3 { margin: 0px; }</style></head>" + "<h3>" + type + "</h3>" + "<p>" + info + "</p></html>";
        break;
    }
}

function gameStatusColour() {

    if (!darkone.initialised)
        return "transparent";

    debug && console.log("romState: '" + machineListModel[machineListView.currentIndex].romState + "'");
    switch ( machineListModel[machineListView.currentIndex].romState ) {
    case 0:
        // correct
        return "#00ce00";
    case 1:
        // mostly correct
        return "#d2bc00";
    case 2:
        // incorrect
        return "#ff0000";
    case 3:
        // notfound
        return "#cccacb";
    case 4:
    default:
        // unknown
        return "#0030c6";
    }
}

function keyEvent2String(e) {

    var mods = "";
    if ( e.modifiers & Qt.ControlModifier )
        mods += "C-"
    if ( e.modifiers & Qt.ShiftModifier )
        mods += "S-"
    if ( e.modifiers & Qt.AltModifier )
        mods += "M-"

    if ( e.text.trim() != "" )
        return mods + e.text;
    else {
        switch ( e.key ) {
        case Qt.Key_Up: return mods + "up"; break;
        case Qt.Key_Down: return mods + "down"; break;
        case Qt.Key_Left: return mods + "left"; break;
        case Qt.Key_Right: return mods + "right"; break;
        case Qt.Key_Tab: case Qt.Key_Backtab: return mods + "tab"; break;
                         case Qt.Key_Space: return mods + "space"; break;
                         case Qt.Key_Enter: case Qt.Key_Return: return mods + "enter"; break;
                                            default: return mods + "undefined";
        }
    }
}

function colourScheme(scheme) {
    if (darkone.colourScheme != scheme)
        darkone.colourScheme = scheme;
    switch (scheme) {
    case "dark":
        colourScheme1Button.checked = true;
        colourScheme1Button.opacity = 1.0;
        darkone.colour1 = "#777777"; // gradient 1
        darkone.colour2 = "#000000"; // gradient 2
        darkone.colour3 = "#333333"; // gradient 3
        darkone.colour4 = "#222222"; // gradient 4
        darkone.colour5 = "#111111"; // menu
        darkone.textColour1 = "#eeeeee"; // primary text
        darkone.textColour2 = "#1769ff"; // list highlighted
        break;
    case "metal":
        colourScheme2Button.checked = true;
        colourScheme2Button.opacity = 1.0;
        darkone.colour1 = "#000000";
        darkone.colour2 = "#ffffff";
        darkone.colour3 = "#aaaaaa";
        darkone.colour4 = "#000000";
        darkone.colour5 = "#999999";
        darkone.textColour1 = "#222222";
        darkone.textColour2 = "#0000ff";
        break;
    }
    debug && console.log("[colourScheme] scheme: '" + scheme + "', " +
                         "colourScheme1Button.checked: '" + colourScheme1Button.checked + "', " +
                         "colourScheme2Button.checked: '" + colourScheme2Button.checked + "'")
}

function inFocus() {
    debug2 && console.log("\n[inFocus]\n" +
                          "darkoneFocusScope: '" + darkoneFocusScope.focus + "|" + darkoneFocusScope.activeFocus + "'\n" +
                          "machineListView: '" + machineListView.focus + "|" + machineListView.activeFocus + "'\n" +
                          "overlay: '" + overlay.focus + "|"  + overlay.activeFocus + "'\n" +
                          "overlayScreen: '" + overlayScreen.focus + "|"  + overlayScreen.activeFocus + "'\n" +
                          "overlayDataTypeCycleItem: '" + overlayDataTypeCycleItem.focus + "|"  + overlayDataTypeCycleItem.activeFocus + "'\n" +
                          "preferencesFocusScope: '" + preferencesFocusScope.focus + "|" + preferencesFocusScope.activeFocus + "'\n" +
                          "toolbar: '" + toolbar.focus + "|" + toolbar.activeFocus + "'\n" +
                          "toolbarFocusScope: '" + toolbarFocusScope.focus + "|" + toolbarFocusScope.activeFocus + "'\n" +
                          "searchTextInput: '" + searchTextInput.focus + "|" + searchTextInput.activeFocus + "'\n" +
                          "")
}

function focus(vFocus) {

    if (!darkone.initialised || fadeIn.running)
        return;

    debug3 && console.log("\n\n[focus()] vFocus: '" + vFocus + "', type: '" + typeof(vFocus) + "'\n" +
                          "          resetFocused: '" + resetFocused + "', focused[]: '" + focused + "'\n");

    if (resetFocused == "") {
        switch ( typeof(vFocus) ) {
        case "string":
            var bAdd = true;
            if (focused.length > 0)
                bAdd = focused[focused.length - 1] != vFocus
            if (bAdd) {
                debug3 && console.log("[focus+] focused[]: '" + focused + "'");
                focused.push(vFocus);
                debug3 && console.log("[focus+] focused[]: '" + focused + "'");
            }
            break;
        default:
            if (vFocus > 0) {
                var bSet = false;
                var last = ""
                while (focused.length > 0 && !bSet) {
                    debug3 && console.log("[focus?] focused[]: '" + focused + "'");
                    last = focused[focused.length - 1];
                    switch ( last ) {
                    case "overlay": {
                        debug3 && console.log("[focus=]: '" + last + "'");
                        overlay.focus = "true";
                        bSet = true;
                        break;
                    }
                    case "machineListView": {
                        if (!darkone.listHidden) {
                            debug3 && console.log("[focus=]: '" + last + "'");
                            machineListView.focus = "true";
                            bSet = true;
                        }
                        break;
                    }
                    case "toolbarFocusScope": {
                        if (!darkone.toolbarHidden) {
                            debug3 && console.log("[focus=]: '" + last + "'");
                            toolbarFocusScope.focus = "true";
                            bSet = true;
                        }
                        break;
                    }
                    }
                    if (!bSet) {
                        debug3 && console.log("[focus-] focused[]: '" + focused + "'");
                        focused.pop();
                        debug3 && console.log("[focus-] focused[]: '" + focused + "'");
                    }
                }
                if (focused.length > 25)
                    focused = focused.slice(focused.length - 10, focused.length - 1);
                if (!bSet) {
                    focused = ["overlay"];
                    overlay.focus = true;
                }
                darkone.activeBorders = true;
            } else {
                resetFocused = focused[focused.length - 1] || "overlay";
                debug3 && console.log("[focus?] resetFocused: '" + resetFocused + "', focused[]: '" + focused + "'");
                darkone.activeBorders = false;
            }
            break;
        }
    }
}

function version(maxmin, verTarget, verCheck) {

    var verTargetPart = verTarget.match(/(\d*)\.(\d*)\.(\d*)/);
    var verCheckPart = verCheck.match(/(\d*)\.(\d*)\.(\d*)/);
    var bMet;
    if (verTargetPart != null && verCheckPart != null) {
        if (verTargetPart.length != verCheckPart.length) {
            bMet = false;
            debug && console.log("[version] comparison failed. version types differ");
        } else {
            bMet = true;
            var bContinue = true;
            while (verTargetPart.length > 0 && bContinue) {
                switch (maxmin) {
                case "min":
                    if (verCheckPart[0] > verTargetPart[0])
                        bContinue = false;
                    else if (verCheckPart[0] < verTargetPart[0])
                        bMet = bContinue = false;
                    break;
                case "max":
                    if (verCheckPart[0] < verTargetPart[0])
                        bContinue = false;
                    else if (verCheckPart[0] > verTargetPart[0])
                        bMet = bContinue = false;
                    break;
                }
                verTargetPart.shift();
                verCheckPart.shift();
            }
        }
    }
    debug2 && console.log("[version] maxmin: '" + maxmin + "', verTarget: '" + verTarget + "', verCheck: '" + verCheck + "', result: '" + bMet + "'");

    return bMet;
}
