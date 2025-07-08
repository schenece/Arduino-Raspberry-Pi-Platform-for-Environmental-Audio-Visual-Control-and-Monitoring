#ifndef SPEAKER_CONTROLLER_H
#define SPEAKER_CONTROLLER_H

enum class SpeakerState { STOPPED, PLAYING, PAUSED };

namespace SpeakerController {
  void begin();
  void start();
  void stop();
  void pause();
  void resume();
  void next();
  void prev();
  void setVolume(int vol);
  void loop();
  void printStatus();

  SpeakerState getState();
  int getLastTrack(); // ✅ NEW
}

#endif