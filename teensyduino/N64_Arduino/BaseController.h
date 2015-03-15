#ifndef BASECONTROLLER_H
#define BASECONTROLLER_H

#include <stdint.h>

#include "Arduino.h"
#include "pin_config.h"
#include "common.h"

#define CNAME_LEN 10
#define PN(n) ((n+1)/2) //Converts -1/1 to 0/1
#define AXIS(n,d) (129+n*2+PN(d))
#define HAT(x,y) (192+(y+1)*3+(x+1))

class BaseController {
public:
    uint8_t pinmask;
    uint8_t* globalmask;
    char controller_name[CNAME_LEN];
    controller_type_t controller_type;

    struct JoystickStatusStruct *JoyStatus;

    BaseController(struct JoystickStatusStruct *JoyStatus, uint8_t* global_pins, char* controller_name);
    virtual void init();
    virtual void setup_pins() = 0;
    virtual void detect_controllers(uint8_t pins_avail) = 0;
    virtual void read_state() = 0;
    virtual void fillStatus(struct JoystickStatusStruct *joylist);
    virtual void fillJoystick(struct JoystickStatusStruct *joystick, uint8_t datamask) = 0;
    void safe_detect_controllers();
    bool use_3V();
    uint8_t get_deviants(uint8_t pins_avail, uint8_t expected);
};

#endif /* BASECONTROLLER_H */
