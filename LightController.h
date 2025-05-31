#ifndef LIGHT_CONTROLLER_H
#define LIGHT_CONTROLLER_H

#include <Arduino.h>

namespace LightController {

  enum class LampState {
    IDLE,
    FADE_IN,
    FADE_OUT,
    OFF_INTERVAL,
    STOPPED
  };

  void begin();
  void start();
  void stop();
  void update();
  void setPattern(int index);
  void printCurrentState();
  void onZeroCross();  // Called by ISR
}

#endif  // LIGHT_CONTROLLER_H