#include "joystick_status.h"

#define VERSION 1
#define NAME_LEN 32
class ButtonMapping {
  uint8_t version = 1;
  uint8_t NES[8] = {0};
  uint8_t SNES[12] = {0};
  uint8_t N64[16] = {0};

  char name[NAME_LEN] = "";

  void get_btn(controller_type_t type, uint8_t byte, uint8_t bit);
}


class SettingsLoader {
  SettingsLoader();

  void set_defaults();

  void save();
  bool load();

  std::vector<ButtonMapping> maps;
};
