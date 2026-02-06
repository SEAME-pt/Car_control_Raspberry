import QtQuick 2.15
import com.cardata 1.0

// Error Overlay
Rectangle {
    id: errorOverlay
    anchors.fill: parent
    color: "#CC000000" // Semi-transparent black
    visible: CarData.showError
    z: 1000 // High z-index to appear on top

    MouseArea {
        // Prevents clicks from passing through
        anchors.fill: parent
    }

    Rectangle {
        anchors.centerIn: parent
        width: parent.width * 0.7
        height: parent.height * 0.7
        color: "#1F1F1F"
        border.color: "#FF3B3B"
        border.width: parent.width * 0.002
        radius: parent.width * 0.01

        Column {
            anchors.centerIn: parent
            spacing: parent.height * 0.05

            Text {
                text: "⚠ ERROR"
                font.family: vwHeadFont.name
                font.pixelSize: 32
                font.bold: true
                color: "#FF3B3B"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                text: CarData.errorMessage
                font.family: vwHeadFont.name
                font.pixelSize: 20
                color: "#FFFFFF"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // Optional: Add dismiss button
            Rectangle {
                width: parent.width * 0.6
                height: parent.height * 0.4
                color: "#FF3B3B"
                radius: 5
                anchors.horizontalCenter: parent.horizontalCenter
                Text {
                    text: "Dismiss"
                    font.family: vwHeadFont.name
                    font.pixelSize: 16
                    color: "#FFFFFF"
                    anchors.centerIn: parent
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: CarData.dismissError()
                }
            }
        }
    }
}
