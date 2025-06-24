#ifndef RTC_SCHEDULER_H
#define RTC_SCHEDULER_H

#include "Config.h"   
#include <RTClib.h>

namespace RtcScheduler {
  struct ActiveWindow {
    DateTime start;
    DateTime end;
  };

  extern RTC_DS3231 rtc;

  void begin();

  ActiveWindow getActiveWindow(
    DateTime now = rtc.now(),        
    int startHour = ACTIVE_START_HOUR,
    int startMinute = ACTIVE_START_MINUTE,
    int durationHour = ACTIVE_DURATION_HOUR,
    int durationMinute = ACTIVE_DURATION_MINUTE
  );

  bool isActiveWindow(const ActiveWindow& window, DateTime nowOverride = rtc.now());
}

#endif