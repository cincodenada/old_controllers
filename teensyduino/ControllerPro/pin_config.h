#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include "core_pins.h"

#define PORT(p0,p1,p2,p3)

#define SP0 5
#define SP1 3
#define SP2 2
#define SP3 4


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

#define FP0 0
#define FP1 23
#define FP2 22
#define FP3 1

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

/*
#define EXTRA_PINS 20,21,6,19
//+5V = SNES, GND = NES
#define S_NES_PINS 17,18,15,16
*/

#define S_NES_PINS 20,21,6,19
#define EXTRA_PINS 17,18,15,16

//D7 = clock, D6 = latch
#define CLOCK_PIN 9
#define LATCH_PIN 10

#define LED_PIN 13

//C6/7 = 9/10
#define TRIGGER_PIN LED_PIN

#define IO_MASK 0x0F

#define NUMSLOTS 4

// Values for s_nes_pins
// Reversed depending on whether we're using transistors,
// or supplying directly from the I/O ports
//#define BUFFERED_MODE
#ifdef BUFFERED_MODE
  #define MODE_SNES LOW
  #define MODE_NES HIGH
#else
  #define MODE_SNES HIGH
  #define MODE_NES LOW
#endif

constexpr uint8_t fast_pins[] = {FAST_PINS};
constexpr uint8_t slow_pins[] = {SLOW_PINS};
constexpr uint8_t s_nes_pins[] = {S_NES_PINS};
constexpr uint8_t extra_pins[] = {EXTRA_PINS};

#endif /* PIN_CONFIG_H */
