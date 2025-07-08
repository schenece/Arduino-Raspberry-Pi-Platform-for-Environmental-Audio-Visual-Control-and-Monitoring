#!/bin/bash

# === Input arguments ===
SESSION_DIR=$1            # Directory to store recordings
TOTAL_DURATION=$2         # Total recording session time (in seconds)
CHUNK_DURATION=$3         # Duration of each audio/video chunk (in seconds)
LOG_FILE=$4               # Full path to session log file

# === Create session directory if it doesn't exist ===
mkdir -p "$SESSION_DIR"

# === Start log for the session ===
echo "[INFO] Session begins at $(date +%Y-%m-%dT%H:%M:%S)" | tee -a "$LOG_FILE" /home/pi/session_log_latest.txt

# === Clear any previously tracked audio/video PIDs ===
> /home/pi/Desktop/recorder_active_pids.txt

# === Calculate when to end the entire session ===
END_TIME=$(( $(date +%s) + TOTAL_DURATION ))
INDEX=1  # Start with chunk index 1

# === Main recording loop ===
while [ $(date +%s) -lt $END_TIME ]; do
  # Timestamp for this chunk
  START_TS=$(date +%Y-%m-%dT%H%M%S)

  # Define output filenames
  AUDIO_FILE="$SESSION_DIR/audio_${START_TS}_chunk${INDEX}.wav"
  VIDEO_FILE="$SESSION_DIR/video_${START_TS}_chunk${INDEX}.mp4"

  # === Log chunk start ===
  echo "[INFO] Audio chunk $INDEX starts at $START_TS" | tee -a "$LOG_FILE" /home/pi/session_log_latest.txt

  # Start audio in background
  arecord -D plughw:1,0 -f cd -t wav -d $((CHUNK_DURATION + 10)) "$AUDIO_FILE" &
  APID=$!

  sleep 1  # Audio gets head start

  # Start video in background
  echo "[INFO] Video chunk $INDEX starts at $(date +%Y-%m-%dT%H:%M:%S)" | tee -a "$LOG_FILE" /home/pi/session_log_latest.txt
  libcamera-vid -t $((CHUNK_DURATION * 1000)) -o "$VIDEO_FILE" --width 640 --height 480 --framerate 30 &
  VPID=$!

  echo "$APID" >> /home/pi/Desktop/recorder_active_pids.txt
  echo "$VPID" >> /home/pi/Desktop/recorder_active_pids.txt

  wait $VPID
  echo "[INFO] Video chunk $INDEX ended at $(date +%Y-%m-%dT%H:%M:%S)" | tee -a "$LOG_FILE" /home/pi/session_log_latest.txt

  wait $APID
  echo "[INFO] Audio chunk $INDEX ended at $(date +%Y-%m-%dT%H:%M:%S)" | tee -a "$LOG_FILE" /home/pi/session_log_latest.txt

  INDEX=$((INDEX + 1))
done

echo "[INFO] Session ended at $(date +%Y-%m-%dT%H:%M:%S)" | tee -a "$LOG_FILE" /home/pi/session_log_latest.txt
