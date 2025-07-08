
# Arduino-Raspberry-Pi-Platform-for-Environmental-Audio-Visual-Control-and-Monitoring

This project provides a synchronized hardware–software system that enables dynamic light and audio playback based on real-time cues and wildlife presence. It is designed for field deployment and bird behavior studies using a hybrid setup with an Arduino (Metro M0) and Raspberry Pi Zero.

---

## 📁 Folder Structure

```
Arduino-Raspberry-Pi-Platform-for-Environmental-Audio-Visual-Control-and-Monitoring/
├── arduino/               # Metro M0 Express source code
│   ├── LightController.h      # Controls AC lamp via dimmer + light fade logic
│   ├── SpeakerController.cpp  # DFPlayer Mini interface + playback control
│   ├── SystemManager.cpp      # State machine logic: IDLE / ACTIVE
│   ├── Config.h               # Constants and flags
│   └── *.py                   # Python tools to generate fade tables
├── raspberrypi/           # Raspberry Pi Zero scripts
│   ├── dev_sync.sh            # GPIO pulse sync script
│   ├── rtc_wakeup.py          # RTC-driven system wake-up logic
│   └── presence_detect.py     # (optional) Audio/video presence detection
├── speakerfiles/          # MP3 files loaded to DFPlayer SD card
└── README.md              # You’re reading it now
```
---

## 🧠 System Overview

The system operates as a **time-aware, two-state controller**:

- **IDLE (daytime)**: Camera and microphone stay active for passive observation; lamps and speaker are off.
- **ACTIVE (nighttime)**: Lamps fade in/out in sync with each audio track. Speaker plays randomized MP3s.

Transitions are triggered by the Raspberry Pi via a brief GPIO pulse at **sunset/sunrise**, based on its internal RTC.

---

## 🛠 Hardware Components

- 🟢 **Adafruit Metro M0 Express**
  - Listens for state-change pulses.
  - Controls AC dimmers and the speaker via DFPlayer Mini.
- 🍓 **Raspberry Pi Zero**
  - Computes sunrise/sunset via RTC.
  - Sends trigger pulse to Metro.
  - Optionally detects bird presence (camera/mic).
- 💡 **DynRobot AC Dimmer Modules** (x2)
- 🔊 **DFPlayer Mini + Rockville HP4S-8 BK speaker**
- 📷 **Unistorm Night Vision Camera**
- 🎙 **SPH0645LM4H Digital Microphone**

---

## 🔁 Light-Speaker Synchronization

During ACTIVE mode, as each new track plays:
- LightController begins a fade-in synced with track duration.
- On track end, light fades out.
- The system repeats continuously with randomized track selection.

---

## 🔧 Developer Notes

- `extract_fade_steps.py` and `generate_fade_table.py` (in `arduino/`) help derive fade timings per track.
- Logs and debug output help trace system state transitions.
- All timing and detection logic is offloaded to the Pi; the Arduino acts as a reactive executor.

---

## 📜 License

MIT License (see `LICENSE` if applicable).
