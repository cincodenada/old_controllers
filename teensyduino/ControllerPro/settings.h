#pragma once

#include <vector>
#include <stdint.h>

#include "simple_string.h"
#include "joystick_status.h"

#define VERSION 1

class SettingsLoader {
 public:
  SettingsLoader();

  void set_defaults();
  void add_map(const char* name, ButtonMapping&& map);
  ButtonMapping& find_map(const char* name);
  ButtonMapping& get_map(size_t idx);

  void save();
  bool load();

 private:
  std::vector<ButtonMapping> maps;
  std::vector<SimpleString> map_names;
};
