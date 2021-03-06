#ifndef SNESCONTROLLER_H
#define SNESCONTROLLER_H

#include "base_reader.h"

class SNESReader : public BaseReader {
public:
  uint8_t raw_dump[16]; // Temp dump location

  SNESReader(JoystickStatus *JoyStatus, uint8_t* global_pins, const char* controller_name)
    : BaseReader(JoyStatus, global_pins, controller_name) {
    this->controller_type = SNES;
  };

  void init();
  void setup_pins();
  void prune(uint8_t candidates);
  void print_status(short int cnum);
  void get();
  void translate_raw_data();
  void clear_dump();

  static void isr_read();

  void read_state();
  void fillJoystick(JoystickStatus *joystick, uint8_t datamask);
};

#endif /* SNESCONTROLLER_H */
