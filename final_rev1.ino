#include "Config.h"
#include "SystemManager.h"
#include "CommandRouter.h"
#include "debug_utils.h"
#include <RTClib.h>  // ✅ Include RTC library

RTC_DS3231 rtc;      // ✅ RTC instance

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000);

  debugPrintln("[Setup] Initializing system...");

  if (!rtc.begin()) {
    debugPrintln("[RTC] Failed to initialize RTC!");
  } else {
    if (rtc.lostPower()) {
      debugPrintln("[RTC] RTC lost power, setting time to compile time.");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  }

  SystemManager::begin(&rtc);  // ✅ Pass RTC to SystemManager
  debugPrintln("[Setup] Done.");
}

void loop() {
  CommandRouter::handleSerial();
  SystemManager::loop();
}