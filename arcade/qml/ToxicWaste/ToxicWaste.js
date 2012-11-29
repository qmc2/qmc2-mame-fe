var last_item_text;
var last_item_image;

function init() {
    gamelist_model.clear();
    for (var i = 1; i < 101; i++)
        gamelist_model.append({"name": "Item " + i, "id": i});
}

function baseWidth() {
    return 800;
}

function baseHeight() {
    return 600;
}

function scaleFactor() {
    var factor = toxic_waste_main.width / baseWidth();
    return factor;
}

function itemEntered(item_text, item_image) {
    if ( !item_text.fontResized ) {
        if ( last_item_text != undefined )
            itemExited(last_item_text, last_item_image);
        last_item_text = item_text;
        last_item_image = item_image;
        item_text.fontResized = true;
        item_text.font.pixelSize += 5;
        item_image.opacity = 1;
    }
}

function itemExited(item_text, item_image) {
    if ( item_text.fontResized ) {
        item_text.fontResized = false;
        item_text.font.pixelSize -= 5;
        item_image.opacity = 0.7;
    }
}

function itemClicked(id, name) {
    gamename_text.text = name + " clicked!"
}

function itemDoubleClicked(id, name) {
    gamename_text.text = name + " double-clicked!"
}
