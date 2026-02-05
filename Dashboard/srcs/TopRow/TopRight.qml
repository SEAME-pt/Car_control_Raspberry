import QtQuick
import com.cardata 1.0

Rectangle {
    color: "#131313"

    FontLoader {
        id: vwHeadFont

        source: "qrc:/fonts/VWTextRegular.otf"
    }

    Text {
        text: CarData.temperature.toFixed(1) + "°" + (CarData.temperatureUnit === "Celsius" ? "C" : "F")
        color: "#F9F9F9"
        font.pixelSize: Math.min(parent.width * 0.8, parent.height * 0.8)
        font.family: vwHeadFont.name
        font.bold: true
        anchors.centerIn: parent
    }
}