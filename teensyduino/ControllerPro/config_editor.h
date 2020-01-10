#include <stdint.h>
#include <chrono>

#include "serial_console.h"
#include "joystick_status.h"

using Clock = std::chrono::steady_clock;

enum class ConfigState {
  DISABLED,
  ENTERING,
  CONFIG,
  EXITING,
};

class ConfigEditor {
  std::chrono::time_point<Clock> timer_start{};
  constexpr static auto chord_len = std::chrono::seconds(5);

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
  }

  ConfigState transition(ConfigState from, uint8_t buttons) {
    if(buttons & 0b00110000) {
      switch(state) {
        case ConfigState::DISABLED:
          timer_start = Clock::now();
          return ConfigState::ENTERING;
        case ConfigState::ENTERING:
          if((Clock::now() - timer_start) >= chord_len) {
            console.log("Entering config mode");
            return ConfigState::CONFIG;
          }
          break;
        case ConfigState::CONFIG:
          return ConfigState::EXITING;
        case ConfigState::EXITING:
          if((Clock::now() - timer_start) >= chord_len) {
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
