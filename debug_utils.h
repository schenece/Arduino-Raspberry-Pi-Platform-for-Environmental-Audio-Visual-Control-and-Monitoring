#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#include <Arduino.h>
#include "Config.h"

#if DEBUG_MODE
  #define debugPrint(x)   Serial.print(x)
  #define debugPrintln(x) Serial.println(x)
#else
  #define debugPrint(x)
  #define debugPrintln(x)
#endif

#endif