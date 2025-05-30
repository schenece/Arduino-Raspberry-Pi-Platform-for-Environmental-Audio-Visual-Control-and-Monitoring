#include "Config.h"
#include "SystemManager.h"
#include "CommandRouter.h"
#include "debug_utils.h"  // ✅ This was missing

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000);

  debugPrintln("[Setup] Initializing system...");
  SystemManager::begin();
  debugPrintln("[Setup] Done.");
}

void loop() {
  CommandRouter::handleSerial();
  SystemManager::loop();
}