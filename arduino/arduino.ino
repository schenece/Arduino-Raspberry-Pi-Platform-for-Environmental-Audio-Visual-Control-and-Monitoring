#include "Config.h"
#include <Adafruit_SleepyDog.h>
#include "SystemManager.h"
#include "CommandRouter.h"
#include "debug_utils.h"
#include "RtcScheduler.h"  

void setup() {
  Serial.begin(115200);
  delay(1000);  // Allow USB to settle
  while (!Serial && millis() < 2000);

  debugPrintln("[Setup] Initializing system...");

  RtcScheduler::begin(); // Do NOT bundle this into SystemManager::begin()
  SystemManager::begin();     

  int wdtTimeoutMS = Watchdog.enable(8000, false);  // 8 seconds timeout
  Serial.print("[WDT] Timeout set to ");
  Serial.print(wdtTimeoutMS);
  Serial.println(" ms");

  debugPrintln("[Setup] Done.");
}

void loop() {
  CommandRouter::handleSerial();  // Serial command interface
  SystemManager::loop();          // System behavior
  
  static unsigned long lastWdt = 0;
  if (millis() - lastWdt > 6000) {
    Watchdog.reset();
    lastWdt = millis();
  }
}