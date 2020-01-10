#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <stdint.h>

void delay(int millisecond) {};

#include "../serial_console.h"
#include "../settings.h"
#include "../settings.cpp"

template<typename T>
bool operator==(const T& a, const T& b) {
  return memcmp(&a, &b, sizeof(T)) == 0;
}

TEST_CASE("Adding a map works") {
  SECTION("Write") {
    Settings settings;

    ButtonMapping new_map;
    settings.add_map("Test", {
        {1,2,3,4,5,6,7,8},
        {1,2,3,4,5,6,7,8,9,10,11,12},
        {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16},
    });

    auto map = settings.find_map("Test");
    for(int i=0; i < 16; i++) {
      if(i < 8) { REQUIRE(map.NES_map[i] == i+1); }
      if(i < 12) { REQUIRE(map.SNES_map[i] == i+1); }
      REQUIRE(map.N64_map[i] == i+1);
    }

    settings.save();
    REQUIRE(EEPROM.data[0] == 1);
    // One map
    REQUIRE(EEPROM.data[16] == 1);
    // Length-prefixed name
    REQUIRE(EEPROM.data[24] == 4);
    REQUIRE(strncmp((const char*)&EEPROM.data[25], "Test", 4) == 0);

    // Maps
    size_t map_base = 25+4;
    for(int i=0; i < 16; i++) {
      if(i < 8) { REQUIRE(EEPROM.data[map_base+i] == i+1); }
      if(i < 12) { REQUIRE(EEPROM.data[map_base+8+i] == i+1); }
      REQUIRE(EEPROM.data[map_base+20+i] == i+1);
    }
  }

  SECTION("Read") {
    /*
    for(int i=0; i<100; i++) {
      if(i % 16 == 0) { printf("\n"); }
      printf("%02x ", EEPROM.data[i]);
    }
    */
    Settings settings;
    REQUIRE(settings.load());

    auto map = settings.find_map("Test");
    for(int i=0; i < 16; i++) {
      if(i < 8) { REQUIRE(map.NES_map[i] == i+1); }
      if(i < 12) { REQUIRE(map.SNES_map[i] == i+1); }
      REQUIRE(map.N64_map[i] == i+1);
    }
  }
}
