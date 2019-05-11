#include "NESController.h"
#include <stdio.h>

void NESController::init() {
    BaseController::init();

    this->controller_type = NES;
}

void NESController::setup_pins() {
    //For our pins, set SNES flag to low (=NES)
    for(int i=0; i<NUM_SLOTS; i++) {
        if(pinmask & (0x01 << i)) {
            pinMode(this->slow_pins[i], INPUT_PULLUP);
            digitalWrite(this->s_nes_pins[i], LOW);
        }
    }
}

void NESController::clear_dump() {
    memset(this->raw_dump, 0, 16);
}

void NESController::detect_controllers(uint8_t pins_avail) {
    // At this point any SNES controllers are out of the running
    // Enable the rest as NES controllers, and see which ones
    // pull the DATA line high (and are thus NES controllers)

    for(int i=0; i<NUM_SLOTS; i++) {
        if(pins_avail & (0x01 << i)) {
            printMsg("Detecting NES on pin %d", i);
            pinMode(this->slow_pins[i], INPUT); // Hi-Z so the pulldown works
            digitalWrite(this->slow_pins[i], LOW); // Hi-Z so the pulldown works
            digitalWrite(this->s_nes_pins[i], LOW);
            // We're safe to use LATCH now, since SNES controllers
            // are where they want to be
            pinMode(latch_pins[i], OUTPUT);
            digitalWrite(latch_pins[i], HIGH);
        }
    }


    // Anyone that responds is an NES controller
    this->pinmask = this->get_deviants(pins_avail, 0);

    this->latch(LOW, pins_avail);
}

void NESController::read_state() {
    //digitalWrite(PIN_TRIGGER, HIGH);

    // read in data and dump it to raw_dump
    this->get();

    this->fillStatus(this->JoyStatus);
    //digitalWrite(PIN_TRIGGER, LOW);
}

void NESController::get() {
    this->reset_isr_data();
    this->isr_data.mode = 1;
    this->isr_data.pins = this->slow_pins;
    this->isr_data.end_byte = &this->isr_data.buf[8];
    Timer1.initialize();
    Timer1.attachInterrupt(&this->isr_read, 6);

    // Wait for it to do its thing
    while(this->isr_data.mode == 1) {}

    // Transfer to raw_dump
    for(int i=0; i < 8; i++) {
        this->raw_dump[i] = ~this->isr_data.buf[i];
    }
}

void NESController::isr_read() {
    int mask = 0x01;
    switch(BaseController::isr_data.cur_stage) {
        case 0:
            BaseController::latch(HIGH, 0xF);
            digitalWriteFast(CLOCK_PIN, LOW);
            break;
        case 1:
            // First bit is on latch, so read it
            for(int i=0; i < NUM_SLOTS; i++) {
                if(digitalReadFast(BaseController::isr_data.pins[i])) {
                    *BaseController::isr_data.cur_byte |= mask;
                }
                mask <<= 1;
            }
            break;
        case 3:
            BaseController::latch(LOW, 0xF);
            break;
        case 4:
            // Do nothing
            break;
        case 5:
            if(BaseController::isr_data.cur_byte >= BaseController::isr_data.end_byte) {
                BaseController::isr_data.mode = 2;
                Timer1.detachInterrupt();
            } else {
                digitalWriteFast(CLOCK_PIN, HIGH);
                BaseController::isr_data.cur_byte++;
            }
            break;
        case 6:
            for(int i=0; i < NUM_SLOTS; i++) {
                if(digitalReadFast(BaseController::isr_data.pins[i])) {
                    *BaseController::isr_data.cur_byte |= mask;
                }
                mask <<= 1;
            }
            digitalWriteFast(CLOCK_PIN, LOW);
            // Set back to 4 so it gets incremented to 5
            BaseController::isr_data.cur_stage = 4;
            break;
    }
    BaseController::isr_data.cur_stage++;
}

void NESController::fillJoystick(JoystickStatus *joystick, uint8_t datamask) {
    joystick->clear();

    // line 1
    // bits: A, B, Select, Start, Dup, Ddown, Dleft, Dright
    // (reversed)
    for (int i=0; i<8; i++) {
        printBin(binstr, this->raw_dump[i]);
        printMsg(binstr);
        //If the button is pressed, set the bit
        if(raw_dump[i] & datamask) {
            joystick->buttonset[0] |= (0x80 >> i);

        }
    }
}
