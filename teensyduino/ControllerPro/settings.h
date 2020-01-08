#pragma once

#include <vector>
#include <stdint.h>

#include "joystick_status.h"

#define VERSION 1

class SettingsLoader {
 public:
  SettingsLoader();

  void set_defaults();

  void save();
  bool load();

  std::vector<ButtonMapping> maps;
};
