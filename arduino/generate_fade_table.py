import sys

# === Constants ===
AC_HZ = 60
AC_CYCLE_MS = 1000 / AC_HZ
delayForBrightestUs = 1800
delayForDarkestUs = 6500

# === Track Durations (ms) ===
track_durations_ms = [
    8140, 8336, 3604, 5695, 3893, 4806, 4435, 5935,
    5501, 5195, 5923, 6021, 4748, 10399, 7256, 7059,
    6291, 9327, 7506, 4606, 4606, 6264, 9617
]

# === Argument Handling ===
if len(sys.argv) < 2:
    print("Usage: python3 generate_fade_table.py <brightnessStepUs>")
    sys.exit(1)

try:
    brightnessStepUs = int(sys.argv[1])
except ValueError:
    print("Error: brightnessStepUs must be an integer.")
    sys.exit(1)

totalSteps = (delayForDarkestUs - delayForBrightestUs) / brightnessStepUs

print("Track #, Duration (ms), brightnessStepUs, totalSteps, fadeCycles, cyclesPerStep")

for i, duration in enumerate(track_durations_ms, start=1):
    fadeTime = duration / 2
    fadeCycles = fadeTime / AC_CYCLE_MS
    cyclesPerStep = max(1, round(fadeCycles / totalSteps))
    print(f"{i}, {duration}, {brightnessStepUs}, {int(totalSteps)}, {int(fadeCycles)}, {cyclesPerStep}")