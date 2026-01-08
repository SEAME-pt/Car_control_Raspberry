
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Item {
    id: gauge
    property real value: 0
    property real minimumValue: 0
    property real maximumValue: 240

    width: 350
    height: 350

    // Equivalente ao speedColorProvider
    function speedColorProvider(v){
        if (v < 60)
            return "#32D74B"
        else if (v >= 60 && v < 150)
            return "yellow"
        else
            return "red"
    }

    // Converte valor para ângulo
    function valueToAngle(v) {
        const minAngle = -144
        const maxAngle = 144
        return minAngle + (v - minimumValue) * (maxAngle - minAngle) / (maximumValue - minimumValue)
    }

    // Fundo
    Rectangle {
        anchors.fill: parent
        color: "#1E1E1E"
        radius: width
        opacity: 0.5

        Image {
            anchors.fill: parent
            source: "qrc:/assets/background.svg"
            asynchronous: true
            sourceSize.width: width
        }

        // ARCO PRINCIPAL DESENHADO NO CANVAS
        Canvas {
            id: arcCanvas
            anchors.fill: parent

            onPaint: {
                var ctx = getContext("2d");
                ctx.reset();

                var outerRadius = width / 2;
                var startAngle = valueToAngle(gauge.minimumValue) - 90;
                var endAngle = valueToAngle(gauge.value) - 90;

                var gradientColor = speedColorProvider(gauge.value);

                ctx.beginPath();
                ctx.lineWidth = outerRadius * 0.225;
                ctx.strokeStyle = gradientColor;

                ctx.arc(
                    outerRadius,
                    outerRadius,
                    outerRadius - ctx.lineWidth / 2,
                    startAngle * Math.PI / 180,
                    endAngle * Math.PI / 180
                );
                ctx.stroke();
            }

            onWidthChanged: requestPaint()
            onHeightChanged: requestPaint()
            onVisibleChanged: requestPaint()
        }
    }

    // PONTEIRO
    Item {
        id: needleHolder
        anchors.centerIn: parent
        width: parent.width
        height: parent.height

        rotation: valueToAngle(gauge.value)

        Image {
            id: needle
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            source: "qrc:/assets/needle.svg"
            height: parent.height * 0.27
            width: height * 0.1
            antialiasing: true
            asynchronous: true
            transform: Rotation { origin.x: width/2; origin.y: height }
        }

        // Substitui Glow (Qt 5) → MultiEffect (Qt 6)
        MultiEffect {
            anchors.fill: needle
            source: needle
            shadowEnabled: true
            shadowColor: "white"
            shadowOpacity: 0.8
            shadowBlur: 0.5
        }
    }

    // TEXTO CENTRAL
    ColumnLayout {
        anchors.centerIn: parent

        Label {
            text: gauge.value.toFixed(0)
            font.pixelSize: 85
            font.family: "Inter"
            color: "#01E6DE"
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: "KMH"
            font.pixelSize: 46
            font.family: "Inter"
            color: "#01E6DE"
            Layout.alignment: Qt.AlignHCenter
        }
    }

    // TICKS (marcadores)
    Repeater {
        model: 25  // 0 a 240 → marcações de 10 km/h

        delegate: Item {
            width: gauge.width
            height: gauge.height
            anchors.centerIn: parent

            property real angle: -144 + (index * (288 / 24))
            rotation: angle

            Image {
                source: "qrc:/assets/tickmark.svg"
                width: gauge.width * 0.018
                height: gauge.width * 0.15
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                antialiasing: true
            }

            Text {
                text: index * 10
                color: index * 10 <= gauge.value ? "white" : "#777776"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: gauge.height * 0.20
                font.pixelSize: gauge.width * 0.05
                rotation: -angle
            }
        }
    }

}

