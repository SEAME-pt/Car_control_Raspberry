import QtQuick

Canvas {
    id: roadCanvas
    anchors.fill: parent
    
    property real topY: height * 0.25
    property real topWidth: width * 0.05
    property real bottomY: height
    property real bottomWidth: width * 0.7
    property real centerX: width * 0.5
    
    onPaint: {
        var ctx = getContext("2d");
        ctx.reset();
        
        // Draw road with edge glow
        var leftEdgeX = centerX - bottomWidth * 0.5;
        var rightEdgeX = centerX + bottomWidth * 0.5;
        
        // Road fill
        ctx.fillStyle = "#404040";
        ctx.beginPath();
        ctx.moveTo(centerX - topWidth * 0.5, topY);
        ctx.lineTo(centerX + topWidth * 0.5, topY);
        ctx.lineTo(rightEdgeX, bottomY);
        ctx.lineTo(leftEdgeX, bottomY);
        ctx.closePath();
        ctx.fill();
        
        // Left edge glow
        var leftGradient = ctx.createLinearGradient(leftEdgeX - width * 0.05, bottomY, leftEdgeX, bottomY);
        leftGradient.addColorStop(0, "rgba(200, 200, 200, 0)");
        leftGradient.addColorStop(1, "rgba(200, 200, 200, 0.3)");
        ctx.strokeStyle = leftGradient;
        ctx.lineWidth = 3;
        ctx.beginPath();
        ctx.moveTo(centerX - topWidth * 0.5, topY);
        ctx.lineTo(leftEdgeX, bottomY);
        ctx.stroke();
        
        // Right edge glow
        var rightGradient = ctx.createLinearGradient(rightEdgeX, bottomY, rightEdgeX + width * 0.05, bottomY);
        rightGradient.addColorStop(0, "rgba(200, 200, 200, 0.3)");
        rightGradient.addColorStop(1, "rgba(200, 200, 200, 0)");
        ctx.strokeStyle = rightGradient;
        ctx.lineWidth = 3;
        ctx.beginPath();
        ctx.moveTo(centerX + topWidth * 0.5, topY);
        ctx.lineTo(rightEdgeX, bottomY);
        ctx.stroke();
    }
}
