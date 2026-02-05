import QtQuick

Item {
    id: topRow

    property real leftColWidth
    property real centerColWidth
    property real rightColWidth
    property real topRowHeight
    property real topStartY
    property real leftStartX
    property real centerStartX
    property real rightStartX
    property string fontFamily


    TopLeft {
        x: leftStartX
        y: topStartY
        width: leftColWidth
        height: topRowHeight
    }

    TopCenter {
        x: centerStartX
        y: topStartY
        width: centerColWidth
        height: topRowHeight
    }

    TopRight {
        x: rightStartX
        y: topStartY
        width: rightColWidth
        height: topRowHeight
    }
}
