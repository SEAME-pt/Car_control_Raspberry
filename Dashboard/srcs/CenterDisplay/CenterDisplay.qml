import QtQuick 2.15

Rectangle {
    id: centerDisplay

    color: "transparent"

    FontLoader {
        id: vwHeadFont

        source: "qrc:/fonts/VWTextRegular.otf"
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        width: centerDisplay.width * 0.75
        spacing: centerDisplay.height * 0.04

        SpeedLimit {
            anchors.horizontalCenter: parent.horizontalCenter
            speedLimit: 120
        }

        SpeedInfo {
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // Battery information
        BatteryInfo {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.horizontalCenterOffset: 2
        }

        // Motor state indicator bar
        MotorState {
            width: parent.width
            anchors.horizontalCenter: parent.horizontalCenter
        }

    }

}
