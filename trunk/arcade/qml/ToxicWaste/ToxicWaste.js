var lastItemText;
var lastItemImage;
var currentItemText;
var currentItemImage;

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
    var factor = toxicWasteMain.width / baseWidth();
    return factor;
}

function itemEntered(itemText, itemImage) {
    if ( !itemText.fontResized ) {
        if ( lastItemText != undefined )
            itemExited(lastItemText, lastItemImage);
        lastItemText = itemText;
        lastItemImage = itemImage;
        itemText.fontResized = true;
        itemText.font.pixelSize += 5;
        itemImage.opacity = 1;
    }
}

function itemExited(itemText, itemImage) {
    if ( itemText.fontResized ) {
        itemText.fontResized = false;
        itemText.font.pixelSize -= 5;
        itemImage.opacity = 0.7;
    }
}

function itemClicked(id, name) {
    gamenameText.text = name + " clicked!"
}
