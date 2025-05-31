#include "RtcScheduler.h"
#include "debug_utils.h"

// ==== USER-CONFIGURABLE SCHEDULE ====
// Edit these values to change the system's automatic ACTIVE window
const int ACTIVE_HOUR = 23;                 // Start time: hour (24h format)
const int ACTIVE_MINUTE = 5;                // Start time: minute
const int ACTIVE_DURATION_MINUTES = 3;      // ACTIVE window duration (minutes)

namespace {
  RTC_DS3231 rtc;
  DateTime activeStart;
  DateTime activeEnd;
}

void RtcScheduler::begin() {
  if (!rtc.begin()) {
    debugPrintln("[RTC] Failed to initialize.");
    return;
  }

  if (rtc.lostPower()) {
    debugPrintln("[RTC] Lost power. Setting default time.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  DateTime now = rtc.now();
  activeStart = DateTime(now.year(), now.month(), now.day(), ACTIVE_HOUR, ACTIVE_MINUTE, 0);
  activeEnd   = activeStart + TimeSpan(0, 0, ACTIVE_DURATION_MINUTES, 0);

  debugPrint(F("[RTC] ACTIVE window: "));
  debugPrint(activeStart.timestamp());
  debugPrint(" → ");
  debugPrintln(activeEnd.timestamp());
}

bool RtcScheduler::isActiveWindow() {
  // Temporarily overridden for testing purposes
  // DateTime now = rtc.now();
  // return (now >= activeStart && now < activeEnd);
  return true;
}