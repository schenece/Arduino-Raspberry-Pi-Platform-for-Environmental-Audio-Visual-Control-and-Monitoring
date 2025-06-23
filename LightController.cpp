#include "LightController.h"
#include "debug_utils.h"

namespace {

// ====== CONFIGURATION ======
const int zeroCrossPin = 2;
const int acdPin = 3;

const unsigned int minDelay = 2000;
const unsigned int maxDelay = 6000;

const int numPatterns = 10;
const unsigned long patterns[numPatterns][3] = {
  {1300, 8500, 5300}, {2300, 8000, 3700}, {9800, 4700, 6400},
  {4700, 8700, 3500}, {8600, 5900, 6900}, {4100, 4300, 5200},
  {7600, 5500, 2500}, {1900, 3300, 3900}, {6300, 5700, 6100},
  {9700, 9300, 4700}
};

// ====== INTERNAL STATE ======
volatile bool zeroCrossDetected = false;

int currentPattern = -1;
unsigned int delayTime = maxDelay;
unsigned int fadeStep = 0;
LightController::LampState currentState = LightController::LampState::IDLE;
LightController::LampState previousState = LightController::LampState::IDLE;

unsigned long offStartTime = 0;
bool running = false;

}  // namespace

// ====== PUBLIC API ======

void LightController::begin() {
  pinMode(zeroCrossPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(zeroCrossPin), onZeroCross, RISING);

  pinMode(acdPin, OUTPUT);
  digitalWrite(acdPin, LOW);

  debugPrintln(F("[Light] Zero-cross detection and AC dimmer initialized."));
}

void LightController::onZeroCross() {
  zeroCrossDetected = true;
}

void LightController::start() {
  if (running) {
    static unsigned long lastLog = 0;
    if (millis() - lastLog > 3000) {
      lastLog = millis();
      debugPrintln(F("[Light] Already running."));
    }
    return;
  }

  running = true;
  currentState = LampState::FADE_IN;

  if (currentPattern < 0) {
    currentPattern = random(0, numPatterns);
  }

  delayTime = maxDelay;
  fadeStep = (maxDelay - minDelay) /
             ((patterns[currentPattern][0] * 1000UL) / (1000000UL / 50));

  debugPrint(F("[Light] Starting pattern index "));
  debugPrintln(currentPattern);
}

void LightController::stop() {
  if (!running) return;

  running = false;
  currentState = LampState::STOPPED;
  zeroCrossDetected = false;
  digitalWrite(acdPin, LOW);

  debugPrintln(F("[Light] Stopped lighting pattern."));
}

void LightController::update() {
  if (!running) return;

  if (currentState != previousState) {
    printCurrentState();
    previousState = currentState;
  }

  switch (currentState) {
    case LampState::IDLE:
      start();  // Automatically restart pattern
      break;

    case LampState::FADE_IN:
      if (zeroCrossDetected) {
        zeroCrossDetected = false;
        delayMicroseconds(delayTime);
        digitalWrite(acdPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(acdPin, LOW);

        delayTime -= fadeStep;
        if (delayTime <= minDelay) {
          delayTime = minDelay;
          currentState = LampState::FADE_OUT;
        }
      }
      break;

    case LampState::FADE_OUT:
      if (zeroCrossDetected) {
        zeroCrossDetected = false;
        delayMicroseconds(delayTime);
        digitalWrite(acdPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(acdPin, LOW);

        delayTime += fadeStep;
        if (delayTime >= maxDelay) {
          delayTime = maxDelay;
          currentState = LampState::OFF_INTERVAL;
          offStartTime = millis();
        }
      }
      break;

    case LampState::OFF_INTERVAL:
      if (millis() - offStartTime >= patterns[currentPattern][2]) {
        debugPrintln(F("[Light] OFF interval done. Restarting pattern."));
        currentPattern = -1;  // ✅ reset pattern for next cycle
        currentState = LampState::IDLE;
        running = false;
      }
      break;

    case LampState::STOPPED:
      break;
  }
}

void LightController::setPattern(int index) {
  if (index >= 0 && index < numPatterns) {
    currentPattern = index;
    debugPrint(F("[Light] Pattern manually set to: "));
    debugPrintln(index);
  }
}

void LightController::printCurrentState() {
  debugPrint(F("[Light] State changed to: "));
  switch (currentState) {
    case LampState::IDLE:         debugPrintln("IDLE"); break;
    case LampState::FADE_IN:      debugPrintln("FADE_IN"); break;
    case LampState::FADE_OUT:     debugPrintln("FADE_OUT"); break;
    case LampState::OFF_INTERVAL: debugPrintln("OFF_INTERVAL"); break;
    case LampState::STOPPED:      debugPrintln("STOPPED"); break;
  }
}