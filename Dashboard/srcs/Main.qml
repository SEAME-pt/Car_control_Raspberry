import QtQuick 2.15
import QtQuick.Window 2.15
import com.dashboard 1.0
import com.cardata 1.0
import "CenterDisplay"
import "LeftDisplay"
import "ErrorDisplay"

Window {
    visible: true
    width: Dashboard.width
    height: Dashboard.height
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
        console.log("Initial showError:", CarData.showError)
        console.log("Initial errorMessage:", CarData.errorMessage)
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
        property real topRowHeight: gridContainer.height * 0.15
        property real centerRowHeight: gridContainer.height * 0.7
        property real bottomRowHeight: gridContainer.height * 0.15
        // Column width
        property real leftColWidth: gridContainer.width * 0.30
        property real centerColWidth: gridContainer.width * 0.40
        property real rightColWidth: gridContainer.width * 0.30

        anchors.fill: parent
        anchors.margins: Dashboard.margin


        // Center column - spans full height
        CenterDisplay {
            x: gridContainer.leftColWidth
            y: 0
            width: gridContainer.centerColWidth
            height: gridContainer.height
        }

        // Left Column - spans full height
        LeftDisplay {
            x: 0
            y: 0
            width: gridContainer.leftColWidth
            height: gridContainer.height
        }

        // Right Column - spans full height
        Rectangle {
            x: gridContainer.leftColWidth + gridContainer.centerColWidth
            y: 0
            width: gridContainer.rightColWidth
            height: gridContainer.height
            color: "#FFE66D"
        }

    }

    ErrorOverlay {
        anchors.fill: parent
    }

}
