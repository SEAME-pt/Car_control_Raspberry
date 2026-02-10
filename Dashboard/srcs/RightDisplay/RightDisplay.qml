import QtQuick 2.15

Rectangle {
    color: "transparent"
    clip: true

    // which panel to show: "music" or "settings"
    property string panelType: "music"

    Component {
        id: musicComp
        MusicPlayer { anchors.fill: parent }
    }

    Component {
        id: settingsComp
        Settings { anchors.fill: parent }
    }

    Loader {
        id: rightLoader
        anchors.fill: parent
        sourceComponent: panelType === "music" ? musicComp : settingsComp
        
        Component.onCompleted: {
            console.log("RightDisplay Loader initialized with panelType:", panelType)
        }
    }
    
    onPanelTypeChanged: {
        console.log("RightDisplay panelType changed to:", panelType)
    }
}
