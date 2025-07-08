#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

#include <Arduino.h>

namespace RelayController {
  const int RELAY_PIN = 5;

  inline void begin() {
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);  // Start OFF
  }

  inline void turnOn() {
    digitalWrite(RELAY_PIN, HIGH);  // Relay ON → Connect amplifier
  }

  inline void turnOff() {
    digitalWrite(RELAY_PIN, LOW);   // Relay OFF → Disconnect amplifier
  }
}

#endif