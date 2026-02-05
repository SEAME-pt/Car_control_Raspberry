import QtQuick

Rectangle {
    id: leftDisplay
    color: "#1a1a1a"
    clip: true
    radius: width * 0.04

    Item {
        id: roadContainer
        height: parent.height * 0.875
        width: parent.width
        anchors.horizontalCenter: parent.horizontalCenter

        // Grid canvas (behind)
        GridCanvas {
            id: gridCanvas
        }
        // Road canvas (on top)
        RoadCanvas {
            id: roadCanvas
        }
        // Car SVG on the road
        Image {
            id: carImage
            source: "qrc:/assets/car.svg"
            width: parent.width * 0.75
            height: width * 0.6
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: parent.height * 0.01
            fillMode: Image.PreserveAspectFit
            smooth: true
        }
        Rectangle {
            width: parent.width
            height: 2
            color: "#6a6a6a"
            y: parent.height
        }
    }
}
