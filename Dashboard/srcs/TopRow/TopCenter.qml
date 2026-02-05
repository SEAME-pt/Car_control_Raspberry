import QtQuick

Rectangle {
    color: "#131313"

    FontLoader {
        id: vwHeadFont

        source: "qrc:/fonts/VWTextRegular.otf"
    }

    property string currentTime: ""
        // Timer to update time every second
        Timer {
            interval: 1000
            running: true
            repeat: true
            onTriggered: currentTime = new Date().toLocaleTimeString(Qt.locale(), "hh:mm")
        }

        Text {
            text: currentTime
            color: "#F9F9F9"
            font.pixelSize: Math.min(parent.width * 0.8, parent.height * 0.8)
            font.family: vwHeadFont.name
            font.bold: true
            anchors.centerIn: parent
        }

    }