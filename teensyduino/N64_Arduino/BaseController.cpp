#include "BaseController.h"

BaseController::BaseController(struct JoystickStatusStruct *JoyStatus, uint8_t* global_pins, char* controller_name) {
    this->JoyStatus = JoyStatus;
    this->globalmask = global_pins;
    strncpy(this->controller_name, controller_name, CNAME_LEN);
}

void BaseController::init() {
    snprintf(msg, MSG_LEN, "Initiating %s controllers", this->controller_name);
    Serial.println(msg);

    this->detect_controllers();
    this->setup_pins();

    snprintf(msg, MSG_LEN, "NES Pinmask: %X", this->pinmask);
    Serial.println(msg);
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
        snprintf(msg, MSG_LEN, "Inpins %X, pins_avail %X, pinmask %X...", inpins, pins_avail, pinmask);
        Serial.println(msg);
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

