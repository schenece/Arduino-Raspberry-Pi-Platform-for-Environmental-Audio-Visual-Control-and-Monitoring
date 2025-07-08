#!/bin/bash

# ========================
#      TIME CONVERTERS
# ========================

iso_to_epoch() {
  local iso="$1"
  if [[ "$OSTYPE" == "darwin"* ]]; then
    date -j -f "%Y-%m-%dT%H:%M:%S" "$iso" +%s
  else
    date -d "$iso" +%s
  fi
}

epoch_to_iso() {
  local epoch="$1"
  if [[ "$OSTYPE" == "darwin"* ]]; then
    date -j -r "$epoch" "+%Y-%m-%dT%H:%M:%S"
  else
    date -d "@$epoch" "+%Y-%m-%dT%H:%M:%S"
  fi
}

# # Test ISO → Epoch → ISO roundtrip
# iso="2025-06-27T19:30:00"
# epoch=$(iso_to_epoch "$iso")
# iso_back=$(epoch_to_iso "$epoch")

# echo "Original ISO      : $iso"
# echo "Converted to Epoch: $epoch"
# echo "Back to ISO       : $iso_back"

# # Check roundtrip
# if [[ "$iso" == "$iso_back" ]]; then
#   echo "✅ Roundtrip PASSED"
# else
#   echo "❌ Roundtrip FAILED"
# fi

# ================================================================================================================================================
#     normalize_time_to_iso
# ================================================================================================================================================

normalize_time_to_iso() {
  local device="$1"
  local raw="$2"

  case "$device" in
    mac)
      echo "$raw" ;;  # Already ISO
    pi)
      echo "$raw" | cut -d'.' -f1 | sed 's/ /T/' ;;
    win | windows)
      echo "$raw" | sed 's/ /T/' ;;
    user | iso)
      echo "$raw" ;;  # From override, assume ISO
    *)
      echo "ERROR: Unknown device '$device'" >&2
      return 1 ;;
  esac
}

# # -------------------------------------
# # Pseudo RTC Input from Raspberry Pi
# # (simulate hwclock -r)
# # -------------------------------------
# raw_pi="2025-06-27 19:30:00.123456"

# # -------------------------------------
# # Actual RTC Input from macOS
# # -------------------------------------
# raw_mac=$(date "+%Y-%m-%dT%H:%M:%S")

# # -------------------------------------
# # Run tests
# # -------------------------------------
# echo "====== Test: normalize_time_to_iso ======"
# echo "--- Simulated Pi RTC input ---"
# echo "Raw Pi      : $raw_pi"
# iso_pi=$(normalize_time_to_iso "pi" "$raw_pi")
# echo "→ ISO Output: $iso_pi"

# echo
# echo "--- Actual Mac RTC input ---"
# echo "Raw Mac     : $raw_mac"
# iso_mac=$(normalize_time_to_iso "mac" "$raw_mac")
# echo "→ ISO Output: $iso_mac"

# echo
# echo "====== Done ======"

get_current_iso() {
  # Prefer hwclock if it works, fallback to system time
  if output=$(sudo hwclock -r 2>/dev/null); then
    normalize_time_to_iso "pi" "$output"
  else
    date "+%Y-%m-%dT%H:%M:%S"
  fi
}

# ================================================================================================================================================
#     convert_iso_to_format
# ================================================================================================================================================

convert_iso_to_format() {
  local iso="$1"
  local format="$2"

  case "$format" in
    iso)
      echo "$iso"
      ;;
    epoch)
      iso_to_epoch "$iso"
      ;;
    *)
      echo "ERROR: Unsupported format '$format'" >&2
      return 1
      ;;
  esac
}

# echo
# echo "====== Test: convert_iso_to_format ======"

# raw_pi="2025-06-27 19:30:00.123456"
# raw_mac=$(date "+%Y-%m-%dT%H:%M:%S")

# # Normalize to ISO
# iso_pi=$(normalize_time_to_iso "pi" "$raw_pi")
# iso_mac=$(normalize_time_to_iso "mac" "$raw_mac")

# echo "Pi  → ISO: $iso_pi"
# echo "Mac → ISO: $iso_mac"

# echo "--- Convert Pi ISO to Epoch ---"
# epoch_pi=$(convert_iso_to_format "$iso_pi" "epoch")
# echo "Pi Epoch: $epoch_pi"

# echo "--- Convert Mac ISO to Epoch ---"
# epoch_mac=$(convert_iso_to_format "$iso_mac" "epoch")
# echo "Mac Epoch: $epoch_mac"

# echo "--- Pass-through ISO (Mac) ---"
# echo "Mac ISO : $(convert_iso_to_format "$iso_mac" "iso")"

# ================================================================================================================================================
#     convert_epoch_to_format
# ================================================================================================================================================

# raw_pi="2025-06-27 19:30:00.123456"
# raw_mac=$(date "+%Y-%m-%dT%H:%M:%S")

# # Normalize to ISO
# iso_pi=$(normalize_time_to_iso "pi" "$raw_pi")
# iso_mac=$(normalize_time_to_iso "mac" "$raw_mac")

# echo "Pi  → ISO: $iso_pi"
# echo "Mac → ISO: $iso_mac"

# # ISO → Epoch
# epoch_pi=$(convert_iso_to_format "$iso_pi" "epoch")
# epoch_mac=$(convert_iso_to_format "$iso_mac" "epoch")

convert_epoch_to_format() {
  local epoch="$1"
  local format="$2"
  case "$format" in
    iso)   epoch_to_iso "$epoch" ;;
    epoch) echo "$epoch" ;;
  esac
}

# # Epoch → ISO
# echo
# echo "====== Test: convert_epoch_to_format ======"
# echo "Pi  Epoch → ISO: $(convert_epoch_to_format "$epoch_pi" "iso")"
# echo "Mac Epoch → ISO: $(convert_epoch_to_format "$epoch_mac" "iso")"
# echo "Mac Epoch → Epoch: $(convert_epoch_to_format "$epoch_mac" "epoch")"

# ================================================================================================================================================
#     load_active_window()
# ================================================================================================================================================

compute_window_from_day() {
  local day="$1"        # e.g. 2025-06-27
  local hour="$2"       # e.g. 19
  local minute="$3"     # e.g. 30
  local duration="$4"   # in seconds

  if [[ -z "$day" || -z "$hour" || -z "$minute" || -z "$duration" ]]; then
    echo "ERROR: Missing arguments to compute_window_from_day" >&2
    return 1
  fi

  if ! [[ "$duration" =~ ^[0-9]+$ ]]; then
    echo "ERROR: Duration must be integer seconds, got '$duration'" >&2
    return 1
  fi

  local start_iso="${day}T$(printf "%02d:%02d:00" "$hour" "$minute")"
  local start_epoch end_epoch end_iso

  start_epoch=$(iso_to_epoch "$start_iso") || return 1
  end_epoch=$(( start_epoch + duration ))
  end_iso=$(epoch_to_iso "$end_epoch") || return 1

  echo "$start_iso $end_iso"
}

# # !/bin/bash
# source ./utils.sh  # contains iso_to_epoch, epoch_to_iso
# source ./time_logic.sh  # contains compute_window_from_day()

# echo "===== Test: compute_window_from_day ====="

# day="2025-06-27"
# hour=19
# minute=30
# duration=36000

# echo "--- Input ---"
# echo "Day:      $day"
# echo "Start:    ${hour}:${minute}"
# echo "Duration: ${duration}s"

# read -r start_iso end_iso <<< "$(compute_window_from_day "$day" "$hour" "$minute" "$duration")"

# echo "--- Output ---"
# echo "Start ISO: $start_iso"
# echo "End ISO:   $end_iso"

# ================================================================================================================================================
#     is_within_window()
# ================================================================================================================================================

is_within_window() {
  local current_iso="$1"
  local start_iso="$2"
  local end_iso="$3"

  if [[ -z "$current_iso" || -z "$start_iso" || -z "$end_iso" ]]; then
    echo "ERROR: is_within_window requires 3 ISO arguments" >&2
    return 2
  fi

  local current_epoch start_epoch end_epoch
  current_epoch=$(iso_to_epoch "$current_iso") || return 1
  start_epoch=$(iso_to_epoch "$start_iso") || return 1
  end_epoch=$(iso_to_epoch "$end_iso") || return 1

  if (( current_epoch >= start_epoch && current_epoch < end_epoch )); then
    return 0  # TRUE
  else
    return 1  # FALSE
  fi
}

# echo
# echo "===== Test: is_within_window (10-hour session) ====="

# start="2025-06-27T19:30:00"
# end="2025-06-28T05:30:00"  # 10 hours later

# declare -a cases=(
#   "2025-06-27T19:29:59 OUTSIDE"
#   "2025-06-27T19:30:00 INSIDE"
#   "2025-06-27T23:59:59 INSIDE"
#   "2025-06-28T04:00:00 INSIDE"
#   "2025-06-28T05:29:59 INSIDE"
#   "2025-06-28T05:30:00 OUTSIDE"
#   "2025-06-28T06:00:00 OUTSIDE"
# )

# for entry in "${cases[@]}"; do
#   current=$(echo "$entry" | cut -d' ' -f1)
#   expected=$(echo "$entry" | cut -d' ' -f2)

#   if is_within_window "$current" "$start" "$end"; then
#     result="INSIDE"
#   else
#     result="OUTSIDE"
#   fi

#   printf "Time: %-20s → Result: %-7s [%s]\n" "$current" "$result" \
#     "$( [[ "$result" == "$expected" ]] && echo PASS || echo FAIL )"
# done

# ================================================================================================================================================
#     pick_matching_window()
# ================================================================================================================================================

pick_matching_window() {
  local current="$1"
  local today_start="$2"
  local today_end="$3"
  local yest_start="$4"
  local yest_end="$5"

  if is_within_window "$current" "$today_start" "$today_end"; then
    echo "$today_start $today_end"
    return 0
  elif is_within_window "$current" "$yest_start" "$yest_end"; then
    echo "$yest_start $yest_end"
    return 0
  else
    return 1  # Not within any window
  fi
}

# echo
# echo "===== Test: pick_matching_window() ====="

# # Define test windows (10hr duration)
# today_start="2025-06-28T19:30:00"
# today_end="2025-06-29T05:30:00"

# yest_start="2025-06-27T19:30:00"
# yest_end="2025-06-28T05:30:00"

# declare -a test_times=(
#   "2025-06-27T22:00:00"
#   "2025-06-28T04:00:00"
#   "2025-06-28T20:00:00"
#   "2025-06-29T04:59:59"
#   "2025-06-29T05:30:00"
#   "2025-06-27T10:00:00"
# )

# for current in "${test_times[@]}"; do
#   if result=$(pick_matching_window "$current" "$today_start" "$today_end" "$yest_start" "$yest_end"); then
#     echo "Time: $current → Window: ${result// / → }"
#   else
#     echo "Time: $current → Window: NONE"
#   fi
# done

# ================================================================================================================================================
#     compute_active_window()
# ================================================================================================================================================

compute_active_window() {
  local current_iso="$1"
  local hour="$2"
  local minute="$3"
  local duration="$4"

  if [[ -z "$current_iso" || -z "$hour" || -z "$minute" || -z "$duration" ]]; then
    echo "ERROR: Missing arguments to compute_active_window" >&2
    return 1
  fi

  local current_day=$(echo "$current_iso" | cut -d'T' -f1)

  # Compute yesterday’s date in YMD
  if [[ "$OSTYPE" == "darwin"* ]]; then
    local yest_day=$(date -j -v-1d -f "%Y-%m-%d" "$current_day" "+%Y-%m-%d")
  else
    local yest_day=$(date -d "$current_day -1 day" "+%Y-%m-%d")
  fi

  # Build both windows
  read -r today_start today_end <<< "$(compute_window_from_day "$current_day" "$hour" "$minute" "$duration")" || return 1
  read -r yest_start yest_end <<< "$(compute_window_from_day "$yest_day" "$hour" "$minute" "$duration")" || return 1

  # Return matching window (if any)
  pick_matching_window "$current_iso" "$today_start" "$today_end" "$yest_start" "$yest_end"
}

# echo
# echo "===== Test: compute_active_window() ====="

# hour=19
# minute=30
# duration=36000  # 10 hours

# declare -a test_times=(
#   "2025-06-27T22:00:00"
#   "2025-06-28T04:00:00"
#   "2025-06-28T20:00:00"
#   "2025-06-29T04:59:59"
#   "2025-06-29T05:30:00"
#   "2025-06-27T10:00:00"
# )

# for current in "${test_times[@]}"; do
#   if result=$(compute_active_window "$current" "$hour" "$minute" "$duration"); then
#     echo "Time: $current → Active Window: ${result// / → }"
#   else
#     echo "Time: $current → Active Window: NONE"
#   fi
# done

# ================================================================================================================================================
#     get_system_state()
# ================================================================================================================================================

get_system_state() {
  local current="$1"
  local start="$2"
  local end="$3"

  if is_within_window "$current" "$start" "$end"; then
    echo "ACTIVE"
  else
    echo "IDLE"
  fi
}

# echo
# echo "===== Test: get_system_state() ====="

# # 10-hour duration window
# session_duration=36000
# hour=19
# minute=30

# # Build both windows based on a reference date
# today="2025-06-28"

# # Get yesterday (reused logic)
# if [[ "$OSTYPE" == "darwin"* ]]; then
#   yesterday=$(date -j -v-1d -f "%Y-%m-%d" "$today" "+%Y-%m-%d")
# else
#   yesterday=$(date -d "$today -1 day" "+%Y-%m-%d")
# fi

# read -r today_start today_end <<< "$(compute_window_from_day "$today" "$hour" "$minute" "$session_duration")"
# read -r yest_start yest_end   <<< "$(compute_window_from_day "$yesterday" "$hour" "$minute" "$session_duration")"

# declare -a test_times=(
#   "2025-06-27T22:00:00 ACTIVE"
#   "2025-06-28T04:00:00 ACTIVE"
#   "2025-06-28T20:00:00 ACTIVE"
#   "2025-06-29T04:59:59 ACTIVE"
#   "2025-06-29T05:30:00 IDLE"
#   "2025-06-27T10:00:00 IDLE"
# )

# for line in "${test_times[@]}"; do
#   iso_now=$(echo "$line" | cut -d' ' -f1)
#   expected=$(echo "$line" | cut -d' ' -f2)

#   match=$(pick_matching_window "$iso_now" "$today_start" "$today_end" "$yest_start" "$yest_end")
#   if [[ -n "$match" ]]; then
#     read -r match_start match_end <<< "$match"
#     verdict=$(get_system_state "$iso_now" "$match_start" "$match_end")
#   else
#     verdict="IDLE"
#   fi

#   printf "Time: %-20s → State: %-6s [%s]\n" "$iso_now" "$verdict" \
#     "$( [[ "$verdict" == "$expected" ]] && echo PASS || echo FAIL )"
# done

# ================================================================================================================================================
#     get_session_folder_name()
# ================================================================================================================================================

get_session_folder_name() {
  local session_start_iso="$1"
  local session_end_iso="$2"

  # Parse values
  local start_year="${session_start_iso:0:4}"

  local start_stamp end_stamp
  start_stamp=$(iso_to_MMDDhhmm "$session_start_iso")
  end_stamp=$(iso_to_MMDDhhmm "$session_end_iso")

  echo "session_${start_year}_${start_stamp}_${end_stamp}"
}

# Convert ISO timestamp to MMDDhhmm for compact use
iso_to_MMDDhhmm() {
  local iso="$1"
  if [[ "$OSTYPE" == "darwin"* ]]; then
    date -j -f "%Y-%m-%dT%H:%M:%S" "$iso" "+%m%d%H%M"
  else
    date -d "$iso" "+%m%d%H%M"
  fi
}

# echo
# echo "===== Test: get_session_folder_name ====="

# start="2025-06-27T19:30:00"
# end="2025-06-28T05:30:00"
# expected="session_2025_06271930_06280530"
# result=$(get_session_folder_name "$start" "$end")

# echo "Start ISO: $start"
# echo "End ISO  : $end"
# echo "Expected : $expected"
# echo "Result   : $result"

# if [[ "$result" == "$expected" ]]; then
#   echo "✅ PASS"
# else
#   echo "❌ FAIL"
# fi

# echo "=========================================="

iso_to_MMDDhhmmss() {
  local iso="$1"
  if [[ "$OSTYPE" == "darwin"* ]]; then
    date -j -f "%Y-%m-%dT%H:%M:%S" "$iso" "+%m%d%H%M%S"
  else
    date -d "$iso" "+%m%d%H%M%S"
  fi
}

iso_to_hhmmss() {
  local iso="$1"
  if [[ "$OSTYPE" == "darwin"* ]]; then
    date -j -f "%Y-%m-%dT%H:%M:%S" "$iso" "+%H%M%S"
  else
    date -d "$iso" "+%H%M%S"
  fi
}

iso_to_hhmm() {
  local iso="$1"
  if [[ "$OSTYPE" == "darwin"* ]]; then
    date -j -f "%Y-%m-%dT%H:%M:%S" "$iso" "+%H%M"
  else
    date -d "$iso" "+%H%M"
  fi
}

iso_to_year() {
  local iso="$1"
  echo "${iso:0:4}"
}

# # Test input
# iso="2025-06-27T19:30:45"
# echo "Input ISO: $iso"
# echo

# # Expected outputs
# expected_MMDDhhmm="06271930"
# expected_MMDDhhmmss="0627193045"
# expected_hhmm="1930"
# expected_year="2025"

# # Actual outputs
# actual_MMDDhhmm=$(iso_to_MMDDhhmm "$iso")
# actual_MMDDhhmmss=$(iso_to_MMDDhhmmss "$iso")
# actual_hhmm=$(iso_to_hhmm "$iso")
# actual_year=$(iso_to_year "$iso")

# # Print results
# echo "iso_to_MMDDhhmm     → $actual_MMDDhhmm     [expected: $expected_MMDDhhmm]"
# echo "iso_to_MMDDhhmmss   → $actual_MMDDhhmmss   [expected: $expected_MMDDhhmmss]"
# echo "iso_to_hhmm         → $actual_hhmm         [expected: $expected_hhmm]"
# echo "iso_to_year         → $actual_year         [expected: $expected_year]"

# # Test results
# [[ "$actual_MMDDhhmm" == "$expected_MMDDhhmm" ]] && echo "✅ MMDDhhmm PASSED" || echo "❌ MMDDhhmm FAILED"
# [[ "$actual_MMDDhhmmss" == "$expected_MMDDhhmmss" ]] && echo "✅ MMDDhhmmss PASSED" || echo "❌ MMDDhhmmss FAILED"
# [[ "$actual_hhmm" == "$expected_hhmm" ]] && echo "✅ hhmm PASSED" || echo "❌ hhmm FAILED"
# [[ "$actual_year" == "$expected_year" ]] && echo "✅ year PASSED" || echo "❌ year FAILED"

# echo "======= Done ======="

# ================================================================================
# check_exists: General path existence checker
# Usage: check_exists <path>
# Returns 0 (true) if exists, 1 otherwise
# ================================================================================

check_exists() {
  local path="$1"
  [[ -e "$path" ]]
}

# echo
# echo "===== Test: Create Session Folder from Current Active Window ====="

# # Load configuration and logic
# source ./config.sh

# # Step 1: Get current time (simulating Pi)
# raw_pi="2025-06-30 00:28:00.123"
# current_iso=$(normalize_time_to_iso "pi" "$raw_pi")

# # Step 2: Use current time + config to compute active window
# start_hour="$START_HOUR"
# start_minute="$START_MINUTE"
# duration_secs="$SESSION_TOTAL_DURATION"

# read -r window_start window_end <<< "$(compute_active_window "$current_iso" "$start_hour" "$start_minute" "$duration_secs")"

# echo "→ Active Window: $window_start → $window_end"

# # Step 3: Generate session folder name
# folder_name=$(get_session_folder_name "$window_start" "$window_end")
# echo "→ Folder name: $folder_name"

# # Step 4: Check if folder exists (should be false)
# if check_exists "$folder_name"; then
#   echo "✖ Folder unexpectedly exists: $folder_name"
# else
#   echo "✔ Folder does not yet exist."
# fi

# # Step 5: Create folder
# mkdir "$folder_name"
# echo "→ Created folder: $folder_name"

# # Step 6: Check existence again (should be true)
# if check_exists "$folder_name"; then
#   echo "✔ Folder exists after creation."
# else
#   echo "✖ Folder was not created properly!"
# fi

# # Step 7: Cleanup
# rm -r "$folder_name"
# echo "🧹 Cleanup done."

# echo "===== Done ====="

# ================================================================================================================================================
#     get_session_file_name()
# ================================================================================================================================================

get_session_file_name() {
  local type="$1"             # e.g. audio, video, meta
  local start_iso="$2"        # e.g. 2025-06-27T19:30:00
  local end_iso="$3"          # e.g. 2025-06-27T21:30:00

  local start_stamp end_stamp
  start_stamp=$(iso_to_hhmmss "$start_iso")
  end_stamp=$(iso_to_hhmmss "$end_iso")

  echo "${type}_${start_stamp}_${end_stamp}"
}

# get_session_file_name "audio" "2025-06-27T23:30:00" "2025-06-27T01:30:00"

# ================================================================================================================================================
#     Detect active processes
# ================================================================================================================================================

# is_mic_running() {
#   pgrep -f arecord > /dev/null # For production 

#   # # For testing: 
#   # # return 0 # Pretend mic is running
#   # pgrep -f "mic_sim" > /dev/null
# }

# is_cam_running() {
#   pgrep -f libcamera-vid > /dev/null # For production 

#   # # For testing: 
#   # # return 1 # Pretend cam is NOT running
#   # pgrep -f "cam_sim" > /dev/null
# }

is_mic_running() {
  ps aux | grep -v grep | grep -q 'arecord.*\.wav'
}

is_cam_running() {
  ps aux | grep -v grep | grep -q 'libcamera-vid.*\.h264'
}

is_mic_stopped() { ! is_mic_running; }
is_cam_stopped() { ! is_cam_running; }

# ================================================================================================================================================
#     verify_recording_state()
#     Checks if mic and cam are both running or both idle.
#     Returns 0 if in sync, 1 if desynced.
# ================================================================================================================================================

verify_recording_state() {
  local mic_active cam_active

  is_mic_running; mic_active=$?
  is_cam_running; cam_active=$?

  if [[ "$mic_active" -eq 0 && "$cam_active" -eq 0 ]]; then
    echo "[✔] Both mic and cam are active — recording in sync."
    return 0
  elif [[ "$mic_active" -ne 0 && "$cam_active" -ne 0 ]]; then
    echo "[✔] Neither mic nor cam are active — clean slate."
    return 0
  else
    echo "[⚠] Desync detected:"
    [[ "$mic_active" -eq 0 ]] && echo "   → Mic is running" || echo "   → Mic is NOT running"
    [[ "$cam_active" -eq 0 ]] && echo "   → Cam is running" || echo "   → Cam is NOT running"
    return 1
  fi
}

# # Run test
# verify_recording_state
# echo "Exit Code: $?"

# ================================================================================================================================================
#     force_kill_recorders()
#     Terminates both mic and cam recorder processes forcefully.
#     Can be used after desync or during shutdown.
# ================================================================================================================================================

force_kill_recorders() {
  echo "[🛑] Forcing termination of recorder processes..."

  # Microphone
  if is_mic_running; then
    echo "→ Killing mic process..."
    pkill -f arecord         # PRODUCTION
    # pkill -f "mic_sim"     # Uncomment for TESTING
  else
    echo "→ Mic process not running."
  fi

  # Camera
  if is_cam_running; then
    echo "→ Killing cam process..."
    pkill -f libcamera-vid   # PRODUCTION
    # pkill -f "cam_sim"     # Uncomment for TESTING
  else
    echo "→ Cam process not running."
  fi

  echo "[🧼] Cleanup signal sent — validating shutdown..."
  wait_until is_mic_stopped 5
  wait_until is_cam_stopped 5
  echo "[✅] Shutdown complete — all recorders stopped cleanly."
}

# echo
# echo "===== Test: force_kill_recorders ====="

# # Pre-cleaning to avoid ghost processes
# pkill -f "FAKE_MIC" 2>/dev/null
# pkill -f "FAKE_CAM" 2>/dev/null
# sleep 0.5

# # Fake mic/cam processes (simulated with 'sleep 10' loops)
# # Simulate fake mic
# bash -c "exec -a mic_sim bash -c 'while true; do sleep 10; done'" &
# echo $! > fake_mic.pid

# # Simulate fake cam
# bash -c "exec -a cam_sim bash -c 'while true; do sleep 10; done'" &
# echo $! > fake_cam.pid
# sleep 0.5

# # Run checks
# verify_recording_state
# force_kill_recorders

# # Post-kill verification
# sleep 0.5
# if ! is_mic_running && ! is_cam_running; then
#   echo "[✅] All processes terminated successfully."
# else
#   echo "[❌] Some recorder processes still running!"
# fi

# # Clean up PID files
# rm -f fake_mic.pid fake_cam.pid

# ========================================================================================
# wait_while
#   - Repeatedly calls the given function.
#   - As soon as the function returns false (non-zero), it exits with success.
#   - If the timeout is exceeded while the function remains true, it returns 1.

#                 Total wait: 5s
#       ┌──────┬──────┬──────┬──────┬──────┐
# Time: 0s    0.5s   1.0s   1.5s   ...   5.0s
# Check:  ✓     ✓      ✓      ✓     ...   ⏰
# ========================================================================================

# Function Under Test
wait_while() {
  local func="$1"               # Name of function (event) to check
  local timeout="${2:-5}"       # Max total time we’re willing to wait (default: 5s) 
  local interval="${3:-0.5}"    # How often to check (default: 0.5s)
  local elapsed=0

  while (( $(echo "$elapsed < $timeout" | bc -l) )); do
    if ! "$func"; then
      echo "[⏳] Done waiting: $func is now false"
      return 0
    fi
    sleep "$interval"
    elapsed=$(echo "$elapsed + $interval" | bc)
  done

  echo "[⚠] Timeout: $func still returns true"
  return 1
}

# echo
# echo "=============================="
# echo "✅ TEST: wait_while() Utility"
# echo "=============================="

# # Cleanup any stale processes
# pkill -f mic_sim 2>/dev/null
# pkill -f cam_sim 2>/dev/null
# sleep 0.5

# # ---------------------------
# # Test 1: Wait for mic_sim to stop (normal case)
# # ---------------------------
# echo
# echo "[TEST 1] Wait while mic_sim is running..."

# # Start short-lived mic_sim (2 seconds)
# bash -c "exec -a mic_sim bash -c 'sleep 2'" &
# mic_pid=$!

# wait_while is_mic_running 5 0.5
# result=$?

# if [[ "$result" -eq 0 ]]; then
#   echo "[✅] mic_sim stopped within timeout."
# else
#   echo "[❌] mic_sim did not stop in time!"
# fi

# # ---------------------------
# #   Test 2: Timeout expected (cam_sim runs too long)
# #   Simulate a long-running cam_sim process (10s),
# #   and only give wait_while 3 seconds to wait.

# #   ⚠️ NOTE: Test 2 may produce false positives on macOS due to process race conditions.
# #      Reliable behavior will be tested on Raspberry Pi with real recorder binaries.
# # ---------------------------

# echo
# echo "[TEST 2] Expect timeout — cam_sim runs for 10s, timeout is 3s"

# pkill -f cam_sim 2>/dev/null
# sleep 0.2

# bash -c "exec -a cam_sim bash -c 'sleep 10'" &
# cam_pid=$!
# sleep 0.2

# echo "[TEST] Expect timeout — cam_sim runs for 10s, timeout is 3s"
# wait_while is_cam_running 3 0.5
# result=$?

# if [[ "$result" -eq 1 ]]; then
#   echo "[✅] Correctly timed out while waiting on cam_sim."
# else
#   echo "[❌] Unexpected success — cam_sim should still be running!"
# fi

# # Cleanup all
# kill "$cam_pid" 2>/dev/null
# wait "$cam_pid" 2>/dev/null
# sleep 0.2

# echo
# echo "✅ All wait_while tests completed."

# ⚠️ NOTE: Test 2 may produce false positives on macOS due to process race conditions.
# Reliable behavior will be tested on Raspberry Pi with real recorder binaries.

# ========================================================================================
# wait_until()
# ========================================================================================

wait_until() {
  local func="$1"
  local timeout="${2:-5}"
  local interval="${3:-0.5}"
  local elapsed=0

  while (( $(echo "$elapsed < $timeout" | bc -l) )); do
    if "$func"; then
      echo "[⏳] Condition met: $func returned true"
      return 0
    fi
    sleep "$interval"
    elapsed=$(echo "$elapsed + $interval" | bc)
  done

  echo "[⚠] Timeout: $func still returns false"
  return 1
}

# echo
# echo "===== TEST: wait_until + is_mic_stopped / is_cam_stopped ====="

# # --- Test 1: mic is stopped (nothing running) ---
# echo
# echo "[TEST 1] mic is already stopped"
# if wait_until is_mic_stopped 3 0.2; then
#   echo "✅ PASS: mic is stopped"
# else
#   echo "❌ FAIL: mic should be stopped"
# fi

# # --- Test 2: cam is already stopped (nothing running) ---
# echo
# echo "[TEST 2] cam is already stopped"
# if wait_until is_cam_stopped 3 0.2; then
#   echo "✅ PASS: cam is stopped"
# else
#   echo "❌ FAIL: cam should be stopped"
# fi

# # --- Simulate mic and cam ---
# echo
# echo "[SETUP] Launching mic_sim and cam_sim for tests..."
# bash -c "exec -a mic_sim bash -c 'sleep 5'" &
# bash -c "exec -a cam_sim bash -c 'sleep 5'" &
# sleep 0.3

# # --- Test 3: mic is running, should NOT pass is_mic_stopped immediately ---
# echo
# echo "[TEST 3] mic is running — expect timeout on is_mic_stopped"
# if wait_until is_mic_stopped 2 0.5; then
#   echo "❌ FAIL: mic should still be running"
# else
#   echo "✅ PASS: mic is running, so wait_until timed out as expected"
# fi

# # --- Test 4: cam is running — expect timeout on is_cam_stopped ---
# echo
# echo "[TEST 4] cam is running — expect timeout on is_cam_stopped"
# if wait_until is_cam_stopped 2 0.5; then
#   echo "❌ FAIL: cam should still be running"
# else
#   echo "✅ PASS: cam is running, so wait_until timed out as expected"
# fi

# # --- Cleanup ---
# sleep 5  # Let mic_sim and cam_sim exit naturally
# echo
# echo "[🧹] Waiting for processes to exit naturally..."
# wait_until is_mic_stopped 3
# wait_until is_cam_stopped 3
# echo "[✅] Done cleanup."

# echo
# echo "===== END TEST ====="

# ========================================================================================
# compute_chunk_duration()
# ========================================================================================

compute_chunk_duration() {
  local start_iso="$1"
  local session_end_iso="$2"
  local requested_duration="${3:-$CHUNK_DURATION}"

  if [[ -z "$start_iso" || -z "$session_end_iso" ]]; then
    echo "ERROR: compute_chunk_duration requires start_iso and session_end_iso" >&2
    return 1
  fi

  local start_epoch session_end_epoch end_epoch actual_duration chunk_end_iso

  start_epoch=$(iso_to_epoch "$start_iso")
  session_end_epoch=$(iso_to_epoch "$session_end_iso")
  end_epoch=$(( start_epoch + requested_duration ))

  # Clamp to session end
  if (( end_epoch > session_end_epoch )); then
    end_epoch="$session_end_epoch"
  fi

  actual_duration=$(( end_epoch - start_epoch ))

  if (( actual_duration <= 0 )); then
    echo "SKIP"
    return 0
  fi

  chunk_end_iso=$(epoch_to_iso "$end_epoch")
  echo "$actual_duration $chunk_end_iso"
}

# # ===== Test Cases =====
# echo
# echo "===== TEST: compute_chunk_duration ====="
# session_end_iso="2025-07-01T23:00:00"

# declare -a test_cases=(
#   "2025-07-01T22:30:00 1800 EXPECT_FULL"
#   "2025-07-01T22:50:00 1800 EXPECT_TRUNCATE"
#   "2025-07-01T23:00:00 1800 EXPECT_SKIP"
#   "2025-07-01T23:10:00 1800 EXPECT_SKIP"
# )

# for line in "${test_cases[@]}"; do
#   start_iso=$(echo "$line" | awk '{print $1}')
#   requested_duration=$(echo "$line" | awk '{print $2}')
#   label=$(echo "$line" | awk '{print $3}')

#   echo
#   echo "→ TEST CASE: $label"
#   echo "   Start ISO         : $start_iso"
#   echo "   Session End ISO   : $session_end_iso"
#   echo "   Requested Duration: ${requested_duration}s"

#   read -r result end_iso <<< "$(compute_chunk_duration "$start_iso" "$session_end_iso" "$requested_duration")"

#   if [[ "$result" == "SKIP" ]]; then
#     echo "   Result            : ⏹ SKIP — no time remaining"
#   else
#     echo "   Result            : ✅ $result sec → Ends at: $end_iso"
#   fi
# done

# echo
# echo "===== Done ====="

# ========================================================================================
# generate_chunk_filename()
# ========================================================================================

generate_chunk_filename() {
  local type="$1"
  local start_iso="$2"
  local end_iso="$3"

  if [[ -z "$type" || -z "$start_iso" || -z "$end_iso" ]]; then
    echo "ERROR: generate_chunk_filename requires type, start_iso, end_iso" >&2
    return 1
  fi

  local filename
  filename=$(get_session_file_name "$type" "$start_iso" "$end_iso")

  # Future logging stub
  # log_chunk_start "$type" "$start_iso" "$duration" "$end_iso" "$filename"

  echo "$filename"
}

# # ===== Test Cases =====
# echo
# echo "===== TEST: generate_chunk_filename ====="

# declare -a test_cases=(
#   "audio 2025-07-01T22:30:00 2025-07-01T22:45:00"
#   "video 2025-07-01T23:00:00 2025-07-02T00:00:00"
#   "audio 2025-07-01T23:50:00 2025-07-02T00:05:00"
# )

# for line in "${test_cases[@]}"; do
#   type=$(echo "$line" | awk '{print $1}')
#   start_iso=$(echo "$line" | awk '{print $2}')
#   end_iso=$(echo "$line" | awk '{print $3}')

#   echo
#   echo "→ TEST CASE"
#   echo "   Type      : $type"
#   echo "   Start ISO : $start_iso"
#   echo "   End ISO   : $end_iso"

#   filename=$(generate_chunk_filename "$type" "$start_iso" "$end_iso")
#   echo "   → Filename : $filename"
# done

# echo
# echo "===== Done ====="

# ========================================================================================
# start_mic()
# ========================================================================================

start_mic() {
  local folder="$1"
  local filename="$2"
  local duration="$3"

  if [[ -z "$folder" || -z "$filename" || -z "$duration" ]]; then
    echo "ERROR: start_mic requires folder, filename, and duration" >&2
    return 1
  fi

  local filepath="${folder}/${filename}.wav"
  echo "[🎤] Starting mic → $filepath (duration: ${duration}s)"
  
  arecord \
    -D "$AUDIO_DEVICE" \
    -f S32_LE \
    -c 2 \
    -r "$AUDIO_SAMPLE_RATE" \
    -d "$duration" \
    "$filepath" &

  local pid=$!
  echo "[🎤] Mic recording PID: $pid"
}

# ========================================================================================
# start_cam()
# ========================================================================================

start_cam() {
  local folder="$1"
  local filename="$2"
  local duration="$3"

  if [[ -z "$folder" || -z "$filename" || -z "$duration" ]]; then
    echo "ERROR: start_cam requires folder, filename, and duration" >&2
    return 1
  fi

  local filepath="${folder}/${filename}.h264"
  echo "[🎥] Starting cam → $filepath (duration: ${duration}s)"
  libcamera-vid \
    --width "$VIDEO_WIDTH" \
    --height "$VIDEO_HEIGHT" \
    --framerate "$VIDEO_FPS" \
    -t "$((duration * 1000))" \
    --nopreview \
    -o "$filepath" &

  local pid=$!
  echo "[🎥] Cam recording PID: $pid"
}

# ========================================================================================
# start_recorders()
# ========================================================================================

start_recorders() {
  local folder="$1"
  local start_iso="$2"
  local session_end_iso="$3"
  local chunk_duration="${4:-$CHUNK_DURATION}"

  if [[ -z "$folder" || -z "$start_iso" || -z "$session_end_iso" ]]; then
    echo "ERROR: start_recorders requires folder, start_iso, and session_end_iso" >&2
    return 1
  fi

  # Get duration and chunk end time
  read -r duration end_iso <<< "$(compute_chunk_duration "$start_iso" "$session_end_iso" "$chunk_duration")"
  if [[ "$duration" == "SKIP" ]]; then
    echo "⏹ Skipping: no time left in session window"
    return 0
  fi

  # Generate personalized filenames
  local audio_filename video_filename
  audio_filename=$(generate_chunk_filename "audio" "$start_iso" "$end_iso")
  video_filename=$(generate_chunk_filename "video" "$start_iso" "$end_iso")

  # Launch both devices
  start_mic "$folder" "$audio_filename" "$duration"
  start_cam "$folder" "$video_filename" "$duration"
}






