var baseWidth = 800;
var baseHeight = 600;
var lastItemText;
var lastitemBackground;
var initializing = true;

function init() {
    viewer.loadSettings();
    initializing = false;
}

function scaleFactorX() {
    return toxicWasteMain.width / baseWidth;
}

function scaleFactorY() {
    return toxicWasteMain.height / baseHeight;
}

function itemEntered(itemText, itemBackground) {
    if ( !itemText.fontResized ) {
        if ( lastItemText != undefined )
            itemExited(lastItemText, lastitemBackground);
        lastItemText = itemText;
        lastitemBackground = itemBackground;
        itemText.fontResized = true;
        itemText.font.pixelSize += 5;
        itemBackground.opacity = 1;
    }
}

function itemExited(itemText, itemBackground) {
    if ( itemText.fontResized ) {
        itemText.fontResized = false;
        itemText.font.pixelSize -= 5;
        itemBackground.opacity = 0.7;
    }
}

function itemClicked(itemText, itemBackground) {
    if ( itemText.fontResized ) {
        itemText.fontResized = false;
        itemText.font.pixelSize -= 5;
        itemBackground.opacity = 0.7;
    }
}

function overlayOffset(h) {
    var offset;
    h = h * overlayImageFront.scale;
    if ( h < toxicWasteMain.height )
        offset = 0;
    else
        offset = -toxicWasteMain.height/2 + h/2;
    return offset;
}

function gameCardHeader() {
    var gameObject = gameListModel[gamelistView.currentIndex];
    return "<h2>" + gameObject.description + "</h2>" + "<p>id: " + gameObject.id + "<br>romState: " + viewer.romStateText(gameObject.romState) + "</p>";
}
