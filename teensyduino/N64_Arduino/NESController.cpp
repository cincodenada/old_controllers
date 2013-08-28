#include "NESController.h"
#include <stdio.h>

NESController::NESController(struct JoystickStatusStruct *JoyStatus) {
    this->JoyStatus = JoyStatus;
}

void NESController::init() {
    Serial.println("Initiating NES controllers");
    digitalWrite(PIN_TRIGGER, LOW);
    pinMode(PIN_TRIGGER, OUTPUT);

    this->detect_controllers();
}

void NESController::clear_dump() {
  for(int i=0;i<8;i++) {
    this->NES_raw_dump[i] = 0;
  }
}

void NESController::detect_controllers() {
    //For now just handle all with NES
    this->pinmask = IO_MASK;
    //For our pins, set N64 flag high (=NES)
    N64_PORT |= this->pinmask << N64_SHIFT;
    //And set SNES flag to low (=NES)
    SNES_PORT &= ~(this->pinmask << SNES_SHIFT);
}

void NESController::read_state() {
    
    digitalWrite(PIN_TRIGGER, HIGH);

    // read in data and dump it to NES_raw_dump
    this->get();

    this->fillStatus(this->JoyStatus);
    digitalWrite(PIN_TRIGGER, LOW);
}

void NESController::get() {
    char curbit = 0;
    char *bitbin = this->NES_raw_dump;

    //Send a 12-us pulse to the latch pin
    LATCH_PORT |= LATCH_MASK;
    delayMicroseconds(12);
    LATCH_PORT &= ~LATCH_MASK;

    //Record response
    while(curbit < 8) {
        //Read value
        *bitbin = DATA_PORT & this->pinmask;
        ++bitbin;
        ++curbit;
        //Send a 12-us 50% duty cycle clock pulse
        delayMicroseconds(6);
        CLOCK_PORT |= CLOCK_MASK;
        delayMicroseconds(6);
        CLOCK_PORT &= ~CLOCK_MASK;
    }
}

void NESController::fillStatus(struct JoystickStatusStruct *joylist) {
    short int pinlist = this->pinmask;
    short int datamask = 0x01;
    short int allpins = IO_MASK;
    int cnum = 0;
    char msg[100];

    while(pinlist) {
        if(pinlist & 0x01) {
            Serial.println("Filling status: ");
            sprintf(msg, "%X %X %X %d", pinlist, allpins, datamask, cnum);
            Serial.println(msg);
            // The get_NES_status function sloppily dumps its data 1 bit per byte
            // into the get_status_extended char array. It's our job to go through
            // that and put each piece neatly into the struct NES_status
            int i;
            char axisnum, axisdir;
            memset(&joylist[cnum], 0, sizeof(JoystickStatusStruct));
            // line 1
            // bits: A, B, Select, Start, Dup, Ddown, Dleft, Dright
            for (i=0; i<8; i++) {
                sprintf(msg, "%X", this->NES_raw_dump[i]);
                Serial.println(msg);
                //If the button is pressed, set the bit
                if(NES_raw_dump[i] & datamask) {
                    joylist[cnum].buttonset[0] |= (0x01 << i);

                    //Emulate a joystick as well, because why not?
                    if(i > 3) {
                        //x axis = 0, y axis = 1
                        axisnum = (i > 5) ? 0 : 1;
                        //down and right = negative
                        axisdir = (0 == i%2) ? AXIS_MAX : AXIS_MIN;
                        
                        joylist[cnum].axis[axisnum] = axisdir;
                    }
                }
            }
        }
        if(allpins & 0x01) { cnum++; }

        allpins >>= 1;
        pinlist >>= 1;
        datamask <<= 1;
    }
}

/*
void NESController::print_status(short int cnum) {
    // bits: A, B, Z, Start, Dup, Ddown, Dleft, Dright
    // bits: 0, 0, L, R, Cup, Cdown, Cleft, Cright
    Serial.println();
    Serial.print("Start: ");
    Serial.println(NES_status[cnum].data1 & 16 ? 1:0);

    Serial.print("Z:     ");
    Serial.println(NES_status[cnum].data1 & 32 ? 1:0);

    Serial.print("B:     ");
    Serial.println(NES_status[cnum].data1 & 64 ? 1:0);

    Serial.print("A:     ");
    Serial.println(NES_status[cnum].data1 & 128 ? 1:0);

    Serial.print("L:     ");
    Serial.println(NES_status[cnum].data2 & 32 ? 1:0);
    Serial.print("R:     ");
    Serial.println(NES_status[cnum].data2 & 16 ? 1:0);

    Serial.print("Cup:   ");
    Serial.println(NES_status[cnum].data2 & 0x08 ? 1:0);
    Serial.print("Cdown: ");
    Serial.println(NES_status[cnum].data2 & 0x04 ? 1:0);
    Serial.print("Cright:");
    Serial.println(NES_status[cnum].data2 & 0x01 ? 1:0);
    Serial.print("Cleft: ");
    Serial.println(NES_status[cnum].data2 & 0x02 ? 1:0);
    
    Serial.print("Dup:   ");
    Serial.println(NES_status[cnum].data1 & 0x08 ? 1:0);
    Serial.print("Ddown: ");
    Serial.println(NES_status[cnum].data1 & 0x04 ? 1:0);
    Serial.print("Dright:");
    Serial.println(NES_status[cnum].data1 & 0x01 ? 1:0);
    Serial.print("Dleft: ");
    Serial.println(NES_status[cnum].data1 & 0x02 ? 1:0);

    Serial.print("Stick X:");
    Serial.println(NES_status[cnum].stick_x, DEC);
    Serial.print("Stick Y:");
    Serial.println(NES_status[cnum].stick_y, DEC);
}
*/
