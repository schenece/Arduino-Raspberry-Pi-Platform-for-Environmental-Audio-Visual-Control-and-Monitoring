#pragma once

#ifndef LIGHT_CONTROLLER_H
#define LIGHT_CONTROLLER_H

#include <Arduino.h>

#define ZC_PIN 2
#define PSM_PIN 3

namespace LightController {

// === Safe firing delay range (empirically stable) ===
const unsigned int delayForBrightestUs = 1800;
const unsigned int delayForDarkestUs  = 7500;

// === Runtime State ===
volatile bool triggerPending = false;
volatile unsigned long lastZC = 0;

unsigned int fadeInStepUs = 12;
unsigned int fadeOutStepUs = 20;

unsigned int cyclesPerStepIn = 1;
unsigned int cyclesPerStepOut = 1;
unsigned int cycleCounter = 0;

unsigned int delayTimeUs = delayForDarkestUs;
bool fadingActive = false;
bool lightEnabled = false;
bool fadeOutStarted = false;

int trackNumGlobal = 0;

void onZeroCross() {
  triggerPending = true;
  lastZC = micros();
}

void begin() {
  pinMode(ZC_PIN, INPUT);
  pinMode(PSM_PIN, OUTPUT);
  digitalWrite(PSM_PIN, LOW);
  attachInterrupt(digitalPinToInterrupt(ZC_PIN), onZeroCross, RISING);
  Serial.println("[Light] Hardware test light controller initialized.");
}

void start(int trackNum, int fadeTimeMs) {
  fadingActive = true;
  lightEnabled = true;
  delayTimeUs = delayForDarkestUs;
  fadeOutStarted = false;
  cycleCounter = 0;
  trackNumGlobal = trackNum;

  int totalDeltaUs = delayForDarkestUs - delayForBrightestUs;

  // === FADE IN
  float fadeHalfSecIn = fadeTimeMs / 1000.0 / 2.0;
  float fadeCyclesIn = fadeHalfSecIn * 120.0;
  int totalStepsIn = totalDeltaUs / fadeInStepUs;
  cyclesPerStepIn = max(1, round(fadeCyclesIn / totalStepsIn));

  // === FADE OUT
  float fadeHalfSecOut = fadeTimeMs / 1000.0 / 2.0;
  float fadeCyclesOut = fadeHalfSecOut * 120.0;
  int totalStepsOut = totalDeltaUs / fadeOutStepUs;
  cyclesPerStepOut = max(1, round(fadeCyclesOut / totalStepsOut));

  Serial.print("[Light] Starting FADE IN for Track ");
  Serial.print(trackNum);
  Serial.print(" (fadeInStepUs = ");
  Serial.print(fadeInStepUs);
  Serial.print(", fadeOutStepUs = ");
  Serial.print(fadeOutStepUs);
  Serial.print(", fadeTimeMs = ");
  Serial.print(fadeTimeMs);
  Serial.print(", cyclesPerStepIn = ");
  Serial.print(cyclesPerStepIn);
  Serial.print(", cyclesPerStepOut = ");
  Serial.print(cyclesPerStepOut);
  Serial.println(")");
}

void stop() {
  fadingActive = false;
  lightEnabled = false;
  digitalWrite(PSM_PIN, LOW);
  // Serial.println("[Light] Stopped and triac disabled.");
}

// === Integrated LightSync fade logic (from tested MockLightController.h)
void update() {
  if (!triggerPending) return;
  triggerPending = false;

  if (lightEnabled) {
    delayMicroseconds(delayTimeUs);
    digitalWrite(PSM_PIN, HIGH);
    delayMicroseconds(50);  // pulse
    digitalWrite(PSM_PIN, LOW);
  }

  if (!fadingActive) return;

  // Choose which cycle counter to compare
  unsigned int stepTarget = fadeOutStarted ? cyclesPerStepOut : cyclesPerStepIn;

  cycleCounter++;
  if (cycleCounter < stepTarget) return;
  cycleCounter = 0;

  if (!fadeOutStarted) {
    // FADE IN: decrease delay → brighter
    delayTimeUs -= fadeInStepUs;

    if (delayTimeUs <= delayForBrightestUs) {
      delayTimeUs = delayForBrightestUs;
      fadeOutStarted = true;
      Serial.println("✅ FADE IN complete.");
      Serial.println("↘ Starting FADE OUT...");
    }
  } else {
    // FADE OUT: increase delay → darker
    delayTimeUs += fadeOutStepUs;

    if (delayTimeUs >= delayForDarkestUs) {
      delayTimeUs = delayForDarkestUs;
      fadingActive = false;
      Serial.println("✅ FADE OUT complete.");
      Serial.println("🔁 One full FADE cycle complete.");
    }
  }
}

inline void printCurrentState() {
  // Serial.println("[Light] printCurrentState() not implemented in sync mode.");
}

}  // namespace LightController

#endif  // LIGHT_CONTROLLER_H