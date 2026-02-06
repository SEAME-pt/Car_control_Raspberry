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
        folder: "file:///home/afogonca/seame/Car_control_Raspberry/Dashboard/songs"
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
    property string currentArtist: ""

    MediaPlayer {
        id: player
        audioOutput: AudioOutput {}
        
        onPlaybackStateChanged: {
            musicPlayer.isPlaying = (playbackState === MediaPlayer.PlayingState)
        }
        onPositionChanged: {
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
                currentTitle = title
            }
            if (artist && artist !== "") {
                currentArtist = artist
            } else {
                currentArtist = ""
            }
            
            // Update album art - try different methods
            var coverArtImage = metaData.value(MediaMetaData.CoverArtImage)
            
            if (coverArtImage) {
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

    function play() {
        if (folderModel.count > 0) {
            player.play()
        }
        playRequested()
    }
    function pause() {
        player.pause()
        pauseRequested()
    }
    function setTrack(index) {
        if (index < 0) index = 0
        if (index >= folderModel.count) index = folderModel.count - 1
        if (folderModel.count === 0) {
            return
        }
        currentIndex = index
        
        // Build file path manually from folder
        var folderPath = folderModel.folder.toString()
        var fileName = folderModel.get(currentIndex, "fileName")
        
        currentFile = folderPath + "/" + fileName
        currentTitle = fileName.replace(".mp3", "").replace(".MP3", "")
        player.stop()
        player.source = currentFile
        player.play()
        trackChanged(currentIndex)
    }
    function next() {
        setTrack((currentIndex + 1) % folderModel.count)
        nextRequested()
    }
    function previous() {
        setTrack((currentIndex - 1 + folderModel.count) % folderModel.count)
        previousRequested()
    }

    // BLURRED BACKGROUND
    Item {
        id: blurredBackground
        anchors.fill: parent
        z: 0
        
        Image {
            id: backgroundImage
            anchors.fill: parent
            fillMode: Image.PreserveAspectCrop
            smooth: true
            cache: false
            source: albumArtSource.source
            
            layer.enabled: true
            layer.effect: FastBlur {
                radius: 64
            }
        }
        
        // Dark overlay
        Rectangle {
            anchors.fill: parent
            color: "#000000"
            opacity: 0.50
            z: 1
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
        anchors.verticalCenterOffset: -parent.height * 0.125
        width: parent.height * 0.575
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

    Rectangle {
        id: titleBar
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height * 0.075
        height: parent.height * 0.20
        width: parent.width * 0.8
        color: "#000000"
        opacity: 0.50
        radius: parent.height * 0.025
        anchors.horizontalCenter: parent.horizontalCenter
        z: 1
    }

    // Clipped container placed above the translucent background so text remains fully opaque
    Item {
        id: titleClip
        anchors.bottom: parent.bottom
        height: parent.height * 0.17
        width: parent.width * 0.8
        anchors.horizontalCenter: parent.horizontalCenter
        z: 2
        clip: true

        Column {
            anchors.fill: parent
            anchors.margins: parent.width * 0.015
            anchors.verticalCenter: parent.verticalCenter

            Text {
                id: titleText
                text: musicPlayer.currentTitle
                color: "#dcdcdc"
                font.pixelSize: parent.height * 0.33
                font.weight: 600
                anchors.horizontalCenter: parent.horizontalCenter
                x: 0

                property bool needsScroll: width > titleClip.width

                SequentialAnimation on x {
                    running: titleText.needsScroll
                    loops: Animation.Infinite

                    PauseAnimation { duration: 2000 }
                    NumberAnimation {
                        from: 0
                        to: -(titleText.width - titleClip.width)
                        duration: titleText.width * 15
                        easing.type: Easing.Linear
                    }
                    PauseAnimation { duration: 2000 }
                    NumberAnimation {
                        from: -(titleText.width - titleClip.width)
                        to: 0
                        duration: titleText.width * 15
                        easing.type: Easing.Linear
                    }
                }
            }

            Text {
                id: artistText
                text: musicPlayer.currentArtist
                visible: musicPlayer.currentArtist !== ""
                color: "#c5c5c5"
                font.pixelSize: parent.height * 0.24
                font.weight: 500
                anchors.horizontalCenter: parent.horizontalCenter
                x: 0
                opacity: 0.9

                property bool needsScroll: width > titleClip.width

                SequentialAnimation on x {
                    running: artistText.needsScroll
                    loops: Animation.Infinite

                    PauseAnimation { duration: 2000 }
                    NumberAnimation {
                        from: 0
                        to: -(artistText.width - titleClip.width)
                        duration: artistText.width * 12
                        easing.type: Easing.Linear
                    }
                    PauseAnimation { duration: 2000 }
                    NumberAnimation {
                        from: -(artistText.width - titleClip.width)
                        to: 0
                        duration: artistText.width * 12
                        easing.type: Easing.Linear
                    }
                }
            }

        //Progress bar
        Rectangle {
            id: progressBg
            width: parent.width * 0.9
            height: 6
            radius: 3
            color: "#2a2a2a"
            anchors.horizontalCenter: parent.horizontalCenter

            Rectangle {
                id: progressFill
                x: 0
                y: 0
                width: 0
                height: progressBg.height
                radius: 3
                color: "#00b050"
            }
        }
        }
    }

    Column {
        anchors.fill: parent
        z: 1
        // Now playing text
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


    }
}
