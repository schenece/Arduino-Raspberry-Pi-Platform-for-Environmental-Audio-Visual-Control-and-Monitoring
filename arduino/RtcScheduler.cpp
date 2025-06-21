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
  int year, int month, int day,
  int startHour, int startMinute,
  int durationHour, int durationMinute
) {
  DateTime start(year, month, day, startHour, startMinute, 0);
  TimeSpan duration((durationHour * 3600L) + (durationMinute * 60L));
  DateTime end = start + duration;

  debugPrint(F("[RTC] ACTIVE window: "));
  debugPrint(start.timestamp());
  debugPrint(" → ");
  debugPrintln(end.timestamp());

  return { start, end };
}

bool RtcScheduler::isActiveWindow(const ActiveWindow& window, DateTime nowOverride) {
  return (nowOverride >= window.start && nowOverride < window.end);
}