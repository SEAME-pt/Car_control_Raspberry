#!/bin/bash

LOG_FILE="emergencyBreakLatencyTest.log"

# Check if log file exists
if [ ! -f "$LOG_FILE" ]; then
    echo "No log file found???"
    exit 1
fi

awk -F, '
/LATENCY/ {
    latency = $2
    sum += latency
    count++
    printf "Latency %2d: %6d μs (%7.3f ms)\n", count, latency, latency/1000
    if (count == 1 || latency < min) min = latency
    if (count == 1 || latency > max) max = latency
}
END { 
    if (count > 0) {
        avg = sum / count
        print "\n==========================="
        print "Latency Analysis"
        print "==========================="
        print "Total measurements:", count
        printf "Minimum latency:   %d μs (%.3f ms)\n", min, min/1000
        printf "Maximum latency:   %d μs (%.3f ms)\n", max, max/1000
        printf "Average latency:   %d μs (%.3f ms)\n", avg, avg/1000
        print "==========================="
    } else {
        print "No latency data found in log file."
    }
}' "$LOG_FILE"