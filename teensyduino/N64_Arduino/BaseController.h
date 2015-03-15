#ifndef BASECONTROLLER_H
#define BASECONTROLLER_H

#include <stdint.h>

#include "Arduino.h"
#include "pin_config.h"
#include "common.h"
#include "JoystickStatus.h"

#define CNAME_LEN 10

class BaseController {
public:
    uint8_t pinmask;
    uint8_t* globalmask;
    char controller_name[CNAME_LEN];
    controller_type_t controller_type;

    JoystickStatus *JoyStatus;

    BaseController(JoystickStatus *JoyStatus, uint8_t* global_pins, char* controller_name);
    virtual void init();
    virtual void setup_pins() = 0;
    virtual void detect_controllers(uint8_t pins_avail) = 0;
    virtual void read_state() = 0;
    virtual void fillStatus(JoystickStatus *joylist);
    virtual void fillJoystick(JoystickStatus *joystick, uint8_t datamask) = 0;
    void safe_detect_controllers();
    bool use_3V();
    uint8_t get_deviants(uint8_t pins_avail, uint8_t expected);
};

#endif /* BASECONTROLLER_H */
