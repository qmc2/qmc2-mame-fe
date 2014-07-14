var baseWidth = 800;
var baseHeight = 600;
var lastItemText;
var lastitemBackground;
var initializing = true;

function init() {
    viewer.loadSettings();
    initializing = false;
    viewer.log("ToxicWaste: " + qsTr("Initialization done"));
}

function restoreLastIndex() {
    if ( lastIndex < gameListModelCount ) {
        gamelistView.currentIndex = lastIndex;
        gamelistView.positionViewAtIndex(lastIndex, ListView.Beginning);
    }
}

function scaleFactorX() {
    return toxicWasteMain.width / baseWidth;
}

function scaleFactorY() {
    return toxicWasteMain.height / baseHeight;
}

function itemEntered(itemText, itemBackground, itemIcon) {
    if ( !itemText.fontResized ) {
        if ( lastItemText !== undefined )
            itemExited(lastItemText, lastitemBackground);
        lastItemText = itemText;
        lastitemBackground = itemBackground;
        itemText.fontResized = true;
        itemText.font.pixelSize += 5;
        itemBackground.opacity = 1;
        if ( itemIcon !== undefined )
            itemIcon.height += 5;
    }
}

function itemExited(itemText, itemBackground, itemIcon) {
    if ( itemText.fontResized ) {
        itemText.fontResized = false;
        itemText.font.pixelSize -= 5;
        itemBackground.opacity = 0.8;
        if ( itemIcon !== undefined )
            itemIcon.height -= 5;
    }
}

function itemClicked(itemText, itemBackground, itemIcon) {
    if ( itemText.fontResized ) {
        itemText.fontResized = false;
        itemText.font.pixelSize -= 5;
        itemBackground.opacity = 0.8;
        if ( itemIcon !== undefined )
            itemIcon.height -= 5;
    }
}

function overlayOffset(h) {
    var offset;
    h *= overlayImage.scale;
    if ( h < toxicWasteMain.height )
        offset = 0;
    else
        offset = -toxicWasteMain.height/2 + h/2;
    return offset;
}

function gameCardHeader() {
    var gameObject = gameListModel[gamelistView.currentIndex];
    return "<h2>" + gameObject.description + "</h2>" + "<p>"+ qsTr("ID") + ": " + gameObject.id + " / " + qsTr("ROM state") + ": " + viewer.romStateText(gameObject.romState) + "</p>";
}

function nextImageType(imageType) {
    var nextType;
    switch ( imageType ) {
    case "flyer":
        nextType = "cabinet";
        break;
    case "cabinet":
        nextType = "controller";
        break;
    case "controller":
        nextType = "marquee";
        break;
    case "marquee":
        nextType = "title";
        break;
    case "title":
        nextType = "pcb";
        break;
    case "pcb":
        nextType = "preview";
        break;
    case "preview":
    default:
        nextType = "flyer";
        break;
    }
    return nextType;
}

function previousImageType(imageType) {
    var previousType;
    switch ( imageType ) {
    case "flyer":
        previousType = "preview";
        break;
    case "cabinet":
        previousType = "flyer";
        break;
    case "controller":
        previousType = "cabinet";
        break;
    case "marquee":
        previousType = "controller";
        break;
    case "title":
        previousType = "marquee";
        break;
    case "pcb":
        previousType = "title";
        break;
    case "preview":
    default:
        previousType = "pcb";
        break;
    }
    return previousType;
}

function gameImageType(imageType) {
    var typeName;
    switch ( imageType ) {
    case "flyer":
        typeName = qsTr("Flyer image");
        break;
    case "cabinet":
        typeName = qsTr("Cabinet image");
        break;
    case "controller":
        typeName = qsTr("Controller image");
        break;
    case "marquee":
        typeName = qsTr("Marquee image");
        break;
    case "title":
        typeName = qsTr("Title image");
        break;
    case "pcb":
        typeName = qsTr("PCB image");
        break;
    case "preview":
    default:
        typeName = qsTr("Preview image");
        break;
    }
    return typeName;
}

function cachePrefix(imageType) {
    var prefix = "prv";
    switch ( imageType ) {
    case "flyer":
        prefix = "fly";
        break;
    case "cabinet":
        prefix = "cab";
        break;
    case "controller":
        prefix = "ctl";
        break;
    case "marquee":
        prefix = "mrq";
        break;
    case "title":
        prefix = "ttl";
        break;
    case "pcb":
        prefix = "pcb";
        break;
    case "icon":
        prefix = "ico";
        break;
    case "preview":
    default:
        prefix = "prv";
        break;
    }
    return prefix;
}

function imageUrl(imageType) {
    var imgUrl = "image://qmc2/" + cachePrefix(imageType) + "/" + gameListModel[gamelistView.currentIndex].id + "/" + gameListModel[gamelistView.currentIndex].parentId;;
    return imgUrl;
}

function launchButtonSource() {
    var buttonSource = "images/launch_";
    switch ( gameListModel[gamelistView.currentIndex].romState ) {
    case 0:
        buttonSource += "correct.png";
        break;
    case 1:
        buttonSource += "mostlycorrect.png";
        break;
    case 2:
        buttonSource += "incorrect.png";
        break;
    case 3:
        buttonSource += "notfound.png";
        break;
    case 4:
    default:
        buttonSource += "unknown.png";
        break;
    }
    return buttonSource;
}

function validateKey(k) {
    if ( /[^a-zA-Z0-9\*\?$]/.test(k) || k === "" )
        return false;
    else
        return true;
}

function validateSpecialKey(k) {
    if ( /[^\b$]/.test(k) || k === "" )
        return false;
    else
        return true;
}

function emulatorStarted() {
    if ( toxicWasteMain.autoStopAnimations ) {
        waveEffect.running = false;
        backgroundAnim.opacity = 0.0;
    }
}

function emulatorStopped() {
    if ( toxicWasteMain.autoStopAnimations && viewer.runningEmulators() === 1 ) {
        if ( toxicWasteMain.showShaderEffect )
            waveEffect.running = true;
        if ( toxicWasteMain.showBackgroundAnimation )
            backgroundAnim.opacity = 1.0;
    }
}
