#pragma once

#ifndef LIGHT_CONTROLLER_H
#define LIGHT_CONTROLLER_H

#include <Arduino.h>

#define ZC_PIN 2
#define PSM_PIN 3

/*
 * ========================================================================
 *  LightController.h
 *  ----------------------------------------------------------------------
 *  Drives TRIAC-based lamp dimming synchronized to AC zero-cross events.
 *
 *  - Applies a phase delay (delayTimeUs) after each ZC to trigger the TRIAC
 *  - Dynamically adjusts delay to create FADE IN / FADE OUT patterns
 *  - Triggered by SystemManager when speaker playback begins
 *
 *  🔧 Manual sync mode:
 *    - Uses fadeInStepUs and fadeOutStepUs = brightnessStepUs
 *    - Updates every `cyclesPerStep` AC cycles
 *    - Parameters set dynamically per track via LightController::start(...)
 *
 *  Important:
 *    - delayForBrightestUs / delayForDarkestUs define fade bounds
 *    - Assumes 60Hz power → 120 AC cycles per second
 * ========================================================================
 */

namespace LightController {

// === Safe firing delay range (empirically stable) ===
// const unsigned int delayForBrightestUs = 1800;
// const unsigned int delayForDarkestUs  = 6500;

// For lab film:
const unsigned int delayForBrightestUs = 1500;
const unsigned int delayForDarkestUs  = 7200;

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
int zcCooldownCycles = 2;      // skip 2 zero-crosses before TRIAC pulses

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

void start(int trackNum, int brightnessStepUs, int cyclesPerStep) {
  fadingActive = true;
  delayTimeUs = delayForDarkestUs;  // 🔧 Make sure we start from dark
  fadeOutStarted = false;
  cycleCounter = 0;
  trackNumGlobal = trackNum;

  fadeInStepUs = brightnessStepUs;
  fadeOutStepUs = brightnessStepUs;
  cyclesPerStepIn = cyclesPerStep;
  cyclesPerStepOut = cyclesPerStep;

  Serial.print("[Light] Fade INIT: Track ");
  Serial.print(trackNum);
  Serial.print(" using brightnessStepUs = ");
  Serial.print(brightnessStepUs);
  Serial.print(", cyclesPerStep = ");
  Serial.print(cyclesPerStep);
  Serial.println();

  zcCooldownCycles = 2;
  lightEnabled = true;
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
    if (zcCooldownCycles > 0) {
      zcCooldownCycles--;
      return;  // Skip TRIAC trigger during cooldown
    }

    // Clamp to safe range before use
    if (delayTimeUs < delayForBrightestUs || delayTimeUs > delayForDarkestUs) {
      delayTimeUs = delayForDarkestUs;
    }

    delayMicroseconds(delayTimeUs);
    digitalWrite(PSM_PIN, HIGH);
    delayMicroseconds(50);
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