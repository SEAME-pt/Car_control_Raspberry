import QtQuick 2.15
import com.cardata 1.0

Row {
    id: batteryInfo

    property real baseFontSize: parent.height * 0.08

    spacing: parent.width * 0.02

    FontLoader {
        id: vwHeadFont

        source: "qrc:/fonts/VWTextRegular.otf"
    }

    Row {
        spacing: batteryInfo.width * 0.005
        anchors.verticalCenter: parent.verticalCenter

        Text {
            text: CarData.batteryLevel
            color: "white"
            font.pixelSize: batteryInfo.baseFontSize
            font.family: vwHeadFont.name
            font.bold: true
            renderType: Text.NativeRendering
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            text: "%"
            color: "white"
            font.pixelSize: batteryInfo.baseFontSize
            font.family: vwHeadFont.name
            renderType: Text.NativeRendering
            verticalAlignment: Text.AlignVCenter
        }

    }

    Item {
        property real batteryPercentage: CarData.batteryLevel / 100.0

        width: parent.width * 0.25
        height: width * 0.33
        anchors.verticalCenter: parent.verticalCenter

        Image {
            id: batteryOutline

            source: "qrc:/assets/battery.svg"
            anchors.fill: parent
        }

        Rectangle {
            id: batteryFill

            anchors.left: parent.left
            anchors.leftMargin: parent.width * 0.08
            anchors.verticalCenter: parent.verticalCenter
            width: (parent.width - parent.width * 0.24) * parent.batteryPercentage
            height: parent.height - parent.height * 0.36
            color: parent.batteryPercentage > 0.2 ? "#F9F9F9" : "#F44336"
            radius: parent.width * 0.03
        }
    }
    Row {
        spacing: batteryInfo.width * 0.005
        anchors.verticalCenter: parent.verticalCenter

        Text {
            text: CarData.batteryRange
            color: "white"
            font.pixelSize: batteryInfo.baseFontSize
            font.bold: true
            font.family: vwHeadFont.name
            renderType: Text.NativeRendering
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            text: CarData.distanceUnit
            color: "white"
            font.pixelSize: batteryInfo.baseFontSize
            font.family: vwHeadFont.name
            renderType: Text.NativeRendering
            verticalAlignment: Text.AlignVCenter
        }

    }

}
