#include <Arduino.h>
#include "SystemManager.h"
#include "SpeakerController.h"
#include "LightController.h"
#include "RtcScheduler.h"
#include "debug_utils.h"
#include "SdFaultLogger.h"  
#include "FadeTable.h"
#include "RelayController.h"

#include <Adafruit_SleepyDog.h>

/*
 * ========================================================================
 *  SystemManager.cpp
 *  ----------------------------------------------------------------------
 *  Orchestrates overall behavior based on system state (IDLE or ACTIVE).
 *
 *  - Starts/stops speaker and light fades based on RTC-defined windows
 *  - Monitors speaker state for automatic replay of audio
 *  - Coordinates per-track visual sync using fade table values
 *
 *  🔧 Track sync logic:
 *    - On ACTIVE state entry or new track start:
 *      → Look up fade parameters from fadeTable
 *      → Call LightController::start(...) with track-specific values
 *
 *  Dependencies:
 *    - RtcScheduler: determines active time windows
 *    - SpeakerController: manages DFPlayer state
 *    - LightController: handles fade sync
 *    - FadeTable: stores manually tuned brightness & timing per track
 * ========================================================================
 */

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
  RelayController::begin();
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
    if (currentTrack != lastSyncedTrack && currentTrack > 0 && currentTrack <= NUM_TRACKS) {
      lastSyncedTrack = currentTrack;

      const FadeParams& params = fadeTable[currentTrack - 1];

      // Skip missing track (e.g. track 13 is manually set to {0,0})
      if (params.brightnessStepUs == 0 || params.cyclesPerStep == 0) {
        Serial.println("[System] Track has no fade params. Skipping light sync.");
      } else {
        LightController::start(currentTrack, params.brightnessStepUs, params.cyclesPerStep);
        Serial.print("[System] Syncing Track ");
        Serial.print(currentTrack);
        Serial.print(" → Fade Pattern ");
        Serial.print(currentTrack); // ✅ Human-friendly: pattern = track number
        Serial.println();

        Serial.print("[System] Light sync triggered with fade params → ");
        Serial.print("brightnessStepUs = ");
        Serial.print(params.brightnessStepUs);
        Serial.print(", cyclesPerStep = ");
        Serial.println(params.cyclesPerStep);
      }
    }

    SpeakerState currentState = SpeakerController::getState();
    if (prevSpeakerState == SpeakerState::PLAYING && currentState != SpeakerState::PLAYING) {
      RelayController::turnOff();
      LightController::stop();
      Serial.println("[System] Track ended → amp off, lights off.");

      delay(3000);  // Optional: quiet time between tracks
      RelayController::turnOn();
      delay(100);   // Allow relay/amp to settle
      SpeakerController::start();
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
    RelayController::turnOn();
    SpeakerController::start();
    // 🔧 REMOVED: LightController::start() — now triggered by speaker loop
  } else {
    RelayController::turnOff();
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