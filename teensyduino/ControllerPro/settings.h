#include <vector>
#include <stdint.h>

#include "joystick_status.h"

#define VERSION 1
#define NAME_LEN 32
struct ButtonMapping {
  uint8_t version = 1;
  char name[NAME_LEN] = "";

  uint8_t NES_map[8] = {0};
  uint8_t SNES_map[12] = {0};
  uint8_t N64_map[16] = {0};

  uint8_t get_btn(controller_type_t type, uint8_t byte, uint8_t bit);
};


class SettingsLoader {
 public:
  SettingsLoader();

  void set_defaults();

  void save();
  bool load();

  std::vector<ButtonMapping> maps;
};
