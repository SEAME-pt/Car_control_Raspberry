for f in *.mp3; do
	ffmpeg -i "$f" -i front.jpg -map 0 -map 1 \
		-c copy -id3v2_version 3 \
		-metadata:s:v title="Album cover" \
		-metadata:s:v comment="Cover (front)" \
		"tagged_$f"
	rm -f "$f"
	mv "tagged_$f" "$f"
done

