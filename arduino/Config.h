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
#define DEFAULT_VOLUME 30

// ====== RTC Settings ======
const int ACTIVE_START_HOUR = 19;
const int ACTIVE_START_MINUTE = 30;

const int ACTIVE_DURATION_HOUR = 10;
const int ACTIVE_DURATION_MINUTE = 0;

const int IDLE_DURATION_HOUR = 14;
const int IDLE_DURATION_MINUTE = 0;

#endif