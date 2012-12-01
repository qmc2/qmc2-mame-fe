var lastItemText;
var lastitemBackground;

function init() {
    gamelistModel.clear();
    for (var i = 1; i < 101; i++)
        gamelistModel.append({"name": "Item " + i, "id": i});
}

function baseWidth() {
    return 800;
}

function baseHeight() {
    return 600;
}

function scaleFactor() {
    return toxicWasteMain.width / baseWidth();
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
    gamenameText.text = itemText.text + " clicked!"
    if ( itemText.fontResized ) {
        itemText.fontResized = false;
        itemText.font.pixelSize -= 5;
        itemBackground.opacity = 0.7;
    }
}
