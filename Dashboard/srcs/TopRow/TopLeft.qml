import QtQuick 2.15
import com.cardata 1.0

Rectangle {
    color: "#131313"

    FontLoader {
        id: vwHeadFont

        source: "qrc:/fonts/VWTextRegular.otf"
    }

    Text {
        text: CarData.totalDistance.toFixed(1) + " " + (CarData.distanceUnit === "miles" ? "miles" : "km")
        color: "#F9F9F9"
        font.pixelSize: Math.min(parent.width * 0.8, parent.height * 0.8)
        font.family: vwHeadFont.name
        font.bold: true
        anchors.centerIn: parent
    }
}