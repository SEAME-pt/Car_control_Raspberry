import QtQuick 2.15

Rectangle {
    id: bottomCell
    color: "#131313"

    // Properties passed from parent
    property real bottomStartY: 0
    property real bottomRowHeight: 0
    property real bottomRowWidth: 0
    property var rightDisplay: null
    property var fontFamily: ""

    x: 0
    y: bottomStartY
    width: bottomRowWidth
    height: bottomRowHeight

    // Settings button with SVG
    Rectangle {
        id: settingsBtn
        width: 48
        height: 48
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        radius: 6
        color: "transparent"

        Image {
            id: settingsIcons
            anchors.centerIn: parent
            source: "qrc:/assets/settings.svg"
            fillMode: Image.PreserveAspectFit
            width: 28
            height: 28
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: settingsBtn.color = "#222222"
            onExited: settingsBtn.color = "transparent"
            onClicked: {
                console.log("Settings button clicked")
                if (rightDisplay !== null && typeof rightDisplay !== 'undefined') {
                    rightDisplay.panelType = "settings"
                } else {
                    console.log("rightDisplay not available")
                }
            }
        }
    }

    // Music button with SVG
    Rectangle {
        id: musicBtn
        width: 48
        height: 48
        anchors.left: settingsBtn.right
        anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        radius: 6
        color: "transparent"

        Image {
            id: musicIcon
            anchors.centerIn: parent
            source: "qrc:/assets/MusicPlayer.svg"
            fillMode: Image.PreserveAspectFit
            width: 28
            height: 28
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: musicBtn.color = "#222222"
            onExited: musicBtn.color = "transparent"
            onClicked: {
                console.log("Music button clicked")
                if (rightDisplay !== null && typeof rightDisplay !== 'undefined') {
                    rightDisplay.panelType = "music"
                } else {
                    console.log("rightDisplay not available")
                }
            }
        }
    }
}
