#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <Arduino.h>
#include <RTClib.h>
#include "SpeakerController.h"

enum class SystemState { IDLE, ACTIVE };

namespace SystemManager {
  void begin(RTC_DS3231* rtcPtr);        // Pass pointer to external RTC instance
  void loop();                           // Called in main loop to update state
  void setState(SystemState newState);   // Force a state transition
  SystemState getState();
  void printStatus();                    // Optional debug output
}

#endif