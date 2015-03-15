#include "BaseController.h"

BaseController::BaseController(JoystickStatus *JoyStatus, uint8_t* global_pins, char* controller_name) {
    this->JoyStatus = JoyStatus;
    this->globalmask = global_pins;
    strncpy(this->controller_name, controller_name, CNAME_LEN);
}

void BaseController::init() {
    printMsg("Initiating %s controllers", this->controller_name);

    this->safe_detect_controllers();
    this->setup_pins();

    printMsg("%s Pinmask: %X", this->controller_name, this->pinmask);
}

bool BaseController::use_3V() {
    // If newer than N64, use 3.3V
    return (this->controller_type >= N64);
}

void BaseController::safe_detect_controllers() {
    uint8_t SNES_prev;
    uint8_t pins_avail = ~(*globalmask) & IO_MASK;

    printMsg("Searching for %s on %X...", this->controller_name, pins_avail);

    //Save the states
    SNES_prev = SNES_PORT;

    this->detect_controllers(pins_avail);
    *globalmask |= this->pinmask;

    printMsg("Pinmasks: SNES=%X", SNES_PORT);

    //Restore states
    SNES_PORT = SNES_prev;
}

void BaseController::fillStatus(JoystickStatus *joylist) {
    uint8_t pinlist = this->pinmask;
    uint8_t datamask = 0x01;
    uint8_t allpins = *globalmask;
    int cnum = 0;

    while(pinlist) {
        if(pinlist & 0x01) {
            printMsg("%lu: Filling status for %s:\r\n%X %X %X %d", millis(), 
                    this->controller_name, pinlist, allpins, datamask, cnum);

            this->fillJoystick(&joylist[cnum], datamask);
            joylist[cnum].controller_type = this->controller_type;
        }
        if(allpins & 0x01) { cnum++; }

        allpins >>= 1;
        pinlist >>= 1;
        datamask <<= 1;
    }
}

uint8_t BaseController::get_deviants(uint8_t pins_avail, uint8_t expected) {
    int x, resets = 0;
    uint8_t inpins, exp_mask;
    uint8_t pinmask = 0;
    exp_mask = expected ? pins_avail : 0;
    for (x=0; x<64; x++) {
        inpins = this->use_3V()
            ? (DATA3_IN & (pins_avail << DATA3_SHIFT)) >> DATA3_SHIFT
            : (DATA5_IN & (pins_avail << DATA5_SHIFT)) >> DATA5_SHIFT;
        //If any of the lines fall low
        if (inpins != exp_mask) {
            //Reset the counter
            x = 0; 
            //And take note of which ones talked back
            pinmask |= (expected ? (~inpins & pins_avail) : inpins);

            resets++;
            if(resets > 10) { break; }
        }
        printMsg("Inpins %X, pins_avail %X, pinmask %X...", inpins, pins_avail, pinmask);
    }
    return pinmask; 
}
