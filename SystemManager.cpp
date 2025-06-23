#include <Arduino.h>
#include "SystemManager.h"
#include "SpeakerController.h"
#include "LightController.h"
#include "RtcScheduler.h"
#include "debug_utils.h"
#include "SdFaultLogger.h"  

namespace {
  SystemState currentState = SystemState::IDLE;
  unsigned long manualOverrideUntil = 0;
}

void SystemManager::begin() {
  bool sdOk = SdFaultLogger::begin();             // ✅ Try to init SD

  if (sdOk) SdFaultLogger::log("System booted", true);  // ✅ Log boot success

  SpeakerController::begin();
  LightController::begin();
  RtcScheduler::begin();
  setState(SystemState::IDLE);
}

void SystemManager::loop() {
  SpeakerController::loop();
  LightController::update();

  // === Periodic RTC-based state check ===
  static unsigned long lastCheck = 0;
  const unsigned long interval = 5000;  // every 5 seconds

  if (millis() > manualOverrideUntil && millis() - lastCheck > interval) {
    lastCheck = millis();

    DateTime now = RtcScheduler::rtc.now();
    auto win = RtcScheduler::getActiveWindow(
      now.year(), now.month(), now.day(),
      ACTIVE_START_HOUR, ACTIVE_START_MINUTE,
      ACTIVE_DURATION_HOUR, ACTIVE_DURATION_MINUTE
    );

    if (RtcScheduler::isActiveWindow(win, now)) {
      setState(SystemState::ACTIVE);
    } else {
      setState(SystemState::IDLE);
    }
  }

  // === Sync lights with speaker ONLY in ACTIVE ===
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