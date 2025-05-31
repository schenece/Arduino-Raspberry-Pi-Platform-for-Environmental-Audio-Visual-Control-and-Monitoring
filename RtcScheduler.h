#ifndef RTC_SCHEDULER_H
#define RTC_SCHEDULER_H

#include <RTClib.h>

namespace RtcScheduler {
  void begin();
  bool isActiveWindow();
}

#endif