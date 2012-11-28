function init() {
    gamelist_model.clear();
    for (var i = 1; i < 101; i++)
        gamelist_model.append({"name": "Item " + i, "id": i});
}

function itemEntered(item_text, item_image) {
    item_text.font.pixelSize += 5;
    item_image.opacity = 1;
}

function itemExited(item_text, item_image) {
    item_text.font.pixelSize -= 5;
    item_image.opacity = 0.7;
}

function itemClicked(id, name) {
    gamename_text.text = name + " clicked!"
}

function itemDoubleClicked(id, name) {
    gamename_text.text = name + " double-clicked!"
}
