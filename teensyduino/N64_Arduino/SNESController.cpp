#include "SNESController.h"
#include <stdio.h>

void SNESController::init() {
    BaseController::init();

    this->controller_type = SNES;
}

void SNESController::setup_pins() {
    //For our pins, set SNES flag to high (=SNES)
    for(int i=0; i<NUM_SLOTS; i++) {
        if(pinmask & (0x01 << i)) {
            pinMode(slow_pins[i], INPUT_PULLUP);
            digitalWrite(s_nes_pins[i], HIGH);
        }
    }
}

void SNESController::clear_dump() {
    memset(this->raw_dump, 0, 16);
}

void SNESController::detect_controllers(uint8_t pins_avail) {
    // This algorithm was after some experimentation.
    // With a weak pull-down on DATA, we can divine things safely
    // SNES must be detected/eliminated first

    for(int i=0; i<NUM_SLOTS; i++) {
        if(pins_avail & (0x01 << i)) {
            pinMode(slow_pins[i], INPUT); // Hi-Z so the pulldown works
            digitalWrite(slow_pins[i], LOW); // Hi-Z so the pulldown works
            pinMode(s_nes_pins[i], OUTPUT);
            digitalWrite(s_nes_pins[i], HIGH);
        }
    }
    
    // Anyone that pulls DATA high is SNES
    this->pinmask = this->get_deviants(pins_avail, 0);
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
    this->isr_data.mode = 1;
    this->isr_data.pins = slow_pins;
    this->isr_data.end_byte = &this->isr_data.buf[16];
    Timer1.initialize();
    Timer1.attachInterrupt(&this->isr_read, 6);

    // Wait for it to do its thing
    while(this->isr_data.mode == 1) {}

    // Transfer to raw_dump
    for(int i=0; i < 16; i++) {
        this->raw_dump[i] = ~this->isr_data.buf[i];
    }
}

void SNESController::isr_read() {
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
