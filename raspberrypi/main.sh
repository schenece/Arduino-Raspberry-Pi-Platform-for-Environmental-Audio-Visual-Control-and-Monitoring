#!/bin/bash

cd "$(dirname "$0")" || exit 1

# ===============================
#     Load Dependencies
# ===============================
source ./config.sh
source ./lib.sh

# ===============================
#     Dev Mode Overrides (macOS)
#     If this script is running on macOS, then execute the following block.
# ===============================
# if [[ "$OSTYPE" == "darwin"* ]]; then
#     echo "[🧪] macOS detected — applying overrides"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "[🐧] Raspberry Pi detected — applying dev overrides"
    
    # Session Start Time
    START_HOUR=16
    START_MINUTE=03

    # Total Session Duration
    DURATION_HOUR=0
    DURATION_MINUTE=10
    DURATION_SECOND=0

    # Per-Chunk File Duration
    FILE_DURATION_HOUR=0
    FILE_DURATION_MINUTE=2
    FILE_DURATION_SECOND=0

    # Camera Settings
    VIDEO_WIDTH=640
    VIDEO_HEIGHT=480
    VIDEO_FPS=30

    # Audio Settings
    AUDIO_SAMPLE_RATE=48000
    AUDIO_CHANNELS=1
    AUDIO_DEVICE="hw:1,0"

    # System Timers
    # Interval (in seconds) between state checks during main loop
    STATE_CHECK_INTERVAL=5

    # Derived Values (in seconds)
    SESSION_TOTAL_DURATION=$(( DURATION_HOUR * 3600 + DURATION_MINUTE * 60 + DURATION_SECOND ))
    CHUNK_DURATION=$(( FILE_DURATION_HOUR * 3600 + FILE_DURATION_MINUTE * 60 + FILE_DURATION_SECOND ))

    # # Override: get_current_iso
    # get_current_iso() {
    #     date "+%Y-%m-%dT%H:%M:%S"
    # }

    # # Override: start_mic
    # start_mic() {
    #     local folder="$1"
    #     local filename="$2"
    #     local duration="$3"
    #     local filepath="${folder}/${filename}.wav"
    #     echo "[🎤] (SIM) Starting mic: $filepath ($duration s)"
    #     bash -c "exec -a mic_sim sleep $duration" &
    #     echo "[🎤] Mic PID: $!"
    # }

    # # Override: start_cam
    # start_cam() {
    #     local folder="$1"
    #     local filename="$2"
    #     local duration="$3"
    #     local filepath="${folder}/${filename}.h264"
    #     echo "[🎥] (SIM) Starting cam: $filepath ($duration s)"
    #     bash -c "exec -a cam_sim sleep $duration" &
    #     echo "[🎥] Cam PID: $!"
    # }
fi

# ===============================
#     Signal-safe Cleanup
# ===============================
clean_up() {
  echo
  echo "[🔻] Interrupt received — shutting down recorders..."
  force_kill_recorders
  echo "[⏳] Waiting for all processes to stop..."
  wait_until is_mic_stopped 5
  wait_until is_cam_stopped 5
  echo "[✅] All recorders stopped. Cleanup complete."
  exit 0
}

trap clean_up SIGINT

# ===============================
#     Step 1: Get Current Time
# ===============================
now=$(get_current_iso)

# ===============================
#     Step 2: Calculate Active Window
# ===============================
read -r window_start window_end <<< "$(compute_active_window "$now" "$START_HOUR" "$START_MINUTE" "$SESSION_TOTAL_DURATION")"

# Explicit sanity check
if [[ -z "$window_start" || -z "$window_end" ]]; then
  echo "[ℹ] Not within active window — no action taken."
  exit 0
fi

# ===============================
#     Step 3: Compute Session Folder
# ===============================
folder=$(get_session_folder_name "$window_start" "$window_end")

if ! check_exists "$folder"; then
  echo "[📂] Creating session folder: $folder"
  mkdir -p "$folder"
  mkdir -p "$folder/log"
fi

# ===============================
#     Step 4: Check Last Chunk
# ===============================
last_chunk_file="$folder/.last_chunk"
start_iso="$now"

if [[ -f "$last_chunk_file" ]]; then
  last_end_iso=$(cat "$last_chunk_file")
  if [[ -n "$last_end_iso" ]] && is_within_window "$now" "$window_start" "$last_end_iso"; then
    sleep 1  # Let any orphan processes stabilize

    if is_mic_running && is_cam_running; then
      echo "[✔] Both recorders are still running — suppressing trigger."
      exit 0
    else
      echo "[🔻] One or both recorders are NOT running "
      echo "[🔁] Detected reboot or interruption mid-chunk — resuming."
      echo "[🧹] Cleaning up leftover processes (if any)..."
      force_kill_recorders
      # Proceed to Step 5
    fi
  fi
fi

# ===============================
#     Step 5: Launch Recorders
# ===============================
read -r duration chunk_end_iso <<< "$(compute_chunk_duration "$start_iso" "$window_end")"
if [[ "$duration" == "SKIP" ]]; then
  echo "[🛑] No time remaining in session window — exiting."
  exit 0
fi

start_recorders "$folder" "$start_iso" "$window_end" "$duration"

# ===============================
#     Step 6: Save New Chunk End
# ===============================
echo "$chunk_end_iso" > "$last_chunk_file"
echo "[💾] Updated last chunk record: $chunk_end_iso"

# ===============================
#     Step 7: Optional Pause
# ===============================
sleep "$STATE_CHECK_INTERVAL"

# ===============================
#     Done — exit cleanly
# ===============================
exit 0