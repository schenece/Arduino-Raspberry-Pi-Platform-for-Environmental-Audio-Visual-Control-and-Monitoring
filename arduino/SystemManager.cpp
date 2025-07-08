#include <Arduino.h>
#include "SystemManager.h"
#include "SpeakerController.h"
#include "LightController.h"
#include "RtcScheduler.h"
#include "debug_utils.h"
#include "SdFaultLogger.h"  
#include "FadeTable.h"

#include <Adafruit_SleepyDog.h>

namespace {
  SystemState currentState = SystemState::IDLE;
  unsigned long manualOverrideUntil = 0;

  // 🔧 CHANGED: Speaker-sync tracking
  int lastSyncedTrack = -1;
  SpeakerState prevSpeakerState = SpeakerState::STOPPED;
}

void SystemManager::begin() {
  bool sdOk = SdFaultLogger::begin();             // ✅ Try to init SD
  if (sdOk) SdFaultLogger::log("System booted", true);  // ✅ Log boot success

  // RtcScheduler::begin();
  SpeakerController::begin();
  LightController::begin();
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
      now,
      ACTIVE_START_HOUR, ACTIVE_START_MINUTE,
      ACTIVE_DURATION_HOUR, ACTIVE_DURATION_MINUTE
    );

    if (RtcScheduler::isActiveWindow(win, now)) {
      setState(SystemState::ACTIVE);
    } else {
      setState(SystemState::IDLE);
    }
  }

  static bool printedOnce = false;

  if (!printedOnce && millis() > 7000) {  // wait until RTC check has run at least once
    printStatus();
    printedOnce = true;
  }

  // === 🔧 CHANGED: Sync lights with speaker only in ACTIVE
  if (currentState == SystemState::ACTIVE) {
    int currentTrack = SpeakerController::getLastTrack();
    if (currentTrack != lastSyncedTrack && currentTrack > 0) {
      lastSyncedTrack = currentTrack;
      int fadeDurationMs = trackDurationsMs[currentTrack - 1] / 2;
      LightController::start(currentTrack, fadeDurationMs);
    }

    SpeakerState currentState = SpeakerController::getState();
    if (prevSpeakerState == SpeakerState::PLAYING && currentState != SpeakerState::PLAYING) {
      LightController::stop();
      Serial.println("[Light] Automatically stopped after track ended.");
    }
    prevSpeakerState = currentState;
  } else {
    // 🔧 CHANGED: always stop in IDLE
    LightController::stop();
    lastSyncedTrack = -1;
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
    // 🔧 REMOVED: LightController::start() — now triggered by speaker loop
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

// void SystemManager::heartbeat() {

// }