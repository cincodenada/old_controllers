#ifndef BASECONTROLLER_H
#define BASECONTROLLER_H

#include "Arduino.h"
#include "pin_config.h"

// 8 bytes of data that we get from the controller
struct JoystickStatusStruct {
    unsigned char axis[3];
    unsigned char buttonset[2];
};

class BaseController {
public:
    short int pinmask;
    short int datamask;

    void init();
    void print_status(short int cnum);
    void detect_controllers();
    void read_state();
    void fillStatus(struct JoystickStatusStruct *joylist);
};

#endif /* BASECONTROLLER_H */
