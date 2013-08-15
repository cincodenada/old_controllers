#ifndef N64CONTROLLER_H
#define N64CONTROLLER_H

#include "BaseController.h"

// these two macros set arduino pin 2 to input or output, which with an
// external 1K pull-up resistor to the 3.3V rail, is like pulling it high or
// low.  These operations translate to 1 op code, which takes 2 cycles
#define N64_HIGH asm volatile ("in __tmp_reg__, %[port]\nand __tmp_reg__, %[invmask]\nout %[port], __tmp_reg__\n"::[invmask] "r" (invmask), [port] "I" (_SFR_IO_ADDR(DDRD)))
#define N64_LOW asm volatile ("in __tmp_reg__, %[port]\nor __tmp_reg__, %[cmask]\nout %[port], __tmp_reg__\n"::[cmask] "r" (cmask), [port] "I" (_SFR_IO_ADDR(DDRD)))

class N64Controller : public BaseController {
public:
    char N64_raw_dump[33]; // Temp dump location

    struct JoystickStatusStruct *JoyStatus;

    N64Controller() {};
    N64Controller(struct JoystickStatusStruct *JoyStatus);

    void init();
    void print_status(short int cnum);
    void send(unsigned char *buffer, char length);
    void get();
    void translate_raw_data();
    void clear_dump();

    short int pinmask;
    short int datamask;

    void detect_controllers();
    void read_state();
    void fillStatus(struct JoystickStatusStruct *joylist);
};

#endif /* N64CONTROLLER_H */
