import QtQuick 1.1
import "ToxicWaste.js" as ToxicWaste

Rectangle {
    property int fps: 0
    Component.onCompleted: ToxicWaste.init()
    id: toxic_waste_main
    width: 800
    height: 600
    gradient: Gradient {
        GradientStop {
            position: 0
            color: "#3aa82b"
        }
        GradientStop {
            position: 0.75
            color: "#ffffff"
        }
        GradientStop {
            position: 1
            color: "#000000"
        }
    }
    ListView {
        id: gamelist_view
        scale: parent.width / 800
        height: parent.height / scale - 20
        highlightRangeMode: ListView.NoHighlightRange
        snapMode: ListView.NoSnap
        interactive: true
        keyNavigationWraps: false
        anchors.verticalCenterOffset: 0
        anchors.horizontalCenterOffset: 0
        x: parent.width / scale / 2 - width / 2
        y: 10
        width: 304
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 10
        orientation: ListView.Vertical
        flickableDirection: Flickable.AutoFlickDirection
        smooth: true
        delegate: Item {
            property string gameid: id
            id: item_delegate
            height: 64
            Item {
                Image {
                    id: gameitem_image
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    source: "images/gameitem_bg.png"
                    height: item_delegate.height
                    opacity: 0.7
                    Text {
                        id: gameitem_text
                        text: name
                        color: "black"
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.bold: true
                        font.italic: true
                        font.pixelSize: parent.height - 40
                    }
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton
                        onEntered: ToxicWaste.itemEntered(gameitem_text, gameitem_image)
                        onExited: ToxicWaste.itemExited(gameitem_text, gameitem_image)
                        onClicked: ToxicWaste.itemClicked(item_delegate.gameid, gameitem_text.text)
                        onDoubleClicked: ToxicWaste.itemDoubleClicked(item_delegate.gameid, gameitem_text.text)
                    }
                }
            }
        }
        model: ListModel {
            id: gamelist_model
        }
    }

    Text {
        id: gamename_text
        x: 10
        y: 10
        width: 200
        height: 100
        text: ""
        font.pixelSize: 10
    }

    Text {
        id: fps_text
        x: 10
        y: parent.height - 20
        color: "#ffffff"
        text: qsTr("FPS") + ": " + parent.fps.toString()
        font.bold: false
        style: Text.Normal
        font.pixelSize: 10
    }
}
