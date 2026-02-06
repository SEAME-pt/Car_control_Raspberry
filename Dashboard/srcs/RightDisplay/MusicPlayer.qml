import QtQuick
import QtMultimedia
import Qt.labs.folderlistmodel
import Qt5Compat.GraphicalEffects

Rectangle {
    id: musicPlayer
    color: "transparent"
    anchors.fill: parent
    clip: true

    signal playRequested()
    signal pauseRequested()
    signal nextRequested()
    signal previousRequested()
    signal trackChanged(int index)

    FolderListModel {
        id: folderModel
        folder: "file:///home/team1/seame/Car_control_Raspberry/Dashboard/songs"
        nameFilters: ["*.mp3", "*.MP3"]
        showDirs: false
        
        onCountChanged: {
            if (count > 0 && currentFile === "") {
                setTrack(0)
            }
        }
    }

    property int currentIndex: 0
    property bool isPlaying: false
    property string currentFile: ""
    property string currentTitle: "No track"

    MediaPlayer {
        id: player
        audioOutput: AudioOutput {}
        
        onPlaybackStateChanged: {
            musicPlayer.isPlaying = (playbackState === MediaPlayer.PlayingState)
        }
        onPositionChanged: {
            // Progress bar commented out
        }
        onSourceChanged: {
        }
        onMetaDataChanged: {
            console.log("=== Metadata Changed ===")
            
            // Update title from metadata if available
            var title = metaData.stringValue(MediaMetaData.Title)
            var artist = metaData.stringValue(MediaMetaData.ContributingArtist)
            var album = metaData.stringValue(MediaMetaData.AlbumTitle)
            
            console.log("Title:", title)
            console.log("Artist:", artist)
            console.log("Album:", album)
            
            if (title && title !== "") {
                console.log("Setting currentTitle to:", title)
                currentTitle = title
            }
            
            // Update album art
            var coverArtImage = metaData.value(MediaMetaData.CoverArtImage)
            if (coverArtImage) {
                console.log("CoverArt found, updating...")
                coverArt.setImage(coverArtImage)
                albumArtSource.source = "image://coverart/" + Date.now()
            }
        }
        onMediaStatusChanged: {
            if (mediaStatus === MediaPlayer.EndOfMedia && folderModel.count > 1) {
                musicPlayer.next()
            }
        }
    }

    Component.onCompleted: {
    }

    function play() {
        console.log("Play button pressed")
        if (folderModel.count > 0) {
            player.play()
        }
        playRequested()
    }
    function pause() {
        console.log("Pause button pressed")
        player.pause()
        pauseRequested()
    }
    function setTrack(index) {
        console.log("Setting track to index:", index)
        if (index < 0) index = 0
        if (index >= folderModel.count) index = folderModel.count - 1
        currentIndex = index
        currentFile = folderModel.get(currentIndex, "fileURL")
        currentTitle = folderModel.get(currentIndex, "fileBaseName")
        player.stop()
        player.source = currentFile
        player.play()
        trackChanged(currentIndex)
    }
    function next() {
        console.log("Next button pressed")
        setTrack((currentIndex + 1) % folderModel.count)
        nextRequested()
    }
    function previous() {
        console.log("Previous button pressed")
        setTrack((currentIndex - 1 + folderModel.count) % folderModel.count)
        previousRequested()
    }

    // INSANE BLUR + DARKENED BACKGROUND
    Item {
        id: blurredBackground
        anchors.fill: parent
        z: 0
        
        Repeater {
            model: 256
            Image {
                width: parent.width * (10.0 + index * 2.0)
                height: parent.height * (10.0 + index * 2.0)
                x: -parent.width * (4.5 + index * 1.0)
                y: -parent.height * (4.5 + index * 1.0)
                fillMode: Image.Stretch
                smooth: true
                cache: false
                z: index
                opacity: 0.005
                source: albumArtSource.source
            }
        }
        
        // Dark overlay
        Rectangle {
            anchors.fill: parent
            color: "#000000"
            opacity: 0.50
            z: 121
        }
    }
    
    Image {
        id: albumArtSource
        width: 1
        height: 1
        fillMode: Image.Stretch
        smooth: true
        cache: false
        visible: false
    }

    // Normal album art display
    Item {
        id: albumArtContainer
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -parent.height * 0.1
        width: parent.height * 0.6
        height: width
        z: 122
        
        Rectangle {
            id: maskRect
            anchors.fill: parent
            radius: 12
            visible: false
        }
        
        Image {
            id: albumArtDisplay
            anchors.fill: parent
            fillMode: Image.PreserveAspectCrop
            smooth: true
            cache: false
            source: albumArtSource.source
            layer.enabled: true
            layer.effect: OpacityMask {
                maskSource: maskRect
            }
        }
    }

    //Column {
    //    anchors.fill: parent
    //    z: 1
//
    //    // Now playing text
        //Column {
        //    anchors.horizontalCenter: parent.horizontalCenter
        //    spacing: parent.height * 0.01
//
        //    Text {
        //        id: trackTitle
        //        text: musicPlayer.currentTitle
        //        color: "#F9F9F9"
        //        font.bold: true
        //        font.pixelSize: parent.height * 0.88
        //        horizontalAlignment: Text.AlignHCenter
        //        anchors.horizontalCenter: parent.horizontalCenter
        //    }
        //}
//
        //// Controls
        //Row {
        //    anchors.horizontalCenter: parent.horizontalCenter
        //    spacing: parent.width * 0.04
        //    height: parent.height * 0.15
//
        //    Rectangle {
        //        id: prevBtn
        //        width: height
        //        height: parent.height
        //        radius: 6
        //        color: "#2a2a2a"
        //        border.color: "#444"
        //        border.width: 2
//
        //        Text {
        //            anchors.centerIn: parent
        //            text: "◀"
        //            color: "#F9F9F9"
        //            font.pixelSize: parent.width * 0.4
        //        }
//
        //        MouseArea { 
        //            anchors.fill: parent
        //            onClicked: {
        //                console.log("Previous button MouseArea clicked")
        //                musicPlayer.previous()
        //            }
        //        }
        //    }
//
        //    Rectangle {
        //        id: playBtn
        //        width: height * 1.2
        //        height: parent.height
        //        radius: 8
        //        color: musicPlayer.isPlaying ? "#00b050" : "#2a2a2a"
        //        border.color: "#444"
        //        border.width: 2
//
        //        Text {
        //            anchors.centerIn: parent
        //            text: musicPlayer.isPlaying ? "⏸" : "▶"
        //            color: "#F9F9F9"
        //            font.pixelSize: parent.width * 0.4
        //        }
//
        //        MouseArea {
        //            anchors.fill: parent
        //            onClicked: {
        //                console.log("Play/Pause button MouseArea clicked")
        //                if (musicPlayer.isPlaying) musicPlayer.pause(); else musicPlayer.play();
        //            }
        //        }
        //    }
//
        //    Rectangle {
        //        id: nextBtn
        //        width: height
        //        height: parent.height
        //        radius: 6
        //        color: "#2a2a2a"
        //        border.color: "#444"
        //        border.width: 2
//
        //        Text {
        //            anchors.centerIn: parent
        //            text: "▶"
        //            color: "#F9F9F9"
        //            font.pixelSize: parent.width * 0.4
        //        }
//
        //        MouseArea { 
        //            anchors.fill: parent
        //            onClicked: {
        //                console.log("Next button MouseArea clicked")
        //                musicPlayer.next()
        //            }
        //        }
        //    }
        //    //}
        //}

        // Progress bar
        //Rectangle {
        //    id: progressBg
        //    width: parent.width * 0.9
        //    height: 6
        //    radius: 3
        //    color: "#2a2a2a"
        //    anchors.horizontalCenter: parent.horizontalCenter
//
        //    Rectangle {
        //        id: progressFill
        //        x: 0
        //        y: 0
        //        width: 0
        //        height: progressBg.height
        //        radius: 3
        //        color: "#00b050"
        //    }
        //}

    //}

}
