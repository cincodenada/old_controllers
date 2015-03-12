#ifndef BASECONTROLLER_H
#define BASECONTROLLER_H

#include <stdint.h>

#include "Arduino.h"
#include "pin_config.h"
#include "common.h"

#define CNAME_LEN 10
#define PN(n) ((n+1)/2) //Converts -1/1 to 0/1
#define AXIS(n,d) (129+n*2+PN(d))

#define NUM_BUTTONS 16

class BaseController {
public:
    uint8_t button_map[NUM_BUTTONS];

    uint8_t pinmask;
    uint8_t* globalmask;
    char controller_name[CNAME_LEN];

    bool use_3V;

    struct JoystickStatusStruct *JoyStatus;

    BaseController(struct JoystickStatusStruct *JoyStatus, uint8_t* global_pins, char* controller_name);
    virtual void init();
    virtual void setup_pins() = 0;
    virtual void detect_controllers(uint8_t pins_avail) = 0;
    virtual void read_state() = 0;
    virtual void fillStatus(struct JoystickStatusStruct *joylist);
    virtual void fillJoystick(struct JoystickStatusStruct *joystick, uint8_t datamask) = 0;
    void safe_detect_controllers();
    uint8_t get_deviants(uint8_t pins_avail, uint8_t expected);
    void blink_binary(int num, uint8_t bits);
};

#endif /* BASECONTROLLER_H */
