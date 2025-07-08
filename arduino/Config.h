#ifndef CONFIG_H
#define CONFIG_H

#define SD_CS_PIN 10

// ====== Debug Flags ======
#define DEBUG_MODE true
#define DEBUG_MODE_SYSTEM true
#define DEBUG_MODE_SPEAKER true

// ====== DFPlayer Settings ======
#define DFPLAYER_SERIAL_BAUD 9600
#define DFPLAYER_BUSY_PIN 8
#define DEFAULT_VOLUME 18

// ====== Active Window Settings ======
#define ACTIVE_START_HOUR     15
#define ACTIVE_START_MINUTE   50
#define ACTIVE_DURATION_HOUR  2
#define ACTIVE_DURATION_MINUTE 0

#endif