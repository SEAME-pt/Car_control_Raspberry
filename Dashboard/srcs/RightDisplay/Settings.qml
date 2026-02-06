import QtQuick 2.15
import QtQuick.Controls 2.15
import com.cardata 1.0

Rectangle {
    color: "#1a1a1a"
    radius: parent.radius

    // Use persisted speed unit from CarData singleton

    Column {
        anchors.fill: parent
        anchors.margins: parent.width * 0.05
        spacing: parent.height * 0.03

        Text {
            text: "Settings"
            color: "#F9F9F9"
            font.pixelSize: parent.width * 0.08
            font.bold: true
        }

        Rectangle {
            width: parent.width
            height: 1
            color: "#333333"
        }

        Item {
            width: parent.width
            height: parent.height * 0.1

            Text {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                text: "Speed Unit"
                color: "#F9F9F9"
                font.pixelSize: parent.width * 0.06
            }

            ComboBox {
                id: distanceCombo
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                model: ["km", "miles"]
                width: 100
                currentIndex: Math.max(0, model.indexOf(CarData.distanceUnit))
                onCurrentIndexChanged: {
                    CarData.distanceUnit = model[currentIndex]
                }
            }
        }
        Item {
            width: parent.width
            height: parent.height * 0.1

            Text {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                text: "Temperature Unit"
                color: "#F9F9F9"
                font.pixelSize: parent.width * 0.06
            }

            ComboBox {
                id: temperatureCombo
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                model: ["Celsius", "Fahrenheit"]
                width: 100
                currentIndex: Math.max(0, model.indexOf(CarData.temperatureUnit))
                onCurrentIndexChanged: {
                    CarData.temperatureUnit = model[currentIndex]
                }
            }
        }

        Rectangle {
            width: parent.width
            height: 1
            color: "#333333"
        }

        Item {
            width: parent.width
            height: parent.height * 0.1

            Text {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                text: "Cruise Control"
                color: "#F9F9F9"
                font.pixelSize: parent.width * 0.06
            }

            Switch {
                id: cruiseControlSwitch
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                
                onToggled: {
                    // Send cruise control state to server
                    if (checked) {
                        CarData.sendSpeed(CarData.speed)  // Lock current speed
                    } else {
                    }
                    // You can add a custom field for cruise control
                    // CarData.sendCruiseControl(checked)
                }
            }
        }
    }
}
