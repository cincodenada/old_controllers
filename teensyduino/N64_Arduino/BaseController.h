#ifndef BASECONTROLLER_H
#define BASECONTROLLER_H

#include "Arduino.h"
#include "pin_config.h"
#include "common.h"

// 8 bytes of data that we get from the controller
struct JoystickStatusStruct {
    signed short int axis[3];
    unsigned char buttonset[2];
};

class BaseController {
public:
    short int pinmask;
    char* globalmask;

    void init(char pins_avail);
    void print_status(short int cnum);
    void detect_controllers(char pins_avail);
    void read_state();
    void fillStatus(struct JoystickStatusStruct *joylist);
};

#endif /* BASECONTROLLER_H */
