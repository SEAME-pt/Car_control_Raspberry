# Album Art

MP3 metadata contains embedded album art as QImage. QML calls `coverArt.setImage(qimage)` to update a shared QImage pointer. The `CoverArtProvider` reads this pointer when QML requests `image://coverart/` URLs. Everything in memory, no disk I/O.
x