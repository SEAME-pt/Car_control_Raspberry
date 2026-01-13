import QtQuick 6.0
import CustomControls 1.0    // se for um módulo próprio, mantém (verificar compatibilidade)
import QtQuick.Window 6.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 6.0
import "./"
ApplicationWindow {
    width: 1920
    height: 960
    visible: true
    title: qsTr("Car DashBoard")
    color: "#1E1E1E"
    visibility: "FullScreen"
    property int nextSpeed: 60
    // currentSpeed is updated from C++ (main.cpp) via setProperty
    property int currentSpeed: 60
    property int speedLimit: 50
    property int currBatery: 75
    property double avgSpeed: 60
    property double fuelUsage: 35
    property double distance: 188.75
    property double temp: 22.4

    function generateRandom(maxLimit = 70){
        let rand = Math.random() * maxLimit;
        rand = Math.floor(rand);
        return rand;
    }

    function speedColor(value){
        if(value < speedLimit ){
            return "red"
        } else if(value > speedLimit && value < speedLimit + 20){
            return "red"
        }else{
            return "Red"
        }
    }

    Image {
        id: dashboard
        width: parent.width
        height: parent.height
        anchors.centerIn: parent
        source: "qrc:/assets/Dashboard.svg"

        /*
          Top Bar Of Screen
        */

        Image {
            id: topBar
            width: 1357
            source: "qrc:/assets/Vector 70.svg"

            anchors{
                top: parent.top
                topMargin: 26.50
                horizontalCenter: parent.horizontalCenter
            }

            Label{
                id: currentTime
                text: Qt.formatDateTime(new Date(), "hh:mm")
                font.pixelSize: 32
                font.family: "Inter"
                font.bold: Font.DemiBold
                color: "#FFFFFF"
                anchors.top: parent.top
                anchors.topMargin: 25
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Label{
                id: currentDate
                text: Qt.formatDateTime(new Date(), "dd/MM/yyyy")
                font.pixelSize: 32
                font.family: "Inter"
                font.bold: Font.DemiBold
                color: "#FFFFFF"
                anchors.right: parent.right
                anchors.rightMargin: 230
                anchors.top: parent.top
                anchors.topMargin: 25
            }
        }



        /*
          Speed Label
        */

        Gauge {
            id: speedLabel
            width: 450
            height: 450
            property bool accelerating
            // Bind the gauge value to the currentSpeed property so C++ can update it
            value: currentSpeed
            maximumValue: 300

            anchors.top: parent.top
            anchors.topMargin:Math.floor(parent.height * 0.25)
            anchors.horizontalCenter: parent.horizontalCenter

            Behavior on value { NumberAnimation { duration: 1000 }}

        }



        /*
          Speed Limit Label
        */

        Rectangle{
            id:speedLimit
            width: 130
            height: 130
            radius: height/2
            color: "#D9D9D9"
            border.color: "Red"
            border.width: 10

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 50

            Label{
                id:maxSpeedlabel
                text: "120"
                //getRandomInt(100, speedLabel.maximumValue).toFixed(0)
                font.pixelSize: 45
                font.family: "Inter"
                font.bold: Font.Bold
                color: "#311a1a"
                anchors.centerIn: parent

                function getRandomInt(min, max) {
                    return Math.floor(Math.random() * (max - min + 1)) + min;
                }
            }
        }

        Image {
            anchors{
                bottom: car.top
                bottomMargin: 30
                horizontalCenter:car.horizontalCenter
            }
            source: "qrc:/assets/Model 3.png"
        }


        Image {
            id:car
            anchors{
                bottom: speedLimit.top
                bottomMargin: 30
                horizontalCenter:speedLimit.horizontalCenter
            }
            source: "qrc:/assets/Car.svg"
        }

        /*
          Left Road
        */

        Image {
            id: leftRoad
            width: 127
            height: 397
            anchors{
                left: speedLimit.left
                leftMargin: 100
                bottom: parent.bottom
                bottomMargin: 26.50
            }

            source: "qrc:/assets/Vector 2.svg"
        }

        RowLayout{
            spacing: 20

            anchors{
                left: parent.left
                leftMargin: 250
                bottom: parent.bottom
                bottomMargin: 26.50 + 65
            }

            RowLayout{
                spacing: 3
                Label{
                    text: "22.4"
                    font.pixelSize: 32
                    font.family: "Inter"
                    font.bold: Font.Normal
                    font.capitalization: Font.AllUppercase
                    color: "#FFFFFF"
                }

                Label{
                    text: "°C"
                    font.pixelSize: 32
                    font.family: "Inter"
                    font.bold: Font.Normal
                    font.capitalization: Font.AllUppercase
                    opacity: 0.2
                    color: "#FFFFFF"
                }
            }

            RowLayout{
                spacing: 1
                Layout.topMargin: 10
                Rectangle{
                    width: 20
                    height: 15
                    color: speedLabel.value.toFixed(0) > 42.85 ? speedLabel.speedColor : "#01E6DC"
                }
                Rectangle{
                    width: 20
                    height: 15
                    color: speedLabel.value.toFixed(0) > 85.7 ? speedLabel.speedColor : "#01E6DC"
                }
                Rectangle{
                    width: 20
                    height: 15
                    color: speedLabel.value.toFixed(0) > 128.55 ? speedLabel.speedColor : "#01E6DC"
                }
                Rectangle{
                    width: 20
                    height: 15
                    color: speedLabel.value.toFixed(0) > 171.4 ? speedLabel.speedColor : "#01E6DC"
                }
                Rectangle{
                    width: 20
                    height: 15
                    color: speedLabel.value.toFixed(0) > 214.25 ? speedLabel.speedColor : "#01E6DC"
                }
                Rectangle{
                    width: 20
                    height: 15
                    color: speedLabel.value.toFixed(0) > 257.1 ? speedLabel.speedColor : "#01E6DC"
                }
                Rectangle{
                    width: 20
                    height: 15
                    color: speedLabel.value.toFixed(0) > 299.95 ? speedLabel.speedColor : "#01E6DC"
                }
            }

            Label{
                text: speedLabel.value.toFixed(0) + " KMH "
                font.pixelSize: 32
                font.family: "Inter"
                font.bold: Font.Normal
                font.capitalization: Font.AllUppercase
                color: "#FFFFFF"
            }
        }

        /*
          Right Road
        */

        Image {
            id: rightRoad
            width: 127
            height: 397
            anchors{
                right: speedLimit.right
                rightMargin: 100
                bottom: parent.bottom
                bottomMargin: 26.50
            }

            source: "qrc:/assets/Vector 1.svg"
        }

        /*
          Right Side gear
        */

        RowLayout{
            spacing: 20
            anchors{
                right: parent.right
                rightMargin: 350
                bottom: parent.bottom
                bottomMargin: 26.50 + 65
            }

            Label{
                text: "Ready"
                font.pixelSize: 32
                font.family: "Inter"
                font.bold: Font.Normal
                font.capitalization: Font.AllUppercase
                color: "#32D74B"
            }

            Label{
                text: "P"
                font.pixelSize: 32
                font.family: "Inter"
                font.bold: Font.Normal
                font.capitalization: Font.AllUppercase
                color: "#FFFFFF"
            }

            Label{
                text: "R"
                font.pixelSize: 32
                font.family: "Inter"
                font.bold: Font.Normal
                font.capitalization: Font.AllUppercase
                opacity: 0.2
                color: "#FFFFFF"
            }
            Label{
                text: "N"
                font.pixelSize: 32
                font.family: "Inter"
                font.bold: Font.Normal
                font.capitalization: Font.AllUppercase
                opacity: 0.2
                color: "#FFFFFF"
            }
            Label{
                text: "D"
                font.pixelSize: 32
                font.family: "Inter"
                font.bold: Font.Normal
                font.capitalization: Font.AllUppercase
                opacity: 0.2
                color: "#FFFFFF"
            }
        }

        // Progress Bar
        RadialBar {
            id:radialBar
            anchors{
                verticalCenter: parent.verticalCenter
                left: parent.left
                leftMargin: parent.width / 6
            }

            width: 338
            height: 338
            penStyle: Qt.RoundCap
            dialType: RadialBar.NoDial
            progressColor: "#01E4E0"
            backgroundColor: "transparent"
            dialWidth: 17
            startAngle: 270
            spanAngle: 3.6 * value
            minValue: 0
            maxValue: 100
            value: currBatery
            textFont {
                family: "inter"
                italic: false
                bold: Font.Medium
                pixelSize: 60
            }
            showText: false
            suffixText: ""
            textColor: "#FFFFFF"

            property bool accelerating
            Behavior on value { NumberAnimation { duration: 1000 }}

            ColumnLayout{
                anchors.centerIn: parent
                Label{
                    text: radialBar.value.toFixed(0) + "%"
                    font.pixelSize: 65
                    font.family: "Inter"
                    font.bold: Font.Normal
                    color: "#FFFFFF"
                    Layout.alignment: Qt.AlignHCenter
                }

                Label{
                    text: "Battery charge"
                    font.pixelSize: 28
                    font.family: "Inter"
                    font.bold: Font.Normal
                    opacity: 0.8
                    color: "#FFFFFF"
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        ColumnLayout{
            spacing: 40

            anchors{
                verticalCenter: parent.verticalCenter
                right: parent.right
                rightMargin: parent.width / 6
            }

            RowLayout{
                spacing: 30
                Image {
                    width: 72
                    height: 50
                    source: "qrc:/assets/road.svg"
                }

                ColumnLayout{
                    Label{
                        text: distance + " KM"
                        font.pixelSize: 30
                        font.family: "Inter"
                        font.bold: Font.Normal
                        opacity: 0.8
                        color: "#FFFFFF"
                    }
                    Label{
                        text: "Distance"
                        font.pixelSize: 20
                        font.family: "Inter"
                        font.bold: Font.Normal
                        opacity: 0.8
                        color: "#FFFFFF"
                    }
                }
            }
            RowLayout{
                spacing: 30
                Image {
                    width: 72
                    height: 78
                    source: "qrc:/assets/fuel.svg"
                }

                ColumnLayout{
                    Label{
                        text: fuelUsage + " kwh/km"
                        font.pixelSize: 30
                        font.family: "Inter"
                        font.bold: Font.Normal
                        opacity: 0.8
                        color: "#FFFFFF"
                    }
                    Label{
                        text: "Avg. Fuel Usage"
                        font.pixelSize: 20
                        font.family: "Inter"
                        font.bold: Font.Normal
                        opacity: 0.8
                        color: "#FFFFFF"
                    }
                }
            }
            RowLayout{
                spacing: 30
                Image {
                    width: 72
                    height: 72
                    source: "qrc:/assets/speedometer.svg"
                }

                ColumnLayout{
                    Label{
                        text: avgSpeed + " kmh"
                        font.pixelSize: 30
                        font.family: "Inter"
                        font.bold: Font.Normal
                        opacity: 0.8
                        color: "#FFFFFF"
                    }
                    Label{
                        text: "Avg. Speed"
                        font.pixelSize: 20
                        font.family: "Inter"
                        font.bold: Font.Normal
                        opacity: 0.8
                        color: "#FFFFFF"
                    }
                }
            }
        }
    }
}
