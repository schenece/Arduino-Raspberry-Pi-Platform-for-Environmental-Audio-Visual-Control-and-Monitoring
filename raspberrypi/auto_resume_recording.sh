#!/bin/bash

CONFIG_FILE="/home/pi/Desktop/rec_config.env"
STATE_FILE="/home/pi/Desktop/recording_state.txt"
MAIN_SCRIPT="/home/pi/Desktop/main_recorder.sh"
LOG_FILE="/home/pi/Desktop/auto_resume_log.txt"

# === Load config ===
source "$CONFIG_FILE"

# === Timestamp function ===
log() {
  echo "[INFO] $(date +%Y-%m-%dT%H:%M:%S) $1" >> "$LOG_FILE"
}

# === Time calculations ===
NOW_SEC=$(date +%s)
START_SEC=$(date -d "today $START_HOUR:$START_MINUTE" +%s)
END_SEC=$(( START_SEC + DURATION_HOUR*3600 + DURATION_MINUTE*60 + DURATION_SECOND ))

# === Evaluate based on state file ===
if [ -f "$STATE_FILE" ]; then
  STATE=$(cat "$STATE_FILE")
  if [ "$STATE" == "COMPLETED" ]; then
    log "Session already completed. Skipping launch."
    exit 0
  elif [ "$STATE" == "ACTIVE" ]; then
    log "State file shows ACTIVE. Resuming recording..."
    /bin/bash "$MAIN_SCRIPT" &
    exit 0
  fi
fi

# === If we're within the active window, launch ===
if [ $NOW_SEC -ge $START_SEC ] && [ $NOW_SEC -lt $END_SEC ]; then
  log "Detected reboot within ACTIVE window. Launching recorder."
  /bin/bash "$MAIN_SCRIPT" &
  exit 0
fi

# === Default path: outside session window ===
log "Rebooted outside active window. Nothing to do."
exit 0
