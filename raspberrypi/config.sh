# config.sh

# Session Start Time
START_HOUR=19
START_MINUTE=30

# Total Session Duration
DURATION_HOUR=10
DURATION_MINUTE=0
DURATION_SECOND=0

# Per-Chunk File Duration
FILE_DURATION_HOUR=2
FILE_DURATION_MINUTE=0
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
STATE_CHECK_INTERVAL=10

# Derived Values (in seconds)
SESSION_TOTAL_DURATION=$(( DURATION_HOUR * 3600 + DURATION_MINUTE * 60 + DURATION_SECOND ))
CHUNK_DURATION=$(( FILE_DURATION_HOUR * 3600 + FILE_DURATION_MINUTE * 60 + FILE_DURATION_SECOND ))