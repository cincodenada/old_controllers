#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include "core_pins.h"

#define SLOW_PINS 0,1,2,3
#define SLOW_MASKS \
  CORE_PIN0_BITMASK, \
  CORE_PIN1_BITMASK, \
  CORE_PIN2_BITMASK, \
  CORE_PIN3_BITMASK
#define SLOW_READ \
  CORE_PIN0_PINREG, \
  CORE_PIN1_PINREG, \
  CORE_PIN2_PINREG, \
  CORE_PIN3_PINREG
#define SLOW_SET \
  CORE_PIN0_PORTSET, \
  CORE_PIN1_PORTSET, \
  CORE_PIN2_PORTSET, \
  CORE_PIN3_PORTSET
#define SLOW_CLEAR \
  CORE_PIN0_PORTCLEAR, \
  CORE_PIN1_PORTCLEAR, \
  CORE_PIN2_PORTCLEAR, \
  CORE_PIN3_PORTCLEAR

#define FAST_PINS 17,18,19,4
#define FAST_MASKS \
  CORE_PIN17_BITMASK, \
  CORE_PIN18_BITMASK, \
  CORE_PIN19_BITMASK, \
  CORE_PIN4_BITMASK
#define FAST_READ \
  CORE_PIN17_PINREG, \
  CORE_PIN18_PINREG, \
  CORE_PIN19_PINREG, \
  CORE_PIN4_PINREG
#define FAST_SET \
  CORE_PIN17_PORTSET, \
  CORE_PIN18_PORTSET, \
  CORE_PIN19_PORTSET, \
  CORE_PIN4_PORTSET
#define FAST_CLEAR \
  CORE_PIN17_PORTCLEAR, \
  CORE_PIN18_PORTCLEAR, \
  CORE_PIN19_PORTCLEAR, \
  CORE_PIN4_PORTCLEAR

//+5V = SNES, GND = NES
#define S_NES_PINS 23,22,21,20

//D7 = clock, D6 = latch
#define CLOCK_PIN 16
#define LATCH_PIN 15

#define LED_PIN 13

//C6/7 = 9/10
#define PIN_TRIGGER LED_PIN

#define IO_MASK 0x0F

const int slow_pins[] = {SLOW_PINS};
const int s_nes_pins[] = {S_NES_PINS};

#endif /* PIN_CONFIG_H */
