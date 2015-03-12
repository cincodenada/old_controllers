#include "BaseController.h"

BaseController::BaseController(struct JoystickStatusStruct *JoyStatus, uint8_t* global_pins, char* controller_name) {
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

void BaseController::use_3V() {
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

void BaseController::fillStatus(struct JoystickStatusStruct *joylist) {
    uint8_t pinlist = this->pinmask;
    uint8_t datamask = 0x01;
    uint8_t allpins = *globalmask;
    int cnum = 0;

    while(pinlist) {
        if(pinlist & 0x01) {
            printMsg("%lu: Filling status for %s:\r\n%X %X %X %d", millis(), 
                    this->controller_name, pinlist, allpins, datamask, cnum);

            this->fillJoystick(&joylist[cnum], datamask);
            joylist[cnum]->controller_type = this->controller_type;
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
void BaseController::blink_binary(int num, uint8_t bits) {
    int mask = 1 << (bits-1);
    digitalWrite(PIN_TRIGGER, HIGH);
    delay(300);
    while(mask) {
        digitalWrite(PIN_TRIGGER, LOW);
        delay(100);
        digitalWrite(PIN_TRIGGER, HIGH);
        delay(100 + 200 * (num & mask));
        mask >>= 1;
    }
    digitalWrite(PIN_TRIGGER, LOW);
    delay(300);
    digitalWrite(PIN_TRIGGER, HIGH);
}

void BaseController::translate_buttons(
    struct JoystickStatusStruct *dest,
    struct JoystickStatusStruct *source,
    uint8_t *button_map
) {
    memcpy(dest, source, sizeof(struct JoystickStatusStruct));
    // Reset the buttons
    // Leave axes in place
    // unless we specifically overwrite them
    dest->buttonset[0] = 0;
    dest->buttonset[1] = 0;

    int num_bytes = NUM_BUTTONS/8;
    for(int byte=0; byte < num_bytes; byte++) {
        for(int bit=0; bit < 8; bit++) {
            int btn_num = button_map[i + byte*8];
            if(btn_num > 128) {
                // Deal with axes etc
            } else if(btn_num) {
                if(source->buttonset[byte] && (0x01 << bit)) {
                    btn_num -= 1;
                    int dest_byte = btn_num/8;
                    int dest_bit = btn_num%8;
                    dest->buttonset[dest_byte] |= (0x01 << dest_bit);
                }
            }
        }
    }
}
