import QtQuick 2.15
import com.cardata 1.0

Rectangle {
    id: motorState

    anchors.horizontalCenter: parent.horizontalCenter
    width: parent.width
    height: parent.height * 0.015
    color: "#696969"
    radius: parent.width * 0.005
    clip: true

    // Center line indicator
    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        width: 2
        height: parent.height * 1.5
        color: "#F9F9F9"
        z: 10
    }

    // Acceleration indicator (center to right, green)
    Rectangle {
        anchors.left: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        width: CarData.motorPower > 0 ? (parent.width * 0.5) * (CarData.motorPower / 100.0) : 0
        height: parent.height
        color: "#00FF00"
        radius: parent.radius
        visible: CarData.motorPower > 0
        
        Behavior on width {
            NumberAnimation { duration: 200 }
        }
    }

    // Braking indicator (center to left, red)
    Rectangle {
        anchors.right: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        width: CarData.motorPower < 0 ? (parent.width * 0.5) * (Math.abs(CarData.motorPower) / 100.0) : 0
        height: parent.height
        color: "#FF0000"
        radius: parent.radius
        visible: CarData.motorPower < 0
        
        Behavior on width {
            NumberAnimation { duration: 200 }
        }
    }

}
