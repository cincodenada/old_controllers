#ifndef NESCONTROLLER_H
#define NESCONTROLLER_H

#include "BaseController.h"

class NESController : public BaseController {
public:
    uint8_t raw_dump[8]; // Temp dump location
    static uint8_t init_button_map[NUM_BUTTONS];

    NESController(struct JoystickStatusStruct *JoyStatus, uint8_t* global_pins, char* controller_name)
        : BaseController(JoyStatus, global_pins, controller_name) {};

    void init();
    void setup_pins();
    void print_status(short int cnum);
    void detect_controllers(uint8_t pins_avail);
    void get();
    void translate_raw_data();
    void clear_dump();

    void pulse_latch();
    void pulse_clock();

    void read_state();
    void fillJoystick(struct JoystickStatusStruct *joystick, uint8_t datamask);
};

#endif /* NESCONTROLLER_H */
