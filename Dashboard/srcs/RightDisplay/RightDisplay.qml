import QtQuick 2.15

Rectangle {
    radius: width * 0.04
    color: "transparent"
    clip: true

    MusicPlayer {
        anchors.fill: parent
    }
}
