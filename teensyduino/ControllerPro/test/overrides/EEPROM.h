#pragma once

#define EEPROM_SIZE 1024
struct FakeEEPROM {
  uint8_t data[EEPROM_SIZE];

  size_t length() { return EEPROM_SIZE; }

  template<typename T>
  void get(size_t addr, T& dest) {
    memcpy(&dest, data+addr, sizeof(T));
  }

  template<typename T>
  void put(size_t addr, const T& src) {
    memcpy(data+addr, &src, sizeof(T));
  }

  uint8_t read(size_t addr) {
    return data[addr];
  }

  void write(size_t addr, uint8_t val) {
    data[addr] = val;
  }
};

FakeEEPROM EEPROM;
