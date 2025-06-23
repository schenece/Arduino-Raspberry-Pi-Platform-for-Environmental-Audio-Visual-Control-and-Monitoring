#ifndef SD_FAULT_LOGGER_H
#define SD_FAULT_LOGGER_H

#include <Arduino.h>

namespace SdFaultLogger {
  bool begin();                        // ✅ Return type changed to match .cpp
  void log(const String& message, bool critical = false);  // ✅ Updated to accept a 'critical' flag
  void dumpLogs();                    // Optional
  void clear();                       // ✅ Renamed from clearLogs to clear
}

#endif // SD_FAULT_LOGGER_H