import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    visible: true
    width: 1280
    height: 400
    title: qsTr("SimpleQt QML Window")

    Rectangle {
        anchors.fill: parent
        color: "#131313"

        Image {
            source: "qrc:/assets/Battery.svg"
            width: 400
            height: 400
            anchors.centerIn: parent
        }
    }
}
