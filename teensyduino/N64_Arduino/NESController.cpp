#include "NESController.h"
#include <stdio.h>

void NESController::setup_pins() {
    //For our pins, set SNES flag to low (=NES)
    SNES_PORT &= ~(this->pinmask << SNES_SHIFT);
}

void NESController::clear_dump() {
  for(int i=0;i<8;i++) {
    this->raw_dump[i] = 0;
  }
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
    SNES_PORT &= ~(pins_avail << SNES_SHIFT);

    //Send clock pulses to clear out the shift registers
    //9 to be safe, and allow time to fall
    for(int i=0;i<9;i++) { this->pulse_clock(); }

    //Lines pulled low are either SNES or NES controllers
    //We assume SNES are already eliminated, so
    //the remaining controllers are NES controllers
    //So invert and mask
    this->pinmask = this->get_deviants(pins_avail, 1);
}

void NESController::read_state() {
    
    //digitalWrite(PIN_TRIGGER, HIGH);

    // read in data and dump it to raw_dump
    this->get();
    delay(1);

    this->fillStatus(this->JoyStatus);
    //digitalWrite(PIN_TRIGGER, LOW);
}

void NESController::pulse_latch() {
    //Send a 12-us pulse to the latch pin
    LATCH_PORT |= LATCH_MASK;
    asm volatile ("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"  
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"  
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  ); //8*24 = 12us
    LATCH_PORT &= ~LATCH_MASK;
}

void NESController::pulse_clock() {
    //Send a 12-us 50% duty cycle clock pulse
    asm volatile ("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"  
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  ); //8*12 = 3us
    CLOCK_PORT |= CLOCK_MASK;
    asm volatile ("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"  
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
                  ); //8*12 = 3us
    CLOCK_PORT &= ~CLOCK_MASK;
}

void NESController::get() {
    short int curbit = 8;
    uint8_t *bitbin = this->raw_dump;

    pulse_latch();

    //Record response
    while(curbit) {
        //Read value
        *bitbin = (~DATA5_IN & (this->pinmask << DATA5_SHIFT)) >> DATA5_SHIFT;
        ++bitbin;
        --curbit;
        pulse_clock();
    }
}

void NESController::fillJoystick(struct JoystickStatusStruct *joystick, uint8_t datamask) {
    int i;
    signed short int axisnum, axisdir;
    char ctldata[50] = "";
    memset(joystick, 0, sizeof(JoystickStatusStruct));
    // line 1
    // bits: A, B, Select, Start, Dup, Ddown, Dleft, Dright
    // (reversed)
    for (i=0; i<8; i++) {
        snprintf(ctldata, 50, "%s%X ", ctldata, this->raw_dump[i]);
        //If the button is pressed, set the bit
        if(raw_dump[i] & datamask) {
            joystick->buttonset[0] |= (0x80 >> i);

            //Emulate a joystick as well, because why not?
            if(i > 3) {
                //x axis = 0, y axis = 1
                axisnum = (i > 5) ? 0 : 1;
                //down and right = positive
                axisdir = (0 == i%2) ? AXIS_MIN : AXIS_MAX;
                
                joystick->axis[axisnum] = axisdir;
            }
        }
    }
    printMsg(ctldata);
}
