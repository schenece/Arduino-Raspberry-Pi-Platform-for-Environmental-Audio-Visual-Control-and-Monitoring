#ifndef RTC_SCHEDULER_H
#define RTC_SCHEDULER_H

#include <RTClib.h>

namespace RtcScheduler {
  struct ActiveWindow {
    DateTime start;
    DateTime end;
  };

  extern RTC_DS3231 rtc;

  void begin();

  ActiveWindow getActiveWindow(
    int year, int month, int day,
    int startHour, int startMinute,
    int durationHour, int durationMinute
  );

  bool isActiveWindow(const ActiveWindow& window, DateTime nowOverride = rtc.now());
}

#endif