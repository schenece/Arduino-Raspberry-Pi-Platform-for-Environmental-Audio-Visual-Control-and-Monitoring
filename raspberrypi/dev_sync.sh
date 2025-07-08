#!/bin/bash

# === Pi connection info ===
PI_USER="pi"
PI_HOST="172.20.10.3"
PI_PATH="~/Desktop"

# === Files to transfer ===
FILES=("main.sh" "lib.sh" "config.sh")

echo "[🔁] Syncing files to Pi..."
scp "${FILES[@]}" "$PI_USER@$PI_HOST:$PI_PATH" || {
  echo "[❌] Failed to transfer files."
  exit 1
}

echo "[🕒] Current Pi time:"
ssh pi@172.20.10.3 'date "+%Y-%m-%dT%H:%M:%S"'

echo "[🚀] Running main.sh on Pi..."
ssh "$PI_USER@$PI_HOST" "bash $PI_PATH/main.sh"