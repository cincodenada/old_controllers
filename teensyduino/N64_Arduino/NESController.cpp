#include "NESController.h"
#include <stdio.h>

void NESController::init() {
    BaseController::init();

    this->controller_type = NES;
}

void NESController::setup_pins() {
    //For our pins, set SNES flag to low (=NES)
    for(int i=0; i<NUM_CONTROLLERS; i++) {
        digitalWrite(this->slow_pins[i], LOW);
    }
}

void NESController::clear_dump() {
    memset(this->raw_dump, 0, 16);
}

//Warning! SNES detection must be run *before* this,
//otherwise it wil gobble up SNES controllers as well
//Also: it seems NES controllers will be high until after
//the first query cycle, so we have to send clock pulses
//And that's probably not great for "disabled" 
//SNES controllers anyway
void NESController::detect_controllers(uint8_t pins_avail) {
    //Try setting remaining ports to NES
    //For our pins, set SNES flag to low (=NES)
    for(int i=0; i<NUM_CONTROLLERS; i++) {
        if(pins_avail & (0x01 << i)) {
            digitalWrite(this->slow_pins[i], LOW);
        }
    }

    //Read to clear out any shift registers
    Timer1.initialize();
    Timer1.attachInterrupt(this->isr_read, 6);
    delay(1);
    Timer1.detachInterrupt();

    //Lines pulled low are either SNES or NES controllers
    //We assume SNES are already eliminated, so
    //the remaining controllers are NES controllers
    this->pinmask = this->get_deviants(pins_avail, 1);
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
    this->isr_data.pins = this->slow_pins;
    this->isr_data.end_byte = &this->isr_data.buf[16];
    Timer1.initialize();
    Timer1.attachInterrupt(&this->isr_read, 12);

    // Wait for it to do its thing
    delay(1);
}

void NESController::isr_read() {
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
            if(BaseController::isr_data.cur_byte >= BaseController::isr_data.end_byte) {
                Timer1.detachInterrupt();
            }
            BaseController::isr_data.cur_stage = 2;
        }
    }
}

void NESController::fillJoystick(JoystickStatus *joystick, uint8_t datamask) {
    char ctldata[50] = "";
    joystick->clear();

    // line 1
    // bits: A, B, Select, Start, Dup, Ddown, Dleft, Dright
    // (reversed)
    for (int i=0; i<8; i++) {
        snprintf(ctldata, 50, "%s%X ", ctldata, this->raw_dump[i]);
        //If the button is pressed, set the bit
        if(raw_dump[i] & datamask) {
            joystick->buttonset[0] |= (0x80 >> i);

        }
    }
    printMsg(ctldata);
}
