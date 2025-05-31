#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <Arduino.h>
#include <RTClib.h>
#include "SpeakerController.h"

enum class SystemState { IDLE, ACTIVE };

namespace SystemManager {
  void begin();        // Pass pointer to external RTC instance
  void loop();                           // Called in main loop to update state
  void setState(SystemState newState);   // Force a state transition
  SystemState getState();
  void printStatus();                    // Optional debug output

  void suppressRTCFor(unsigned long durationMs); 
}

#endif