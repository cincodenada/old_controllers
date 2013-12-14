#ifndef NESCONTROLLER_H
#define NESCONTROLLER_H

#include "BaseController.h"

class NESController : public BaseController {
public:
    uint8_t NES_raw_dump[8]; // Temp dump location

    struct JoystickStatusStruct *JoyStatus;

    NESController() {};
    NESController(struct JoystickStatusStruct *, uint8_t *);

    void init();
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
