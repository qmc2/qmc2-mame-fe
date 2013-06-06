var baseWidth = 800;
var baseHeight = 600;
var baseListWidth = 250;
var lastItemText;
var lastitemBackground;
var resetZoom;
var resetListHidden;
var resetToolbarHidden;
var flashCounter = 0;
var inGame = false;
var dataTypes = { "title": { key: "title", name: "title image", type: "image", path: "ttl" },
                  "preview": { key: "preview", name: "preview image", type: "image", path: "prv" },
                  "flyer": { key: "flyer", name: "flyer image", type: "image", path: "fly" },
                  "cabinet": { key: "cabinet", name: "cabinet image", type: "image", path: "cab" },
                  "controller": { key: "controller", name: "controller image", type: "image", path: "ctl" },
                  "marquee": { key: "marquee", name: "marquee image", type: "image", path: "mrq" },
                  "pcb": { key: "pcb", name: "pcb image", type: "image", path: "pcb" },
                  "gameinfo": { key: "gameinfo", name: "game info", type: "text", text: "[game info]" },
                  "emuinfo": { key: "emuinfo", name: "emulator info", type: "text", text: "[emulator info]" } }

function init() {
    if (!darkone.initialised && darkone.lastIndex == -1) {
        viewer.loadSettings();
        colourScheme(darkone.colourScheme);
        viewer.loadGamelist();
        darkone.debug && console.log("[init] lastIndex: '" + darkone.lastIndex + "', " +
                                            "gameListModelCount: '" + gameListModelCount + "'");
        gameListView.currentIndex = lastIndex < gameListModelCount && lastIndex > -1 ? lastIndex : 0;
        gameListView.positionViewAtIndex(gameListView.currentIndex, ListView.Center);
        darkone.debug && console.log("[init] lastIndex: '" + lastIndex + "', " +
                                            "gameListModelCount: '" + gameListModelCount + "'");
        darkone.dataTypeCurrent = darkone.dataTypePrimary;
        listToggle(1 - (darkone.listHidden * 2));
        if (fpsVisible)
            toolbarShowFpsLock = true;
        if (!keepLightOn) {
            lightOutTimer.start();
            lightOutScreenTimer.start();
        }
        darkone.keepLightOn = (darkone.lightTimeout == 0) ? true : false;
        toolbarToggle(darkone.toolbarAutoHide ? -1 : 1 - (darkone.toolbarHidden * 2));
        fadeIn.start();
    } else if(!fadeIn.running) {
        darkone.initialised = true;
        initTimer.stop();
        //power
        lightOut = false;
        lightOutScreen = false;
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
    darkone.debug && console.log("[itemEntered] " +
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
    darkone.debug && console.log("[toolbarToggle 1] " +
                                 "toolbarHidden: '" + darkone.toolbarHidden + "', " +
                                 "toolbarAutoHide: '" + darkone.toolbarAutoHide + "', " +
                                 "force: '" + force + "'");
    if (force > 0 || (darkone.toolbarHidden && !force)) {
        toolbar.state = "shown"
        darkone.toolbarHidden = false;
    } else if ((force < 0 || (!darkone.toolbarHidden && !force)) &&
                !toolbarShowMenuLock && !toolbarShowFpsLock) {
        toolbar.state = "hidden"
        darkone.toolbarHidden = true;
    }
    searchTextInput.focus = false;
    darkone.debug && console.log("[toolbarToggle 2] " +
                                 "toolbarHidden: '" + darkone.toolbarHidden + "', " +
                                 "toolbarAutoHide: '" + darkone.toolbarAutoHide + "', " +
                                 "force: '" + force + "'");
}

function listToggle(force) {
    darkone.debug && console.log("[listToggle 1] " +
                                 "listHidden: '" + darkone.listHidden + "', " +
                                 "force: '" + force + "'");
    if (toolbarShowMenuLock)
        return
    darkone.overlayDuration = 1000;
    resetOverlaySnapTimer.start();
    if (force > 0 || (darkone.listHidden && !force)) {
        gameListView.state = "shown"
        darkone.listHidden = false;
        searchBox.opacity = 1.0;
        showListButton.anchors.left = searchBox.right
        showListButton.rotation = 270;
    } else if (force < 0 || (!darkone.listHidden && !force)) {
        gameListView.state = "hidden"
        darkone.listHidden = true;
        searchBox.opacity = 0;
        showListButton.anchors.left = showListButton.parent.left
        showListButton.rotation = 90;
    }
    searchTextInput.focus = false;
    darkone.debug && console.log("[listToggle 2] " +
                                 "listHidden: '" + darkone.listHidden + "', " +
                                 "force: '" + force + "'");
}

function lightToggle(force) {
    if (force > 0 || (darkone.lightOff && !force)) {
        ignoreLaunch = preferencesLaunchLock ? true : false
        lightOut = false;
        lightOutScreen = false;
        resetListHidden && listToggle(1);
        resetToolbarHidden && toolbarToggle(1);
        !darkone.disableLaunchZoom && zoom(resetZoom / darkone.overlayScale);
        if (!keepLightOn) {
            lightOutTimer.restart();
            lightOutScreenTimer.restart();
        }
    } else if (force < 0 || (!darkone.lightOff && !force)) {
        ignoreLaunch = true
        resetListHidden = !darkone.listHidden;
        listToggle(-1);
        resetToolbarHidden = !darkone.toolbarHidden;
        toolbarToggle(-1);
        overlayStateBlock.opacity = 0
        if (resetZoom == 1 && !darkone.disableLaunchZoom) {
            resetZoom = darkone.overlayScale;
            zoom("0.5");
        }
        lightOutTimer.stop();
        lightOut = true;
    }
}

function round(number, dp) {
    return Math.round(number * Math.pow(10, dp)) / Math.pow(10, dp)
}

function zoom(zoom) {
    darkone.debug && console.log(
        "[zoom] " +
        "scale: window (x,y): ' " + round(scaleFactorX(), 2) + "," + round(scaleFactorY(), 2) + "', " +
        "scale: screen: '" + round(darkone.overlayScale, 2) + "', " +
//        "scale: combined:' " + round(scaleFactorX() * darkone.overlayScale, 2) + "', " +
//        "darkone.height: '" + round(darkone.height, 2) + "', " +
//        "overlayScreen.anchors.topMargin: '" + round(overlayScreen.anchors.topMargin, 2) + "', " +
//        "overlayScreen.y: '" + round(overlayScreen.y, 2) + "', " +
//        "overlayScreen.parent.y: '" + round(overlayScreen.parent.y, 2) + "', " +
        "overlayScreen.height: '" + round(overlayScreen.height, 2) + "', " +
        "darkone.height: '" + round(baseHeight, 2) + "', " +
        "overlayScreen.height scaled: '" + round(overlayScreen.height * overlayScreen.scale, 2) + "', " +
        "overlayScreen.height scaled comp: '" + round(overlayScreen.height * scaleFactorY() * darkone.overlayScale, 2) + "', " +
        "darkone.height scaled: '" + round(darkone.height, 2) + "'")

      switch(typeof(zoom)) {
          case "string":
              if (zoom == "max") {
                  darkone.debug && console.log("[zoom max 1] darkone.overlayScale: '" + darkone.overlayScale + "', " +
                                                            "overlayScreen.scale: '" + overlayScreen.scale + "'");
                  zoom = (darkone.height / (overlayScreen.height * scaleFactorY()));
                  darkone.zoomDuration = Math.max(200, Math.abs(zoom - darkone.overlayScale) * 100 * 8);

                  darkone.debug && console.log("[zoom max] zoom: '" + zoom + "', " +
                                                          "duration: '" + darkone.zoomDuration + "'");
                  darkone.overlayScale = zoom;
                  darkone.debug && console.log("[zoom max 2] darkone.overlayScale: '" + darkone.overlayScale + "', " +
                                                            "overlayScreen.scale: '" + overlayScreen.scale + "'");
              }
              break;
        default:
            darkone.zoomDuration = Math.max(200, Math.abs(zoom - darkone.overlayScale) * 100 * 8);
            darkone.debug && console.log("[zoom] zoom: " + zoom + ", " +
                                                "duration: " + darkone.zoomDuration + ",");
            if (zoom > 1) {
                darkone.debug && console.log("[zoom > 1 1] darkone.overlayScale: '" + darkone.overlayScale + "', " +
                                                          "overlayScreen.scale: '" + overlayScreen.scale + "'");
                darkone.debug && console.log("[zoom] testing on: '" +
                    round(((overlayScreen.height * scaleFactorY() * darkone.overlayScale * zoom) / darkone.height), 2) + " > 0.95'");

                if (((overlayScreen.height * scaleFactorY() * darkone.overlayScale * zoom) / darkone.height ) > 0.95)
                    DarkoneJS.zoom("max");
                else
                    darkone.overlayScale *= zoom;
                    darkone.debug && console.log("[zoom > 1 2] darkone.overlayScale: '" + darkone.overlayScale + "', " +
                                                              "overlayScreen.scale: '" + overlayScreen.scale + "'");
            } else if (zoom < 1 && overlayScreen.scale * zoom >= 0.33) {
                darkone.debug && console.log("[zoom < 1 1] darkone.overlayScale: '" + darkone.overlayScale + "', " +
                                                          "overlayScreen.scale: '" + overlayScreen.scale + "'");
                darkone.overlayScale *= zoom;
                darkone.debug && console.log("[zoom < 1 2] darkone.overlayScale: '" + darkone.overlayScale + "', " +
                                                          "overlayScreen.scale: '" + overlayScreen.scale + "'");
            }
            break;
    }
}

function launchFlash() {
    if (flashCounter == darkone.flashes) {
        launchFlashTimer.stop();
        flashCounter = 0;
        !darkone.disableLaunchZoom && zoom("max");
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
     searchTextInput.focus = false;
     resetListHidden = !darkone.listHidden;
     listToggle(-1);
     resetToolbarHidden = !darkone.toolbarHidden;
     toolbarToggle(-1);
     resetZoom = darkone.overlayScale;
     if (darkone.disableLaunchFlash) {
         !darkone.disableLaunchZoom && zoom("max");
         launchTimer.start();
     } else {
         launchFlashTimer.start();
     }
}

function gameOn() {
     darkone.debug && console.log("[gameOn]");
     inGame = true;
}

function gameOver() {
    if (inGame) {
        darkone.debug && console.log("[gameOver] resetZoom: '" + resetZoom + "', " +
                                                "resetToolbarHidden: '" + resetToolbarHidden + "', " +
                                                "resetListHidden: '" + resetListHidden + "'");
        inGame = false;
        launchButton.opacity = 0.5;
        overlayStateBlock.opacity = (lightOnAnimation.running || lightOut) ? 0 : 0.5;
        resetListHidden && listToggle(1);
        resetToolbarHidden && toolbarToggle(1);
        !darkone.disableLaunchZoom && zoom(resetZoom / darkone.overlayScale);
        !darkone.toolbarHidden ? searchTextInput.focus = true : true;
    }
}

function gameCardHeader() {

    if (!initialised)
        return ""

          var gameObject = gameListModel[gameListView.currentIndex];
    return "<style type='text/css'>p,h2 { margin:0px }</style><h2>" + gameObject.description + "</h2><p>" + qsTr("ID") + ": " + gameObject.id + " / " + qsTr("ROM state") + ": " + viewer.romStateText(gameObject.romState) + "</p>";
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
    darkone.debug && console.log("[adjDataType] type: '" + type + "', " +
                                                "offset: '" + offset + "', " +
                                                "target: '" + target + "'");
    return target;
}

function data(type) {

    if (!initialised)
        return ""

    darkone.debug && console.log("[data] type: '" + type + "', " +
                                        "dataTypeCurrent: '" + darkone.dataTypeCurrent + "', " +
                                        "dataTypePrimary: '" + darkone.dataTypePrimary + "', " +
                                        "dataTypeSecondary: '" + darkone.dataTypeSecondary + "'");
    switch (type) {
        case "image":
            var image = ""
            var image2 = ""
            if (darkone.dataHidden) {
                if (dataTypes[darkone.dataTypePrimary].type == "image") {
                     if ( viewer.loadImage(dataTypes[darkone.dataTypePrimary].path + "/" + gameListModel[gameListView.currentIndex].id) != "" ) {
                         image = "image://qmc2/" + dataTypes[darkone.dataTypePrimary].path + "/" + gameListModel[gameListView.currentIndex].id;
                         darkone.debug && console.log("[data] success using image path: '" + image + "'");
                    }
                }
                if (image == "" && dataTypes[darkone.dataTypeSecondary].type == "image") {
                    if ( viewer.loadImage(dataTypes[darkone.dataTypeSecondary].path + "/" + gameListModel[gameListView.currentIndex].id) != "") {
                         image = "image://qmc2/" + dataTypes[darkone.dataTypeSecondary].path + "/" + gameListModel[gameListView.currentIndex].id;
                         darkone.debug && console.log("[data] success using image path: '" + image + "'");
                    }
                }
                if (image == "" && ( dataTypes[darkone.dataTypePrimary].type == "image" || dataTypes[darkone.dataTypeSecondary].type == "image"))
                   image = "image://qmc2/ttl/default" // default
            } else {
                if (dataTypes[darkone.dataTypeCurrent].type == "image") {
                    if ( viewer.loadImage(dataTypes[darkone.dataTypeCurrent].path + "/" + gameListModel[gameListView.currentIndex].id) != "") {
                         image = "image://qmc2/" + dataTypes[darkone.dataTypeCurrent].path + "/" + gameListModel[gameListView.currentIndex].id;
                         darkone.debug && console.log("[data] success using image path: '" + image + "'");
                    }
                }
                if (image == "" && dataTypes[darkone.dataTypeCurrent].type == "image")
                   image = "image://qmc2/ttl/default" // default
            }
            darkone.debug && console.log("[data] using image path: '" + image + "'");
            return image;
            break;
        case "text":
            var info = ""
            var type = ""
            if (darkone.dataHidden) {
                 if (dataTypes[darkone.dataTypePrimary].type == "text") {
                     type = dataTypes[darkone.dataTypePrimary].text
                     info = viewer.requestInfo(gameListModel[gameListView.currentIndex].id, dataTypes[darkone.dataTypePrimary].key);
                     if (!info.match("no info available")) {
                         darkone.debug && console.log("[data] using text type: '" + type + "'");
                     }
                }
                if (type == "" && dataTypes[darkone.dataTypeSecondary].type == "text") {
                    type = dataTypes[darkone.dataTypeSecondary].text
                    info = viewer.requestInfo(gameListModel[gameListView.currentIndex].id, dataTypes[darkone.dataTypeSecondary].key);
                    if (!info.match("no info available")) {
                        darkone.debug && console.log("[data] using text type: '" + type + "'");
                    }
                }
            }
            else {
                if (dataTypes[darkone.dataTypeCurrent].type == "text") {
                    type = dataTypes[darkone.dataTypeCurrent].text
                    info = viewer.requestInfo(gameListModel[gameListView.currentIndex].id, dataTypes[darkone.dataTypeCurrent].key);
                }
            }
            info.match("no info available") ? darkone.infoMissing = true : darkone.infoMissing = false;
            darkone.debug && console.log("[data] infoMissing: '" + darkone.infoMissing + "', " +
                                                "info: '" + "info" + "'")
            return type == "" ? "" : "<style type='text/css'>p,h3 { margin:0px }</style>" + "<h3>" + type + "</h3>" + "<p>" + info + "</p>";
            break;
        case "name":
            return dataTypes[darkone.dataTypeCurrent].name;
            break;
    }
}

function gameStatusColour() {

    if (!initialised)
        return "transparent"

    darkone.debug && console.log("romState: '" + gameListModel[gameListView.currentIndex].romState + "'");
    switch ( gameListModel[gameListView.currentIndex].romState ) {
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

function validateKey(k) {
    if ( /[^a-zA-Z0-9\*\?$]/.test(k) || k == "" )
        return false;
    else
        return true;
}

function validateSpecialKey(k) {
    if ( /[^\b$]/.test(k) || k == "" )
        return false;
    else
        return true;
}

function colourScheme(scheme) {
    switch (scheme) {
        case "dark" :
            colourScheme1Button.checked = true;
            colourScheme1Button.opacity = 1.0;
            colour1 = "#777777"; // gradient 1
            colour2 = "#000000"; // gradient 2
            colour3 = "#333333"; // gradient 3
            colour4 = "#222222"; // gradient 4
            colour5 = "#111111"; // menu
            textColour1 = "#eeeeee"; // primary text
            textColour2 = "#1769ff"; // list highlighted
            break;
        case "metal":
            colourScheme2Button.checked = true;
            colourScheme2Button.opacity = 1.0;
            colour1 = "#000000";
            colour2 = "#ffffff";
            colour3 = "#aaaaaa";
            colour4 = "#000000";
            colour5 = "#999999";
            textColour1 = "#000000"
            textColour2 = "#0000ff"
            break;
    }
    darkone.debug && console.log("[colourScheme] scheme: '" + scheme + "', " +
                                                "colourScheme1Button.checked: '" + colourScheme1Button.checked + "', " +
                                                "colourScheme2Button.checked: '" + colourScheme2Button.checked + "'")
}
