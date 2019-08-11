#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include "core_pins.h"

#define PORT(p0,p1,p2,p3)

#define SP0 0
#define SP1 1
#define SP2 2
#define SP3 3

#define SLOW_PINS SP0,SP1,SP2,SP3
#define SLOW_MASKS \
  CORE_PIN ## SP0 ## _BITMASK, \
  CORE_PIN ## SP1 ## _BITMASK, \
  CORE_PIN ## SP2 ## _BITMASK, \
  CORE_PIN ## SP3 ## _BITMASK
#define SLOW_READ \
  CORE_PIN ## SP0 ## _PINREG, \
  CORE_PIN ## SP1 ## _PINREG, \
  CORE_PIN ## SP2 ## _PINREG, \
  CORE_PIN ## SP3 ## _PINREG
#define SLOW_SET \
  CORE_PIN ## SP0 ## _PORTSET, \
  CORE_PIN ## SP1 ## _PORTSET, \
  CORE_PIN ## SP2 ## _PORTSET, \
  CORE_PIN ## SP3 ## _PORTSET
#define SLOW_CLEAR \
  CORE_PIN ## SP0 ## _PORTCLEAR, \
  CORE_PIN ## SP1 ## _PORTCLEAR, \
  CORE_PIN ## SP2 ## _PORTCLEAR, \
  CORE_PIN ## SP3 ## _PORTCLEAR

#define FP0 17
#define FP1 18
#define FP2 19
#define FP3 4

#define FAST_PINS FP0,FP1,FP2,FP3
#define FAST_MASKS \
  CORE_PIN ## FP0 ## _BITMASK, \
  CORE_PIN ## FP1 ## _BITMASK, \
  CORE_PIN ## FP2 ## _BITMASK, \
  CORE_PIN ## FP3 ## _BITMASK
#define FAST_READ \
  CORE_PIN ## FP0 ## _PINREG, \
  CORE_PIN ## FP1 ## _PINREG, \
  CORE_PIN ## FP2 ## _PINREG, \
  CORE_PIN ## FP3 ## _PINREG
#define FAST_SET \
  CORE_PIN ## FP0 ## _PORTSET, \
  CORE_PIN ## FP1 ## _PORTSET, \
  CORE_PIN ## FP2 ## _PORTSET, \
  CORE_PIN ## FP3 ## _PORTSET
#define FAST_CLEAR \
  CORE_PIN ## FP0 ## _PORTCLEAR, \
  CORE_PIN ## FP1 ## _PORTCLEAR, \
  CORE_PIN ## FP2 ## _PORTCLEAR, \
  CORE_PIN ## FP3 ## _PORTCLEAR

//+5V = SNES, GND = NES
#define S_NES_PINS 23,22,21,20

//D7 = clock, D6 = latch
#define CLOCK_PIN 16
#define LATCH_PIN 15

#define LED_PIN 13

#define TOUCH_REF 15

//C6/7 = 9/10
#define PIN_TRIGGER LED_PIN

#define IO_MASK 0x0F

constexpr int fast_pins[] = {FAST_PINS};
constexpr int slow_pins[] = {SLOW_PINS};
constexpr int s_nes_pins[] = {S_NES_PINS};

#endif /* PIN_CONFIG_H */
