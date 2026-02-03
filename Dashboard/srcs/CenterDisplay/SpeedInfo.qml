import QtQuick 2.15
import com.cardata 1.0

Column {
    id: speedInfo
    anchors.horizontalCenter: parent.horizontalCenter
    spacing: centerDisplay.height * -0.05

    Text {
        text: "km/h"
        color: "#F9F9F9"
        font.pixelSize: centerDisplay.height * 0.08
        font.family: vwHeadFont.name
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Text {
        text: CarData.speed
        color: "#F9F9F9"
        font.pixelSize: centerDisplay.height * 0.32
        font.family: vwHeadFont.name
        font.bold: true
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Text {
        text: CarData.speed === 0 ? "PARK" : "DRIVE"
        color: CarData.speed === 0 ? "#FF0000" : "#00FF00"
        font.pixelSize: centerDisplay.height * 0.12
        font.family: vwHeadFont.name
        font.bold: true
        anchors.horizontalCenter: parent.horizontalCenter
    }

}
