#include "SNESController.h"
#include <stdio.h>

// Joystick button, by bit position
uint8_t SNESController::init_button_map[NUM_BUTTONS] = {
    1,3,7,8,129,130,131,132,
    2,4,5,6,0,0,0,0
};

void SNESController::init() {
    BaseController::init();

    memcpy(this->button_map, init_button_map, NUM_BUTTONS);
}

void SNESController::setup_pins() {
    //For our pins, set SNES flag to high (=SNES)
    SNES_PORT |= this->pinmask << SNES_SHIFT;
}

void SNESController::clear_dump() {
  for(int i=0;i<16;i++) {
    this->raw_dump[i] = 0;
  }
}

void SNESController::detect_controllers(uint8_t pins_avail) {
    //Try setting remaining ports to NES
    //For our pins, set SNES flag to low (=NES)
    SNES_PORT &= ~(pins_avail << SNES_SHIFT);

    //Lines pulled low are SNES controllers
    //So invert and mask
    this->pinmask = this->get_deviants(pins_avail, 1);
}

void SNESController::read_state() {
    
    //digitalWrite(PIN_TRIGGER, HIGH);

    // read in data and dump it to raw_dump
    this->get();
    delay(1);

    this->fillStatus(this->JoyStatus);
    //digitalWrite(PIN_TRIGGER, LOW);
}

void SNESController::pulse_latch() {
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

void SNESController::pulse_clock() {
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

void SNESController::get() {
    short int curbit = 16;
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

void SNESController::fillJoystick(struct JoystickStatusStruct *joystick, uint8_t datamask) {
    int i, offset;
    signed short int axisnum, axisdir;
    char ctldata[100] = "";
    memset(joystick, 0, sizeof(JoystickStatusStruct));
    // line 1
    // bits: B, Y, Select, Start, Dup, Ddown, Dleft, Dright
    // bits2: A, X, L, R, NCx4
    // (reversed)
    for (i=0; i<8; i++) {
        snprintf(ctldata, 100, "%s%X %X (", ctldata, this->raw_dump[i], this->raw_dump[i+8]);

        // Bit offset, 0 and then 8
        for (offset=0; offset<=8; offset+=8) {
            //If the button is pressed, set the bit
            int btn_num = button_map[i + offset];
            snprintf(ctldata, 100, "%s%d ", ctldata, btn_num);
            if(btn_num && (raw_dump[i + offset] & datamask)) {
                btn_num -= 1; // Adjust to 0-based
                int byte_num = btn_num/8;
                int bit_num = btn_num%8;
                //Emulate a joystick as well, because why not?
                if(btn_num >= 0x80) {
                    int dir_num = btn_num & 0x0F;
                    //x axis = 0, y axis = 1
                    axisnum = (dir_num > 1) ? 0 : 1;
                    //down and right = positive
                    axisdir = (0 == dir_num%2) ? AXIS_MIN : AXIS_MAX;
                    
                    joystick->axis[axisnum] = axisdir;
                } else {
                    joystick->buttonset[byte_num] |= (0x80 >> bit_num);
                }
            }
        }
        snprintf(ctldata, 100, "%s)", ctldata);
    }
    printMsg(ctldata);
}
