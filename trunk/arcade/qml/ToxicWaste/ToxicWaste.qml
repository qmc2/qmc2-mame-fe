import QtQuick 1.1
import "ToxicWaste.js" as ToxicWaste

Rectangle {
    property int fps: 0
    property bool fpsVisible: true
    Component.onCompleted: ToxicWaste.init()
    id: toxic_waste_main
    width: ToxicWaste.baseWidth()
    height: ToxicWaste.baseHeight()
    gradient: Gradient {
        GradientStop {
            position: 0.0
            color: "#3aa82b"
        }
        GradientStop {
            position: 0.75
            color: "#ffffff"
        }
        GradientStop {
            position: 1.0
            color: "#000000"
        }
    }  
    ListView {
        id: gamelist_view
        scale: ToxicWaste.scaleFactor()
        height: parent.height / scale - 20
        flickDeceleration: 1500
        maximumFlickVelocity: 3000
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
                        property bool fontResized: false
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
        anchors.top: parent.top
        anchors.topMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 5
        transformOrigin: Text.TopLeft
        color: "black"
        text: ""
        font.pixelSize: 10
        scale: ToxicWaste.scaleFactor()
    }
    Text {
        id: fps_text
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 5
        transformOrigin: Text.BottomLeft
        color: "lightgrey"
        text: qsTr("FPS") + ": " + parent.fps.toString()
        font.pixelSize: 10
        scale: ToxicWaste.scaleFactor()
        visible: parent.fpsVisible
    }
}
