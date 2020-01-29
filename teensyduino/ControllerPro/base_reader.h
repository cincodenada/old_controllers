#ifndef BASECONTROLLER_H
#define BASECONTROLLER_H

#include <stdint.h>

#include "Arduino.h"
#include "pin_config.h"
#include "binutil.h"
#include "serial_console.h"
#include "joystick_status.h"
#include "TimerOne.h"

#define CNAME_LEN 10
// Maximum data an interrupt will need to read/write
#define TBUFSIZE 32*8

struct interrupt_data_struct {
  // Add a pad byte cause we might try to
  // read one past the end sometimes
  volatile uint8_t buf[TBUFSIZE+1];
  volatile uint8_t *cur_byte;
  volatile uint8_t *end_byte;
  volatile uint8_t cur_bit;
  volatile uint8_t cur_stage;
  volatile uint8_t cur_val;

  // Mode: 0 = writing, 1 = reading, 2 = done
  volatile uint8_t mode;

  const uint8_t *pins;
  uint8_t cur_pin;

  volatile bool done;
};

class BaseReader {
public:
  uint8_t pinmask = 0;
  uint8_t* globalmask;
  char controller_name[CNAME_LEN];
  controller_type_t controller_type;

  JoystickStatus *JoyStatus;

  BaseReader(JoystickStatus *JoyStatus, uint8_t* global_pins, const char* controller_name);
  virtual void init();
  virtual void claim_slot(int num);

  virtual void setup_pins() = 0;
  virtual void prune(uint8_t candidates) = 0;

  virtual void read_state() = 0;
  virtual void fillStatus(JoystickStatus *joylist);
  virtual void fillJoystick(JoystickStatus *joystick, uint8_t datamask) = 0;
  virtual bool is_fast();
  uint8_t read_pin(uint8_t pin);
  uint8_t get_deviants(uint8_t pins_avail, uint8_t expected);

  static void reset_isr_data();

  static interrupt_data_struct isr_data;
};

#endif /* BASECONTROLLER_H */
