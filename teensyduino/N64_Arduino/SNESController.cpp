#include "SNESController.h"
#include <stdio.h>

void blink_binary(int num, char bits) {
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

SNESController::SNESController(struct JoystickStatusStruct *JoyStatus) {
    this->JoyStatus = JoyStatus;
}

void SNESController::init(char pins_avail) {
    Serial.println("Initiating SNES controllers");

    this->detect_controllers(pins_avail);

    //For our pins, set N64 flag high (=S/NES)
    N64_PORT |= this->pinmask << N64_SHIFT;
    //And set SNES flag to high (=SNES)
    SNES_PORT |= this->pinmask << SNES_SHIFT;

    snprintf(msg, MSG_LEN, "SNES Pinmask: %X", this->pinmask);
    Serial.println(msg);
}

void SNESController::clear_dump() {
  for(int i=0;i<16;i++) {
    this->SNES_raw_dump[i] = 0;
  }
}

void SNESController::detect_controllers(char pins_avail) {
    //NES and SNES pull low on idle, so check for that
    //(N64 maintains high, and we use pull-up)
    char N64_prev, SNES_prev;

    //Save the states
    N64_prev = N64_PORT;
    SNES_prev = SNES_PORT;

    //Try setting all ports to SNES
    //For our pins, set N64 flag high (=S/NES)
    N64_PORT |= pins_avail << N64_SHIFT;
    //And set SNES flag to high (=SNES)
    SNES_PORT |= pins_avail << SNES_SHIFT;
    delay(5000);

    //Lines pulled low are SNES controllers
    //So invert and mask
    this->pinmask = (~DATA_IN & (pins_avail << DATA_SHIFT)) >> DATA_SHIFT;

    //Restore states
    N64_PORT = N64_prev;
    SNES_PORT = SNES_prev;
}

void SNESController::read_state() {
    
    //digitalWrite(PIN_TRIGGER, HIGH);

    // read in data and dump it to SNES_raw_dump
    this->get();

    this->fillStatus(this->JoyStatus);
    //digitalWrite(PIN_TRIGGER, LOW);
}

void SNESController::get() {
    char curbit = 16;
    char *bitbin = this->SNES_raw_dump;

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
                  ); //8*12 = 6us
    LATCH_PORT &= ~LATCH_MASK;

    //Record response
    while(curbit) {
        //Read value
        *bitbin = ~DATA_IN & this->pinmask;
        ++bitbin;
        --curbit;
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
                      ); //8*12 = 6us
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
                      ); //6*5 - 1 = 29 nops
        CLOCK_PORT &= ~CLOCK_MASK;
    }
}

void SNESController::fillStatus(struct JoystickStatusStruct *joylist) {
    short int pinlist = this->pinmask;
    short int datamask = 0x01;
    short int allpins = IO_MASK;
    int cnum = 0;

    while(pinlist) {
        if(pinlist & 0x01) {
            Serial.println("Filling status: ");
            snprintf(msg, MSG_LEN, "%X %X %X %d", pinlist, allpins, datamask, cnum);
            Serial.println(msg);
            // The get_SNES_status function sloppily dumps its data 1 bit per byte
            // into the get_status_extended char array. It's our job to go through
            // that and put each piece neatly into the struct SNES_status
            int i;
            signed short int axisnum, axisdir;
            memset(&joylist[cnum], 0, sizeof(JoystickStatusStruct));
            // line 1
            // bits: B, Y, Select, Start, Dup, Ddown, Dleft, Dright
            // bits2: A, X, L, R, NCx4
            // (reversed)
            for (i=0; i<8; i++) {
                snprintf(msg, MSG_LEN, "%X", this->SNES_raw_dump[i]);
                Serial.println(msg);
                //If the button is pressed, set the bit
                if(SNES_raw_dump[i] & datamask) {
                    joylist[cnum].buttonset[0] |= (0x80 >> i);

                    //Emulate a joystick as well, because why not?
                    if(i < 4) {
                        //x axis = 0, y axis = 1
                        axisnum = (i < 2) ? 0 : 1;
                        //down and right = negative
                        axisdir = (0 == i%2) ? AXIS_MAX : AXIS_MIN;
                        
                        joylist[cnum].axis[axisnum] = axisdir;
                    }
                }
                if((i > 3) && (SNES_raw_dump[i+8] & datamask)) {
                    //If it's the others, we've got the 
                    //SNES buttons to deal with
                    joylist[cnum].buttonset[1] |= (0x80 >> i);
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
void SNESController::print_status(short int cnum) {
    // bits: A, B, Z, Start, Dup, Ddown, Dleft, Dright
    // bits: 0, 0, L, R, Cup, Cdown, Cleft, Cright
    Serial.println();
    Serial.print("Start: ");
    Serial.println(SNES_status[cnum].data1 & 16 ? 1:0);

    Serial.print("Z:     ");
    Serial.println(SNES_status[cnum].data1 & 32 ? 1:0);

    Serial.print("B:     ");
    Serial.println(SNES_status[cnum].data1 & 64 ? 1:0);

    Serial.print("A:     ");
    Serial.println(SNES_status[cnum].data1 & 128 ? 1:0);

    Serial.print("L:     ");
    Serial.println(SNES_status[cnum].data2 & 32 ? 1:0);
    Serial.print("R:     ");
    Serial.println(SNES_status[cnum].data2 & 16 ? 1:0);

    Serial.print("Cup:   ");
    Serial.println(SNES_status[cnum].data2 & 0x08 ? 1:0);
    Serial.print("Cdown: ");
    Serial.println(SNES_status[cnum].data2 & 0x04 ? 1:0);
    Serial.print("Cright:");
    Serial.println(SNES_status[cnum].data2 & 0x01 ? 1:0);
    Serial.print("Cleft: ");
    Serial.println(SNES_status[cnum].data2 & 0x02 ? 1:0);
    
    Serial.print("Dup:   ");
    Serial.println(SNES_status[cnum].data1 & 0x08 ? 1:0);
    Serial.print("Ddown: ");
    Serial.println(SNES_status[cnum].data1 & 0x04 ? 1:0);
    Serial.print("Dright:");
    Serial.println(SNES_status[cnum].data1 & 0x01 ? 1:0);
    Serial.print("Dleft: ");
    Serial.println(SNES_status[cnum].data1 & 0x02 ? 1:0);

    Serial.print("Stick X:");
    Serial.println(SNES_status[cnum].stick_x, DEC);
    Serial.print("Stick Y:");
    Serial.println(SNES_status[cnum].stick_y, DEC);
}
*/
