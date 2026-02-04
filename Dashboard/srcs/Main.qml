import QtQuick 2.15
import QtQuick.Window 2.15
import com.dashboard 1.0
import com.cardata 1.0
import "CenterDisplay"
import "LeftDisplay"
import "ErrorDisplay"

Window {
    visible: true
    title: qsTr("SimpleQt QML Window")
	flags: Qt.FramelessWindowHint
	visibility: Window.FullScreen
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
        console.log("Window created and visible")
        console.log("Initial showError:", CarData.showError)
        console.log("Initial errorMessage:", CarData.errorMessage)
        showError = CarData.showError
        errorMessage = CarData.errorMessage
        currentTime = new Date().toLocaleTimeString(Qt.locale(), "hh:mm")
    }

    FontLoader {
        id: vwHeadFont

        source: "qrc:/fonts/VWTextRegular.otf"
    }

    // Property to hold current time
    property string currentTime: ""

    // Timer to update time every second
    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: currentTime = new Date().toLocaleTimeString(Qt.locale(), "hh:mm")
    }

    // 3x3 Grid Example

    Item {
        id: gridContainer

        // Row heights
        property real topRowHeight: gridContainer.height * 0.075
        property real centerRowHeight: gridContainer.height * 0.825
        property real bottomRowHeight: gridContainer.height * 0.10
        // Column width
        property real leftColWidth: gridContainer.width * 0.30
        property real centerColWidth: gridContainer.width * 0.40
        property real rightColWidth: gridContainer.width * 0.30

        anchors.fill: parent
        anchors.margins: Dashboard.margin

		// Top Row - Left Column
		Rectangle {
		    x: 0
		    y: 0
		    width: gridContainer.leftColWidth
		    height: gridContainer.topRowHeight
			color: "#131313"
		    Text {
		        text: currentTime
		        color: "#F9F9F9"
		        font.pixelSize: Math.min(parent.width * 0.3, parent.height * 0.8)  // Scale font size relative to cell size
		        font.family: vwHeadFont.name
		        font.bold: true
		        anchors.centerIn: parent  // Center both horizontally and vertically
		    }
		}
		// Top Row - Center Column
		Rectangle {
			x: gridContainer.leftColWidth
            y: 0
            width: gridContainer.centerColWidth
            height: gridContainer.topRowHeight
            color: "#00FF00"
		}
		// Top Row - Center Column
		Rectangle {
			x: gridContainer.leftColWidth + gridContainer.centerColWidth
            y: 0
            width: gridContainer.rightColWidth
            height: gridContainer.topRowHeight
            color: "#0000FF"
		}

        // Center Row - Left Column
        LeftDisplay {
            x: 0
            y: gridContainer.topRowHeight
            width: gridContainer.leftColWidth
            height: gridContainer.centerRowHeight
        }

        // Center Row - Center Column
        CenterDisplay {
            x: gridContainer.leftColWidth
            y: gridContainer.topRowHeight
            width: gridContainer.centerColWidth
            height: gridContainer.centerRowHeight
        }


        // Center Row - Left Column
        Rectangle {
            x: gridContainer.leftColWidth + gridContainer.centerColWidth
            y: gridContainer.topRowHeight
            width: gridContainer.rightColWidth
            height: gridContainer.centerRowHeight
            color: "#FFE66D"
        }

    }

    ErrorOverlay {
        anchors.fill: parent
    }

}
