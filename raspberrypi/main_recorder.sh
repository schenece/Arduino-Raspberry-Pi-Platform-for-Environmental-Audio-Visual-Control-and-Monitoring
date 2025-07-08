#!/bin/bash

# === Load recording schedule and chunk settings from config ===
CONFIG_FILE="/home/pi/Desktop/rec_config.env"
source "$CONFIG_FILE"

# === Track this script's PID for manual stop ===
echo $$ > /home/pi/Desktop/recorder_main.pid

# === Define directories and state file path ===
RECORDING_DIR="/home/pi/Desktop/recordings"
STATE_FILE="/home/pi/Desktop/recording_state.txt"
mkdir -p "$RECORDING_DIR"

# === Get current time and calculate window ===
NOW_SEC=$(date +%s)
START_SEC=$(date -d "today $START_HOUR:$START_MINUTE" +%s)
END_SEC=$(( START_SEC + DURATION_HOUR*3600 + DURATION_MINUTE*60 + DURATION_SECOND ))

# === Decide when to start ===
if [ $NOW_SEC -lt $START_SEC ]; then
  WAIT_SECONDS=$(( START_SEC - NOW_SEC ))
  echo "IDLE" > "$STATE_FILE"
  echo "[INFO] Waiting $WAIT_SECONDS seconds until ${START_HOUR}:${START_MINUTE}..."
  while [ $WAIT_SECONDS -gt 0 ]; do
    echo -ne "\r[INFO] Waiting $WAIT_SECONDS seconds..."
    sleep 1
    WAIT_SECONDS=$(( WAIT_SECONDS - 1 ))
  done
  echo ""
elif [ $NOW_SEC -ge $START_SEC ] && [ $NOW_SEC -lt $END_SEC ]; then
  echo "[INFO] Already within ACTIVE window. Skipping wait."
else
  echo "[WARN] Outside scheduled window. Exiting."
  echo "COMPLETED" > "$STATE_FILE"
  rm -f /home/pi/Desktop/recorder_main.pid
  exit 0
fi

# === Prepare session ===
SESSION_TS=$(date +%Y-%m-%dT%H%M%S)
SESSION_DIR="$RECORDING_DIR/session_$SESSION_TS"
mkdir -p "$SESSION_DIR"
echo "ACTIVE" > "$STATE_FILE"

# === Log loaded settings ===
{
  echo "[INFO] User settings loaded:"
  echo "  Start time: ${START_HOUR}:${START_MINUTE}"
  echo "  Total duration: ${DURATION_HOUR}h ${DURATION_MINUTE}m ${DURATION_SECOND}s"
  echo "  File duration: ${FILE_DURATION_HOUR}h ${FILE_DURATION_MINUTE}m ${FILE_DURATION_SECOND}s"
} | tee -a "$SESSION_DIR/session_log__${SESSION_TS}.txt"

# === Ctrl+C cleanup ===
cleanup_and_exit() {
  echo "[WARN] Ctrl+C detected. Cleaning up..."
  [ -f /home/pi/Desktop/recorder_active_pids.txt ] && while read PID; do
    kill -9 "$PID" 2>/dev/null
  done < /home/pi/Desktop/recorder_active_pids.txt
  rm -f /home/pi/Desktop/recorder_active_pids.txt
  rm -f /home/pi/Desktop/recorder_main.pid
  echo "COMPLETED" > "$STATE_FILE"
  [ -f "$SESSION_DIR/session_log__${SESSION_TS}.txt" ] && {
    echo "[WARN] Ctrl+C detected. Cleaning up..." >> "$SESSION_DIR/session_log__${SESSION_TS}.txt"
    echo "[INFO] Cleanup complete. Exiting." >> "$SESSION_DIR/session_log__${SESSION_TS}.txt"
  }
  echo "[INFO] Cleanup complete. Exiting."
  exit 1
}
trap cleanup_and_exit INT

# === Launch chunked recorder ===
/home/pi/Desktop/record_both.sh "$SESSION_DIR" \
  $((DURATION_HOUR * 3600 + DURATION_MINUTE * 60 + DURATION_SECOND)) \
  $((FILE_DURATION_HOUR * 3600 + FILE_DURATION_MINUTE * 60 + FILE_DURATION_SECOND)) \
  "$SESSION_DIR/session_log__${SESSION_TS}.txt"

# === Finish ===
echo "COMPLETED" > "$STATE_FILE"
{
  echo "[INFO] Launch complete at $(date +%Y-%m-%dT%H:%M:%S)"
  echo "[INFO] Main recorder exited"
} | tee -a "$SESSION_DIR/session_log__${SESSION_TS}.txt"

rm -f /home/pi/Desktop/recorder_main.pid
exit 0
