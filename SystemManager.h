#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

enum class SystemState { IDLE, ACTIVE };

namespace SystemManager {
  void begin();
  void loop();
  void setState(SystemState state);
  SystemState getState();
  void printStatus();
}

#endif