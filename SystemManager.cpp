#include <Arduino.h>
#include "SystemManager.h"
#include "SpeakerController.h"
#include "debug_utils.h"

namespace {
  SystemState currentState = SystemState::IDLE;
}

void SystemManager::begin() {
  SpeakerController::begin();
  setState(SystemState::IDLE);
}

void SystemManager::setState(SystemState state) {
  if (state == currentState) return;

  currentState = state;

  if (DEBUG_MODE_SYSTEM) {
    debugPrint("[System] Transition to ");
    debugPrintln(state == SystemState::ACTIVE ? "ACTIVE" : "IDLE");
  }

  if (state == SystemState::ACTIVE) {
    SpeakerController::start();
  } else {
    SpeakerController::stop();
  }
}

SystemState SystemManager::getState() {
  return currentState;
}

void SystemManager::loop() {
  SpeakerController::loop();
}

void SystemManager::printStatus() {
  debugPrint("[System] State: ");
  debugPrintln(currentState == SystemState::ACTIVE ? "ACTIVE" : "IDLE");
  SpeakerController::printStatus();
}