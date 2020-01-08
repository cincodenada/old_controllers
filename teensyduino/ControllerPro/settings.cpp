#include "settings.h"
#include "common.h"

#include <EEPROM.h>

SettingsLoader::SettingsLoader() {
  if(!load()) {
    set_defaults();
    save();
  }
}

void SettingsLoader::set_defaults() {
  maps.clear();
  ButtonMapping standard{
    VERSION, "Standard",
    {
      2,1,9,10,
      AXIS(1,-1),AXIS(1,1),AXIS(0,-1),AXIS(0,1),
      //HAT(0,-1),HAT(0,1),HAT(-1,0),HAT(1,0),
    },
    {
      // B Y Sel St U D L R
      2,1,9,10,
      //AXIS(1,-1),AXIS(1,1),AXIS(0,-1),AXIS(0,1),
      HAT(0,-1),HAT(0,1),HAT(-1,0),HAT(1,0),
      // A X L R
      3,4,5,6
    },
    {
      // A B Z St U D L R
      //2,1,7,10, // A/B -> B/Y
      3,2,11,8, // A/B -> A/B
      HAT(0,-1),HAT(0,1),HAT(-1,0),HAT(1,0),
      // X X L R U D L R <-(c)
      0,0,5,6,
      AXIS(3,-1),AXIS(3,1),AXIS(2,-1),AXIS(2,1),
    }
  };
  maps.push_back(std::move(standard));

  ButtonMapping ninswitch{
    VERSION, "Switch",
    {
      // A B Sel St U D L R
      2,1,7,8,
      //AXIS(1,-1),AXIS(1,1),AXIS(0,-1),AXIS(0,1),
      HAT(0,-1),HAT(0,1),HAT(-1,0),HAT(1,0),
    },
    {
      // B Y Sel St U D L R
      1,3,7,8,
      //AXIS(1,-1),AXIS(1,1),AXIS(0,-1),AXIS(0,1),
      HAT(0,-1),HAT(0,1),HAT(-1,0),HAT(1,0),
      // A X L R
      2,4,5,6
    },
    {
      // A B Z St U D L R
      2,1,11,8,
      HAT(0,-1),HAT(0,1),HAT(-1,0),HAT(1,0),
      // X X L R U D L R <-(c)
      0,0,5,6,
      AXIS(3,-1),AXIS(3,1),AXIS(2,-1),AXIS(2,1),
    }
  };
  maps.push_back(std::move(ninswitch));

  ButtonMapping bluetooth {
    VERSION, "Bluetooth",
    {
      2,1,7,8,
      AXIS(1,-1),AXIS(1,1),
    },
    {
      1,3,7,8,
      AXIS(1,-1),AXIS(1,1),
      AXIS(0,-1),AXIS(0,1),
      2,4,5,6
    },
    {
      1,2,6,12,
      AXIS(3,-1),AXIS(3,1),
      AXIS(2,-1),AXIS(2,1),
      0,0,7,8,
      14,15,9,10,
    }
  };
  maps.push_back(std::move(bluetooth));
}

void SettingsLoader::save() {
  EEPROM.write(0, VERSION);
  size_t map_addr = 16;
  EEPROM.put(map_addr, maps.size());
  map_addr += sizeof(size_t);
  for(auto m: maps) {
    EEPROM.put(map_addr, m);
    map_addr += sizeof(ButtonMapping);
  }
}

bool SettingsLoader::load() {
  printMsg(INFO, "Attempting to load...");
  if(EEPROM.read(0) == VERSION) {
    printMsg(INFO, "Loading from EEPROM");
    maps.clear();
    size_t count;
    size_t map_addr = 16;
    EEPROM.get(map_addr, count);
    printMsg(INFO, "Loading %d maps", count);
    map_addr += sizeof(size_t);
    for(size_t i=0; i<count; i++) {
      ButtonMapping cur_map;
      EEPROM.get(map_addr, cur_map);
      maps.push_back(std::move(cur_map));
      map_addr += sizeof(ButtonMapping);
    }
    printMsg(INFO, "Maps loaded");
  }

  return false;
}
