#ifndef BASECONTROLLER_H
#define BASECONTROLLER_H

#include <stdint.h>

#include "Arduino.h"
#include "pin_config.h"
#include "common.h"

// 8 bytes of data that we get from the controller
struct JoystickStatusStruct {
    signed short int axis[3];
    uint8_t buttonset[2];
};

class BaseController {
public:
    uint8_t pinmask;
    uint8_t* globalmask;

    //virtual void init(uint8_t pins_avail);
    //virtual void print_status(short int cnum);
    //virtual void detect_controllers();
    //virtual void read_state();
    //virtual void fillStatus(struct JoystickStatusStruct *joylist);
    uint8_t get_deviants(uint8_t pins_avail, uint8_t expected);
    void blink_binary(int num, uint8_t bits);
};

#endif /* BASECONTROLLER_H */
