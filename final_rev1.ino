#include "Config.h"
#include "SystemManager.h"
#include "CommandRouter.h"
#include "debug_utils.h"
#include "RtcScheduler.h"  

void setup() {
  Serial.begin(115200);
  delay(1000);  // Allow USB to settle
  while (!Serial && millis() < 2000);

  debugPrintln("[Setup] Initializing system...");

  RtcScheduler::begin();     
  SystemManager::begin();    
  debugPrintln("[Setup] Done.");
}

void loop() {
  CommandRouter::handleSerial();  // Serial command interface
  SystemManager::loop();          // System behavior
}