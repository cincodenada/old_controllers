#ifndef SNESCONTROLLER_H
#define SNESCONTROLLER_H

#include "BaseController.h"

class SNESController : public BaseController {
public:
    char SNES_raw_dump[16]; // Temp dump location

    struct JoystickStatusStruct *JoyStatus;

    SNESController() {};
    SNESController(struct JoystickStatusStruct *JoyStatus);

    void init(char pins_avail);
    void print_status(short int cnum);
    void detect_controllers(char pins_avail);
    void get();
    void translate_raw_data();
    void clear_dump();

    short int pinmask;
    short int datamask;

    void read_state();
    void fillStatus(struct JoystickStatusStruct *joylist);
};

#endif /* SNESCONTROLLER_H */
