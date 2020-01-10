#include <stdint.h>

#include "Arduino.h"
#include "serial_console.h"
#include "joystick_status.h"

enum class ConfigState : uint8_t {
  DISABLED = 0,
  ENTERING,
  CONFIG,
  EXITING,
};

class ConfigEditor {
  long timer_start{};
  static constexpr long chord_len = 5*1000;
  static constexpr uint8_t chord = 0b1100; // Sel + St

  // Makes order consistently A B Sel/Z St U D L R
  ButtonMapping config_map {
    {1,2,3,4,5,6,7,8},
    {2,0,3,4,5,6,7,8,1,0,0,0},
    {1,2,3,4,5,6,7,8,0,0,0,0},
  };

 public:
  ConfigState state = ConfigState::DISABLED;

  void update(JoystickStatus& raw_status) {
    auto status = config_map.remap(raw_status);
    auto buttons = status.buttonset[0];
    state = transition(state, buttons);

    console.log("Config state: %d (from %x)", (uint8_t)state, buttons);
  }

  ConfigState transition(ConfigState from, uint8_t buttons) {
    if((buttons & chord) == chord) {
      switch(state) {
        case ConfigState::DISABLED:
          timer_start = millis();
          return ConfigState::ENTERING;
        case ConfigState::ENTERING:
          if((millis() - timer_start) >= chord_len) {
            console.log("Entering config mode");
            return ConfigState::CONFIG;
          }
          break;
        case ConfigState::CONFIG:
          timer_start = millis();
          return ConfigState::EXITING;
        case ConfigState::EXITING:
          if((millis() - timer_start) >= chord_len) {
            console.log("Exiting config mode");
            return ConfigState::DISABLED;
          }
          break;
      }
    } else {
      switch(state) {
        case ConfigState::ENTERING:
          return ConfigState::DISABLED;
        case ConfigState::EXITING:
          return ConfigState::CONFIG;
        default:
          break;
      }
    }

    return from;
  }
};
