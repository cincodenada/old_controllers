#ifndef N64CONTROLLER_H
#define N64CONTROLLER_H

#include "base_reader.h"

//Important: set DATA_PORT to low before using these
//So when we flip to output, we ouput a low!
//Outputting high may damage the N64 controller

//Sets the masked pins to input (HIGH) or output (LOW)
//Which effectively pulls high or low
//These instructions are 1 cycle each, 3 cycles total
#define N64_HIGH asm volatile ("in __tmp_reg__, %[port]\nor __tmp_reg__, %[cmask]\nout %[port], __tmp_reg__\n"::[cmask] "r" (cmask), [port] "I" (_SFR_IO_ADDR(DATA3_PORT)))
#define N64_LOW asm volatile ("in __tmp_reg__, %[port]\nand __tmp_reg__, %[invmask]\nout %[port], __tmp_reg__\n"::[invmask] "r" (invmask), [port] "I" (_SFR_IO_ADDR(DATA3_PORT)))
#define N64_IN asm volatile ("in __tmp_reg__, %[port]\nand __tmp_reg__, %[invmask]\nout %[port], __tmp_reg__\n"::[invmask] "r" (invmask), [port] "I" (_SFR_IO_ADDR(DATA3_DIR)))
#define N64_OUT asm volatile ("in __tmp_reg__, %[port]\nor __tmp_reg__, %[cmask]\nout %[port], __tmp_reg__\n"::[cmask] "r" (cmask), [port] "I" (_SFR_IO_ADDR(DATA3_DIR)))

#define N64_AXIS_MAX (85)

class N64Reader : public BaseReader {
public:
  uint8_t raw_dump[TBUFSIZE]; // Temp dump location

  N64Reader(JoystickStatus *JoyStatus, uint8_t* global_pins, const char* controller_name)
    : BaseReader(JoyStatus, global_pins, controller_name) {
    this->controller_type = N64;
  };

  void init();
  void setup_pins();
  void prune(uint8_t candidates);
  void print_status(short int cnum);
  void send(uint8_t pin, uint8_t *buffer, uint8_t length);
  bool recv(size_t read_bits);
  void get();
  void translate_raw_data();
  void clear_dump();

  static void isr_read();
  static void isr_write();

  void read_state();
  void fillJoystick(JoystickStatus *joystick, uint8_t datamask);

  signed short int safe_axis(int8_t rawval);
};

#endif /* N64CONTROLLER_H */
