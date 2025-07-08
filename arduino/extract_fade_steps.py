import os
from pydub import AudioSegment

# === Config ===
FOLDER = "speakerfiles"
MIN_DELAY = 2000  # µs
MAX_DELAY = 6000  # µs
AC_HZ = 50
CYCLE_US = 1_000_000 / AC_HZ

# === Output Table Header ===
print("Track #,Filename,Duration (s),Duration (ms),Fade Step (µs)")

# === Loop Through MP3 Files ===
for filename in sorted(os.listdir(FOLDER)):
    if filename.lower().endswith(".mp3"):
        path = os.path.join(FOLDER, filename)
        try:
            audio = AudioSegment.from_mp3(path)
            dur_ms = len(audio)
            cycles = (dur_ms * 1000) / CYCLE_US
            fade_step = (MAX_DELAY - MIN_DELAY) / cycles
            track_num = int(filename.replace(".mp3", "").lstrip("0") or "0")
            print(f"{track_num},{filename},{dur_ms/1000:.2f},{dur_ms},{fade_step:.2f}")
        except Exception as e:
            print(f"-,{filename},ERROR,ERROR,{str(e)}")