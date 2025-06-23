#include "SdFaultLogger.h"
#include "debug_utils.h"
#include <SPI.h>
#include <SD.h>                   // ✅ Required for SD.open()
#include "RtcScheduler.h"

#define LOG_FILENAME "LOG.TXT"

using namespace RtcScheduler;

namespace {
  File logFile;
}

bool SdFaultLogger::begin() {
  if (!SD.begin(SD_CS_PIN)) {
    debugPrintln("[Logger] SD init failed!");
    return false;
  }

  debugPrintln("[Logger] SD card initialized.");
  
  // File testFile = SD.open("test.txt", FILE_WRITE);
  // if (testFile) {
  //   testFile.println("Hello from SdFaultLogger test.");
  //   testFile.close();
  //   debugPrintln("[Logger] test.txt written successfully.");
  // } else {
  //   debugPrintln("[Logger] Failed to write test.txt");
  // }

  return true;
}

void SdFaultLogger::log(const String& message, bool critical) {
  #if !TEST_MODE
    if (!critical) return;  // ✅ Skip non-critical logs unless in test mode
  #endif

  debugPrintln("[Logger] Opening log file...");

  logFile = SD.open(LOG_FILENAME, FILE_WRITE);
  if (!logFile) {
    debugPrintln("[Logger] Failed to open log file for writing.");
    return;
  }

  debugPrintln("[Logger] Log file opened successfully.");
  debugPrintln("[Logger] Querying RTC...");

  DateTime now = RtcScheduler::rtc.now();  // Use RTC timestamp
  if (!now.isValid()) {
    logFile.print("INVALID TIME");  // Fallback label
  } else {
    logFile.print(now.timestamp(DateTime::TIMESTAMP_FULL));  // e.g., 2025-05-30T22:13:45
  }
  logFile.print(" - ");
  logFile.println(message);
  logFile.close();

  debugPrintln("[Logger] Log written: " + message);
}

void SdFaultLogger::dumpLogs() {
  logFile = SD.open(LOG_FILENAME, FILE_READ);
  if (!logFile) {
    debugPrintln("[Logger] Filed to open log file for reading.");
    return;
  }

  debugPrintln("[Logger] Contents of log file:");
  while (logFile.available()) {
    Serial.write(logFile.read());
  }
  logFile.close();
}

void SdFaultLogger::clear() {
  SD.remove(LOG_FILENAME);
  debugPrintln("[Logger] Log file cleared.");
}