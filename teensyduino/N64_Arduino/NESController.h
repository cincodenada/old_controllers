#ifndef NESCONTROLLER_H
#define NESCONTROLLER_H

#include "BaseController.h"

class NESController : public BaseController {
public:
    uint8_t raw_dump[8]; // Temp dump location

    NESController(struct JoystickStatusStruct *JoyStatus, uint8_t* global_pins, char* controller_name)
        : BaseController(JoyStatus, global_pins, controller_name) {};

    void setup_pins();
    void print_status(short int cnum);
    void detect_controllers();
    void get();
    void translate_raw_data();
    void clear_dump();

    short int pinmask;

    void read_state();
    void fillStatus(struct JoystickStatusStruct *joylist);
};

#endif /* NESCONTROLLER_H */
