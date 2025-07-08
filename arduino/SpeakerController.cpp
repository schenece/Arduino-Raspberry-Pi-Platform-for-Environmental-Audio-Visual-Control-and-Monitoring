#include <Arduino.h>
#include "SpeakerController.h"
#include "Config.h"
#include "debug_utils.h"
#include <DFRobotDFPlayerMini.h>

namespace {
  DFRobotDFPlayerMini player;
  HardwareSerial& mp3Serial = Serial1;

  SpeakerState state = SpeakerState::STOPPED;
  unsigned long lastCheck = 0;

  int lastPlayedTrack = -1;  // ✅ NEW: stores the last played track number
}

void SpeakerController::begin() {
  delay(100);  // 🌟 Allow power to stabilize (amp, DFPlayer)

  mp3Serial.begin(DFPLAYER_SERIAL_BAUD);
  delay(100);  // 🌟 Give UART time to stabilize

  pinMode(DFPLAYER_BUSY_PIN, INPUT);  // ✅ Ensure Busy Pin is input

  bool initialized = false;
  for (int attempt = 1; attempt <= 2; ++attempt) {
    if (player.begin(mp3Serial, true, false)) {
      initialized = true;
      break;
    }
    Serial.print(F("[Speaker] Init failed. Attempt "));
    Serial.print(attempt);
    Serial.println(F("/2. Retrying in 2 sec..."));
    delay(2000);
  }

  if (!initialized) {
    Serial.println(F("[Speaker] DFPlayer still not responding. Continuing system..."));
    state = SpeakerState::STOPPED;
    return;
  }

  player.volume(DEFAULT_VOLUME);
  Serial.print(F("[Speaker] DFPlayer initialized. Volume: "));
  Serial.println(DEFAULT_VOLUME);

  state = SpeakerState::STOPPED;
}

void SpeakerController::start() {
  int total = -1;

  // Retry reading total files up to 3 times
  for (int i = 0; i < 3; ++i) {
    total = player.readFileCounts();
    if (total > 0) break;
    delay(500);  // Wait before retry
  }

  if (total <= 0) {
    debugPrintln("[Speaker] No valid audio files found on SD card.");
    return;
  }

  int track = random(1, total + 1);
  lastPlayedTrack = track;  // ✅ Track for light sync
  
  player.play(track);
  state = SpeakerState::PLAYING;

  if (DEBUG_MODE_SPEAKER) {
    debugPrint("[Speaker] Playing track ");
    debugPrintln(track);
  }
}

void SpeakerController::stop() {
  player.stop();
  state = SpeakerState::STOPPED;
  debugPrintln("[Speaker] Stopped playback.");
}

void SpeakerController::pause() {
  if (state == SpeakerState::PLAYING) {
    player.pause();
    state = SpeakerState::PAUSED;
    debugPrintln("[Speaker] Paused.");
  }
}

void SpeakerController::resume() {
  if (state == SpeakerState::PAUSED) {
    player.start();
    state = SpeakerState::PLAYING;
    debugPrintln("[Speaker] Resumed.");
  }
}

void SpeakerController::next() {
  player.next();
  state = SpeakerState::PLAYING;
  debugPrintln("[Speaker] Next track.");
}

void SpeakerController::prev() {
  player.previous();
  state = SpeakerState::PLAYING;
  debugPrintln("[Speaker] Previous track.");
}

void SpeakerController::setVolume(int vol) {
  player.volume(vol);
  debugPrint("[Speaker] Volume set to ");
  debugPrintln(vol);
}

void SpeakerController::loop() {
  if (state != SpeakerState::PLAYING) return;

  if (millis() - lastCheck > 1000) {
    lastCheck = millis();

    if (digitalRead(DFPLAYER_BUSY_PIN) == HIGH) {
      debugPrintln("[Speaker] Track ended. Playing next...");
      start();
    }
  }
}

SpeakerState SpeakerController::getState() {
  return state;
}

void SpeakerController::printStatus() {
  debugPrint("[Speaker] State: ");
  switch (state) {
    case SpeakerState::STOPPED: debugPrintln("STOPPED"); break;
    case SpeakerState::PLAYING: debugPrintln("PLAYING"); break;
    case SpeakerState::PAUSED:  debugPrintln("PAUSED"); break;
  }
}

int SpeakerController::getLastTrack() {
  return lastPlayedTrack;  // ✅ NEW: allows sync logic to query track
}