#include <EEPROM.h>
#include "joystick_status.h"

#define VERSION 1
#define NAME_LEN 32
class ButtonMapping {
  uint8_t version = 1;
  uint8_t NES[8] = {0};
  uint8_t SNES[12] = {0};
  uint8_t N64[16] = {0};

  char name[NAME_LEN] = "";

  uint8_t get_btn(controller_type_t type, uint8_t byte, uint8_t bit) {
    switch(type) {
      case NES: return NES[byte*8 + bit];
      case SNES: return SNES[byte*8 + bit];
      case N64: return N64[byte*8 + bit];
    }
  }
};

std::vector<ButtonMapping> maps;

SettingsLoader::SettingsLoader() {
  if !self.load() {
    self.set_defaults();
    self.save();
  }
}

void SettingsLoader::set_defaults() {
  maps.clear();
  ButtonMapping standard;
  standard.NES = {
    2,1,9,10,
    AXIS(1,-1),AXIS(1,1),AXIS(0,-1),AXIS(0,1),
    //HAT(0,-1),HAT(0,1),HAT(-1,0),HAT(1,0),
  };
  standard.SNES = {
    // B Y Sel St U D L R
    2,1,9,10,
    //AXIS(1,-1),AXIS(1,1),AXIS(0,-1),AXIS(0,1),
    HAT(0,-1),HAT(0,1),HAT(-1,0),HAT(1,0),
    // A X L R
    3,4,5,6
  };
  standard.N64 = {
    // A B Z St U D L R
    //2,1,7,10, // A/B -> B/Y
    3,2,11,8, // A/B -> A/B
    HAT(0,-1),HAT(0,1),HAT(-1,0),HAT(1,0),
    // X X L R U D L R <-(c)
    0,0,5,6,
    AXIS(3,-1),AXIS(3,1),AXIS(2,-1),AXIS(2,1),
  };
  strncpy(standard.name, "Standard", NAME_LEN-1);
  standard.name[NAME_LEN-1] = 0;

  maps.push(std::move(standard));

  ButtonMapping ninswitch;
  ninswitch.NES = {
    // A B Sel St U D L R
    2,1,7,8,
    //AXIS(1,-1),AXIS(1,1),AXIS(0,-1),AXIS(0,1),
    HAT(0,-1),HAT(0,1),HAT(-1,0),HAT(1,0),
  };
  ninswitch.SNES = {
    // B Y Sel St U D L R
    1,3,7,8,
    //AXIS(1,-1),AXIS(1,1),AXIS(0,-1),AXIS(0,1),
    HAT(0,-1),HAT(0,1),HAT(-1,0),HAT(1,0),
    // A X L R
    2,4,5,6
  };
  ninswitch.N64 = {
    // A B Z St U D L R
    2,1,11,8,
    HAT(0,-1),HAT(0,1),HAT(-1,0),HAT(1,0),
    // X X L R U D L R <-(c)
    0,0,5,6,
    AXIS(3,-1),AXIS(3,1),AXIS(2,-1),AXIS(2,1),
  };
  strncpy(ninswitch.name, "Switch", NAME_LEN-1);
  ninswitch.name[NAME_LEN-1] = 0;

  maps.push(std::move(ninswitch));

  ButtonMapping bluetooth;
  bluetooth.NES = {
    2,1,7,8,
    AXIS(1,-1),AXIS(1,1),
  };
  bluetooth.SNES = {
    1,3,7,8,
    AXIS(1,-1),AXIS(1,1),
    AXIS(0,-1),AXIS(0,1),
    2,4,5,6
  };
  bluetooth.N64 = {
    1,2,6,12,
    AXIS(3,-1),AXIS(3,1),
    AXIS(2,-1),AXIS(2,1),
    0,0,7,8,
    14,15,9,10,
  };
  strncpy(bluetooth.name, "Switch", NAME_LEN-1);
  bluetooth.name[NAME_LEN-1] = 0;

  maps.push(std::move(bluetooth));
}

void SettingLoader::save() {
  EEPROM.write(0, VERSION);
  size_t map_addr = 16;
  EEPROM.put(map_addr, m.size());
  map_addr += sizeof(size_t);
  for m in maps {
    EEPROM.put(map_addr, m);
    map_addr += sizeof(ButtonMapping);
  }
}

bool SettingsLoader::load() {
  if(EEPROM.read(0) == VERSION) {
    maps.clear();
    size_t count;
    size_t map_addr = 16;
    EEPROM.get(map_addr, count);
    map_addr += sizeof(size_t);
    for(int i=0; i<count; i++) {
      ButtonMapping cur_map;
      EEPROM.get(map_addr, cur_map);
      maps.push(std::move(cur_map));
      map_addr += sizeof(ButtonMapping);
    }
  }

  return false;
}
