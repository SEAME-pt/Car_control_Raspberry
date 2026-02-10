import QtQuick 2.15
import com.cardata 1.0

Item {
    property int speedLimit: CarData.speedLimit

    width: parent.width * 0.35
    height: width

    // White background circle
    Rectangle {
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        radius: width / 2
        color: "white"

        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.9
            height: parent.height * 0.9
            radius: width / 2
            border.color: "#d60000"
            border.width: parent.width * 0.09

            Text {
                anchors.centerIn: parent
                text: speedLimit
                color: "black"
                font.pixelSize: parent.width * 0.4
                font.bold: true
            }

        }

    }

}
