#include "RtcScheduler.h"
#include "debug_utils.h"

namespace RtcScheduler {
  RTC_DS3231 rtc;
}

void RtcScheduler::begin() {
  if (!rtc.begin()) {
    debugPrintln("[RTC] Failed to initialize.");
    return;
  }

  DateTime now = rtc.now();

  if (rtc.lostPower() || now.year() < 2025) {
    debugPrintln("[RTC] Lost power or invalid time. Manual reset required.");
    return;
  }

  debugPrint(F("[RTC] Current time: "));
  debugPrintln(now.timestamp());
}

RtcScheduler::ActiveWindow RtcScheduler::getActiveWindow(
  DateTime now,
  int startHour, int startMinute,
  int durationHour, int durationMinute
) {
  TimeSpan duration((durationHour * 3600L) + (durationMinute * 60L));
  DateTime todayStart(now.year(), now.month(), now.day(), startHour, startMinute, 0);
  DateTime todayEnd = todayStart + duration;

  DateTime start;

  if (now >= todayStart) {
    start = todayStart;
  } else {
    // Special case: it’s morning (e.g., 2am), so maybe yesterday’s window is still open
    DateTime yesterday = now - TimeSpan(86400);
    DateTime yesterdayStart(yesterday.year(), yesterday.month(), yesterday.day(), startHour, startMinute, 0);
    DateTime yesterdayEnd = yesterdayStart + duration;

    if (now < yesterdayEnd) {
      start = yesterdayStart;
    } else {
      start = todayStart;  // Not in yesterday's window, use today
    }
  }

  DateTime end = start + duration;
  return { start, end };
}

bool RtcScheduler::isActiveWindow(const ActiveWindow& window, DateTime nowOverride) {
  return (nowOverride >= window.start && nowOverride < window.end);
  // return true; // For test only 
}