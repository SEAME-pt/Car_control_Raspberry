import QtQuick 2.15
import QtQuick.Window 2.15
import com.dashboard 1.0
import com.cardata 1.0
import "CenterDisplay"
import "LeftDisplay"
import "RightDisplay"
import "ErrorDisplay"
import "TopRow"

Window {
    visible: true
    height: Dashboard.height
    width: Dashboard.width
    title: qsTr("SimpleQt QML Window")
    color: "#131313"

    // Error state - local properties that update from signals
    property bool showError: false
    property string errorMessage: ""

    // Connect to CarData signals
    Connections {
        target: CarData
        
        function onShowErrorChanged() {
            console.log("showError changed to:", CarData.showError)
            showError = CarData.showError
        }
        
        function onErrorMessageChanged() {
            console.log("errorMessage changed to:", CarData.errorMessage)
            errorMessage = CarData.errorMessage
        }
    }

    Component.onCompleted: {
        showError = CarData.showError
        errorMessage = CarData.errorMessage
    }

    FontLoader {
        id: vwHeadFont

        source: "qrc:/fonts/VWTextRegular.otf"
    }


    // 3x3 Grid Example

    Item {
        id: gridContainer

        // Row heights
        property real topRowHeight: gridContainer.height * Dashboard.topHeight
        property real centerRowHeight: gridContainer.height * Dashboard.centerHeight
        property real bottomRowHeight: gridContainer.height * Dashboard.bottomHeight
        // Column width
        property real leftColWidth: gridContainer.width * Dashboard.leftWidth
        property real centerColWidth: gridContainer.width * Dashboard.centerWidth
        property real rightColWidth: gridContainer.width * Dashboard.rightWidth
        // Start Height
        property real topStartY: 0
        property real centerStartY: topRowHeight + (Dashboard.padding_y * gridContainer.height)
        property real bottomStartY: topRowHeight + centerRowHeight
        // Start Width
        property real leftStartX: 0
        property real centerStartX: leftColWidth + (Dashboard.padding_x * gridContainer.width)
        property real rightStartX: centerStartX + centerColWidth + (Dashboard.padding_x * gridContainer.width)

        anchors.fill: parent
        anchors.margins: Dashboard.margin

        TopRow {
            leftColWidth: gridContainer.leftColWidth
            centerColWidth: gridContainer.centerColWidth
            rightColWidth: gridContainer.rightColWidth
            topRowHeight: gridContainer.topRowHeight
            topStartY: gridContainer.topStartY
            fontFamily: vwHeadFont.name
            leftStartX: gridContainer.leftStartX
            centerStartX: gridContainer.centerStartX
            rightStartX: gridContainer.rightStartX
        }

        // Center Row - Left Column
        LeftDisplay {
            x: gridContainer.leftStartX
            y: gridContainer.centerStartY
            width: gridContainer.leftColWidth
            height: gridContainer.centerRowHeight
        }

        // Center Row - Center Column
        CenterDisplay {
            x: gridContainer.centerStartX
            y: gridContainer.centerStartY
            width: gridContainer.centerColWidth
            height: gridContainer.centerRowHeight
        }


        // Center Row - right Column
        RightDisplay {
            x: gridContainer.rightStartX
            y: gridContainer.centerStartY
            width: gridContainer.rightColWidth
            height: gridContainer.centerRowHeight
        }

    }

    ErrorOverlay {
        anchors.fill: parent
    }

}
