import QtQuick 2.15

Canvas {
    id: gridCanvas
    anchors.fill: parent

    property real topY: height * 0.25
    property real topWidth: width * 0.05
    property real bottomY: height
    property real bottomWidth: width
    property real centerX: width * 0.5

    layer.enabled: true
    //layer.effect: Glow {
    //    radius: 8
    //    color: "#686868"
    //    spread: 0.4
    //    samples: 17
    //}

    onPaint: {
        var ctx = getContext("2d");
        ctx.reset();

        // Draw main grid lines
        ctx.strokeStyle = "#686868";
        ctx.lineWidth = 1.5;

        // Horizontal grid lines (going into distance)
        var numHorizontal = 12;
        for (var i = 0; i < numHorizontal; i++) {
            var progress = i / numHorizontal;
            var perspectiveFactor = Math.pow(progress, 2);

            var lineY = topY + (bottomY - topY) * perspectiveFactor;

            ctx.beginPath();
            ctx.moveTo(0, lineY);
            ctx.lineTo(width, lineY);
            ctx.stroke();
        }

        // Vertical grid lines (perspective lines converging to vanishing point)
        var numVertical = 40;
        for (var j = 0; j <= numVertical; j++) {
            var lateralPos = ((j / numVertical) * 2 - 1) * 22;

            var startX = centerX + lateralPos * width * 0.5;
            var endX = centerX + lateralPos * topWidth * 0.5;

            ctx.beginPath();
            ctx.moveTo(startX, bottomY);
            ctx.lineTo(endX, topY);
            ctx.stroke();
        }
    }
}