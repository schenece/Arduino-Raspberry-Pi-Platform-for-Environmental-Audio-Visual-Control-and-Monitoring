#include <Arduino.h>
#include "SystemManager.h"
#include "SpeakerController.h"
#include "LightController.h"
#include "RtcScheduler.h"
#include "debug_utils.h"

namespace {
  SystemState currentState = SystemState::IDLE;
  unsigned long manualOverrideUntil = 0;
}

void SystemManager::begin() {
  SpeakerController::begin();
  LightController::begin();
  RtcScheduler::begin();
  setState(SystemState::IDLE);
}

void SystemManager::loop() {
  SpeakerController::loop();
  LightController::update();

  bool useRTC = (millis() > manualOverrideUntil);

  if (millis() > manualOverrideUntil) {
    if (RtcScheduler::isActiveWindow()) {
      setState(SystemState::ACTIVE);
    } else {
      setState(SystemState::IDLE);
    }
  }

  // Sync lights with speaker only in ACTIVE
  if (currentState == SystemState::ACTIVE) {
    if (SpeakerController::getState() == SpeakerState::PLAYING) {
      LightController::start();
    } else {
      LightController::stop();
    }
  }
}

void SystemManager::setState(SystemState newState) {
  if (newState == currentState) return;

  currentState = newState;

  if (DEBUG_MODE_SYSTEM) {
    debugPrint("[System] Transition to ");
    debugPrintln(newState == SystemState::ACTIVE ? "ACTIVE" : "IDLE");
  }

  if (newState == SystemState::ACTIVE) {
    SpeakerController::start();
    LightController::start();
  } else {
    SpeakerController::stop();
    LightController::stop();
  }
}

SystemState SystemManager::getState() {
  return currentState;
}

void SystemManager::printStatus() {
  debugPrint("[System] State: ");
  debugPrintln(currentState == SystemState::ACTIVE ? "ACTIVE" : "IDLE");
  SpeakerController::printStatus();
  LightController::printCurrentState();
}

void SystemManager::suppressRTCFor(unsigned long durationMs) {
  manualOverrideUntil = millis() + durationMs;
}