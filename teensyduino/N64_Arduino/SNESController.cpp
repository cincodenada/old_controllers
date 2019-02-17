#include "SNESController.h"
#include <stdio.h>

void SNESController::init() {
    BaseController::init();

    this->controller_type = SNES;
}

void SNESController::setup_pins() {
    //For our pins, set SNES flag to high (=SNES)
    for(int i=0; i<NUM_CONTROLLERS; i++) {
        digitalWrite(this->s_nes_pins[i], HIGH);
    }
}

void SNESController::clear_dump() {
    memset(this->raw_dump, 0, 16);
}

void SNESController::detect_controllers(uint8_t pins_avail) {
    // This algorithm was after some experimentation.
    // With a weak pull-down on DATA, we can divine things safely
    // SNES must be detected/eliminated first

    // Pin SNES controller Vdd to GND
    for(int i=0; i<NUM_CONTROLLERS; i++) {
        if(pins_avail & (0x01 << i)) {
            digitalWrite(this->s_nes_pins[i], LOW);
        }
    }

    // SNES controllers will pull DATA lines to their Vdd,
    // which is now our LOW. NES Controllers and empty slots
    // are pulling to 5V, so anything LOW is an SNES controller
    this->pinmask = this->get_deviants(pins_avail, 1);
}

void SNESController::read_state() {
    //digitalWrite(PIN_TRIGGER, HIGH);

    // read in data and dump it to raw_dump
    this->get();

    this->fillStatus(this->JoyStatus);
    //digitalWrite(PIN_TRIGGER, LOW);
}

void SNESController::get() {
    this->reset_isr_data();
    this->isr_data.pins = this->slow_pins;
    this->isr_data.end_byte = &this->isr_data.buf[16];
    Timer1.initialize();
    Timer1.attachInterrupt(&this->isr_read, 12);

    // Wait for it to do its thing
    delay(1);
}

void SNESController::isr_read() {
    if(BaseController::isr_data.cur_stage < 2)  {
        if(BaseController::isr_data.cur_stage == 0) {
            digitalWriteFast(LATCH_PIN, HIGH);
            digitalWriteFast(CLOCK_PIN, LOW);
        } else if(BaseController::isr_data.cur_stage == 1) {
            digitalWriteFast(LATCH_PIN, LOW);
        }
        BaseController::isr_data.cur_stage++;
    } else {
        if(BaseController::isr_data.cur_stage == 2) {
            int mask = 0x01;
            for(int i=0; i < NUM_CONTROLLERS; i++) {
                if(digitalReadFast(BaseController::isr_data.pins[i])) {
                    *BaseController::isr_data.cur_byte |= mask;
                }
                mask <<= 1;
            }
            digitalWriteFast(CLOCK_PIN, HIGH);
            BaseController::isr_data.cur_stage++;
        } else {
            digitalWriteFast(CLOCK_PIN, LOW);
            BaseController::isr_data.cur_byte++;
            if(BaseController::isr_data.cur_byte >= BaseController::isr_data.end_byte) {
                Timer1.detachInterrupt();
            }
            BaseController::isr_data.cur_stage = 2;
        }
    }
}

void SNESController::fillJoystick(JoystickStatus *joystick, uint8_t datamask) {
    int i, setnum;
    char ctldata[100] = "";
    joystick->clear();

    // line 1
    // bits: B, Y, Select, Start, Dup, Ddown, Dleft, Dright
    // bits2: A, X, L, R, NCx4
    // (reversed)
    for (i=0; i<8; i++) {
        printBin(ctldata, this->raw_dump[i]);
        ctldata[8] = ' ';
        printBin(ctldata + 9, this->raw_dump[i+8]);
        printMsg(ctldata);

        // Bit offset, 0 and then 8
        for (setnum=0; setnum<2; setnum++) {
            if(raw_dump[i + setnum*8] & datamask) {
                joystick->buttonset[setnum] |= (0x80 >> i);
            }
        }
    }
}
