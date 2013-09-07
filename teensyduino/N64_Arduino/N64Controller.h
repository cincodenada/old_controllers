#ifndef N64CONTROLLER_H
#define N64CONTROLLER_H

#include "BaseController.h"

//Important: set DATA_PORT to low before using these
//So when we flip to output, we ouput a low!
//Outputting high may damage the N64 controller

//Sets the masked pins to input (HIGH) or output (LOW)
//Which effectively pulls high or low
//Original note recommended a 1K pull-up resistor to 3.3V
//But relying on the controller to pull high seems to work fine
//TODO: Perhaps add a pull-up to be safe?
//These instructions are 1 cycle each, 3 cycles total
#define N64_HIGH asm volatile ("in __tmp_reg__, %[port]\nand __tmp_reg__, %[invmask]\nout %[port], __tmp_reg__\n"::[invmask] "r" (invmask), [port] "I" (_SFR_IO_ADDR(DATA_DIR)))
#define N64_LOW asm volatile ("in __tmp_reg__, %[port]\nor __tmp_reg__, %[cmask]\nout %[port], __tmp_reg__\n"::[cmask] "r" (cmask), [port] "I" (_SFR_IO_ADDR(DATA_DIR)))

class N64Controller : public BaseController {
public:
    char N64_raw_dump[33]; // Temp dump location

    struct JoystickStatusStruct *JoyStatus;

    N64Controller() {};
    N64Controller(struct JoystickStatusStruct *, char *);

    void init();
    void print_status(short int cnum);
    void detect_controllers();
    void send(unsigned char *buffer, char length);
    void get();
    void translate_raw_data();
    void clear_dump();

    short int pinmask;

    void read_state();
    void fillStatus(struct JoystickStatusStruct *joylist);
};

#endif /* N64CONTROLLER_H */
