#include <Arduino.h>
#include "CommandRouter.h"
#include "SystemManager.h"
#include "SpeakerController.h"
#include "debug_utils.h"

void CommandRouter::handleSerial() {
  if (!Serial.available()) return;

  String input = Serial.readStringUntil('\n');
  input.trim();
  input.toLowerCase();

  // Serial.print("[CommandRouter] Received: ");
  // Serial.println(input);

  if (input == "night") {
    SystemManager::setState(SystemState::ACTIVE);
    SystemManager::suppressRTCFor(30000);
  } else if (input == "day") {
    SystemManager::setState(SystemState::IDLE);
    SystemManager::suppressRTCFor(30000);
  } else if (input == "status") {
    SystemManager::printStatus();
  }

  // Only allow speaker control if system is in ACTIVE mode
  else if (SystemManager::getState() == SystemState::ACTIVE) {
    if (input == "pause") {
      SpeakerController::pause();
    } else if (input == "resume") {
      SpeakerController::resume();
    } else if (input == "stop") {
      SpeakerController::stop();
    } else if (input == "next") {
      SpeakerController::next();
    } else if (input == "prev") {
      SpeakerController::prev();
    } else if (input.startsWith("vol")) {
      int v = input.substring(3).toInt();
      SpeakerController::setVolume(v);
    } else {
      Serial.println("[Command] Unknown command.");
    }
  }

  else {
    Serial.println("[Command] Ignored: not allowed in IDLE mode.");
  }
}