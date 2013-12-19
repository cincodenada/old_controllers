#ifndef BASECONTROLLER_H
#define BASECONTROLLER_H

#include <stdint.h>

#include "Arduino.h"
#include "pin_config.h"
#include "common.h"

#define CNAME_LEN 10

// 8 bytes of data that we get from the controller
struct JoystickStatusStruct {
    signed short int axis[3];
    uint8_t buttonset[2];
};

class BaseController {
public:
    uint8_t raw_dump[]; // Temp dump location

    uint8_t pinmask;
    uint8_t* globalmask;
    char controller_name[CNAME_LEN];

    struct JoystickStatusStruct *JoyStatus;

    BaseController(struct JoystickStatusStruct *JoyStatus, uint8_t* global_pins, char* controller_name);
    virtual void init();
    virtual void setup_pins() = 0;
    virtual void detect_controllers() = 0;
    virtual void read_state() = 0;
    virtual void fillStatus(struct JoystickStatusStruct *joylist);
    virtual void fillJoystick(struct JoystickStatusStruct *joystick, uint8_t datamask) = 0;
    uint8_t get_deviants(uint8_t pins_avail, uint8_t expected);
    void blink_binary(int num, uint8_t bits);
};

#endif /* BASECONTROLLER_H */
