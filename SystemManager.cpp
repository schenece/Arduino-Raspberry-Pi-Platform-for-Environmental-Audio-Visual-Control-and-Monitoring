#include <Arduino.h>
#include "SystemManager.h"
#include "SpeakerController.h"
#include "debug_utils.h"

// ✅ RTC-specific headers
#include <RTClib.h>

namespace {
  SystemState currentState = SystemState::IDLE;

  // ✅ RTC instance and target schedule
  RTC_DS3231 rtc;
  DateTime activeStart;
  DateTime activeEnd;
}

void SystemManager::begin(RTC_DS3231 *rtcPtr) {
  // ✅ Initialize speaker as before
  SpeakerController::begin();
  setState(SystemState::IDLE);

  // ✅ Initialize RTC and schedule
  if (!rtc.begin()) {
    debugPrintln("[RTC] Failed to initialize.");
    return;
  }

  if (rtc.lostPower()) {
    debugPrintln("[RTC] Lost power. Setting default time.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  DateTime now = rtc.now();
  activeStart = DateTime(now.year(), now.month(), now.day(), 23, 05, 0);
  activeEnd   = activeStart + TimeSpan(0, 0, 3, 0);  // 3 min later

  debugPrint(F("[RTC] ACTIVE window: "));
  debugPrint(activeStart.timestamp());
  debugPrint(" → ");
  debugPrintln(activeEnd.timestamp());
}

void SystemManager::loop() {
  // ✅ Regular speaker updates
  SpeakerController::loop();

  // ✅ RTC-based state check
  DateTime now = rtc.now();
  if (now >= activeStart && now < activeEnd) {
    setState(SystemState::ACTIVE);
  } else {
    setState(SystemState::IDLE);
  }
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

void SystemManager::printStatus() {
  debugPrint("[System] State: ");
  debugPrintln(currentState == SystemState::ACTIVE ? "ACTIVE" : "IDLE");
  SpeakerController::printStatus();
}