#ifndef BASECONTROLLER_H
#define BASECONTROLLER_H

#include <stdint.h>

#include "Arduino.h"
#include "pin_config.h"
#include "common.h"
#include "JoystickStatus.h"
#include "TimerOne.h"

#define CNAME_LEN 10
// Maximum data an interrupt will need to read/write
#define TBUFSIZE 32

struct interrupt_data_struct {
    // Add a pad byte cause we might try to
    // read one past the end sometimes
    volatile uint8_t buf[TBUFSIZE+1];
    volatile uint8_t *cur_byte;
    volatile uint8_t *end_byte;
    volatile uint8_t cur_bit;
    volatile uint8_t cur_stage;
    volatile uint8_t cur_val;
    uint8_t read_bits;

    // Mode: 0 = writing, 1 = reading, 2 = done
    volatile uint8_t mode;

    const uint8_t *pins;
    uint8_t cur_pin;

    volatile bool done;
};

class BaseController {
public:
    uint8_t pinmask = 0;
    uint8_t* globalmask;
    char controller_name[CNAME_LEN];
    controller_type_t controller_type;
    const uint8_t fast_pins[NUM_CONTROLLERS] = { FAST_PINS };
    const uint8_t slow_pins[NUM_CONTROLLERS] = { SLOW_PINS };
    const uint8_t s_nes_pins[NUM_CONTROLLERS] = { S_NES_PINS };

    JoystickStatus *JoyStatus;

    BaseController(JoystickStatus *JoyStatus, uint8_t* global_pins, const char* controller_name);
    virtual void init();
    virtual void setup_pins() = 0;
    virtual void detect_controllers(uint8_t pins_avail) = 0;
    virtual void read_state() = 0;
    virtual void fillStatus(JoystickStatus *joylist);
    virtual void fillJoystick(JoystickStatus *joystick, uint8_t datamask) = 0;
    void safe_detect_controllers();
    bool is_fast();
    const uint8_t* data_pins();
    static void latch(uint8_t val, uint8_t pins);
    uint8_t get_deviants(uint8_t pins_avail, uint8_t expected);
    uint8_t get_deviants(const uint8_t* pins, uint8_t pins_avail, uint8_t expected);

    static void reset_isr_data();

    static interrupt_data_struct isr_data;
};

#endif /* BASECONTROLLER_H */
