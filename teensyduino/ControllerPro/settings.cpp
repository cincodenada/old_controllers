#include "settings.h"
#include "serial_console.h"

#include <algorithm>
#include <EEPROM.h>

void Settings::init() {
  if(!load()) {
    set_defaults();
    save();
  }
}

ButtonMapping& Settings::find_map(const char* name) {
  auto it = std::find(map_names.begin(), map_names.end(), name);
  return maps[std::distance(map_names.begin(), it)];
}
ButtonMapping& Settings::get_map(size_t idx) {
  return maps[idx];
}

void Settings::set_defaults() {
  maps.clear();
  map_names.clear();

  console.log("Creating default maps");

  add_map("Standard", {
    {
      // A B Sel St U D L R
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
  });

  add_map("Switch", {
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
  });

  add_map("Bluetooth", {
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
  });
}

void Settings::add_map(const char* name, ButtonMapping&& map) {
  maps.push_back(std::move(map));
  map_names.emplace_back(name);
}

void Settings::save() {
  console.log("Writing maps");

  EEPROM.write(0, VERSION);
  size_t map_addr = 16;
  EEPROM.put(map_addr, maps.size());
  map_addr += sizeof(size_t);
  size_t idx = 0;
  for(auto m: maps) {
    auto& name = map_names[idx];
    EEPROM.write(map_addr, (uint8_t)name.size());
    map_addr++;
    for(auto c: name) {
      EEPROM.write(map_addr, c);
      map_addr++;
    }
    EEPROM.put(map_addr, m);
    map_addr += sizeof(ButtonMapping);
    idx++;
  }
}

void Settings::clear() {
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}

bool Settings::load() {
  console.log(INFO, "Attempting to load...");
  if(EEPROM.read(0) == VERSION) {
    console.log(INFO, "Loading from EEPROM");
    maps.clear();
    size_t count;
    size_t map_addr = 16;
    EEPROM.get(map_addr, count);
    console.log(INFO, "Loading %d maps", count);
    map_addr += sizeof(size_t);
    for(size_t i=0; i<count; i++) {
      ButtonMapping cur_map;
      uint8_t name_len = EEPROM.read(map_addr);
      map_addr++;
      SimpleString name;
      for(int i=0; i<name_len; i++) {
        name.push_back(EEPROM.read(map_addr+i));
      }
      map_addr += name_len;
      EEPROM.get(map_addr, cur_map);
      maps.push_back(std::move(cur_map));
      map_names.push_back(std::move(name));

      map_addr += sizeof(ButtonMapping);
    }
    console.log(INFO, "Maps loaded");

    return true;
  }

  return false;
}
