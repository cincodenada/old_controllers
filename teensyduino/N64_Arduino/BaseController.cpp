#include "BaseController.h"

BaseController::BaseController(struct JoystickStatusStruct *JoyStatus, uint8_t* global_pins, char* controller_name) {
    this->JoyStatus = JoyStatus;
    this->globalmask = global_pins;
    strncpy(this->controller_name, controller_name, CNAME_LEN);
}

void BaseController::printMsg(const char* format, ...) {
    va_list args;
    va_start(args, format);
    if(true) {
        vsnprintf(msg, MSG_LEN, format, args);
        Serial.flush();
        Serial.println(msg);
    }
    va_end(args);
}

void BaseController::init() {
    printMsg("Initiating %s controllers", this->controller_name);

    this->detect_controllers();
    this->setup_pins();

    printMsg("%s Pinmask: %X", this->controller_name, this->pinmask);
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
        inpins = (DATA_IN & (pins_avail << DATA_SHIFT)) >> DATA_SHIFT;
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

