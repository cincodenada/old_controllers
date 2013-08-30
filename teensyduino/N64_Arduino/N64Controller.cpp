#include "N64Controller.h"
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

//Assembly stub functions
//TODO: Investigate if PIND vs PORTD makes anny difference?
short int N64_query(char cmask) {
  short int inbit;
  asm volatile ("in %[inbits], %[port]\n"
                "and %[inbits], %[cmask]\n"
                :[inbits] "=r"(inbit)
                :[port] "I" (_SFR_IO_ADDR(DATA_PORT)), [cmask] "r" (cmask)
                );
  return inbit;
}

N64Controller::N64Controller(struct JoystickStatusStruct *JoyStatus) {
    this->JoyStatus = JoyStatus;
}

void N64Controller::init() {
    Serial.println("Initiating N64 controllers");
    digitalWrite(PIN_TRIGGER, LOW);
    pinMode(PIN_TRIGGER, OUTPUT);

    this->detect_controllers();

    //Set up data port for our guys
    //Express low
    DATA_PORT &= ~(this->pinmask << DATA_SHIFT);
    //Set dir to input (low)
    DATA_DIR &= ~(this->pinmask << DATA_SHIFT);

    unsigned char command;
    // Initialize the gamecube controller by sending it a null byte.
    // This is unnecessary for a standard controller, but is required for the
    // Wavebird.
    command = 0;
    noInterrupts();

    this->send(&command, 1);

    digitalWrite(PIN_TRIGGER, HIGH);
    // Stupid routine to wait for the gamecube controller to stop
    // sending its response. We don't care what it is, but we
    // can't start asking for status if it's still responding
    int x;
    // Set pins to input
    DATA_DIR &= ~(this->pinmask << DATA_SHIFT);
    for (x=0; x<64; x++) {
        // make sure the lines are idle for 64 iterations, should
        // be plenty.
        if (!N64_query((this->pinmask << DATA_SHIFT))) { x = 0; }
    }
    digitalWrite(PIN_TRIGGER, LOW);

    // Query for the gamecube controller's status. We do this
    // to get the 0 point for the control stick.
    this->read_state();

    digitalWrite(PIN_TRIGGER, LOW);
}

void N64Controller::clear_dump() {
  for(int i=0;i<33;i++) {
    this->N64_raw_dump[i] = 0;
  }
}

void N64Controller::detect_controllers() {
    //For now just handle all with N64
    this->pinmask = IO_MASK;
    //For our pins, set N64 flag low (=N64)
    N64_PORT &= ~(this->pinmask << N64_SHIFT);
}

void N64Controller::read_state() {
    //Save the pinmask
    short int old_mask = this->pinmask;

    //Run through our controllers one at a time
    short int pinlist = old_mask;
    short int datamask = 0x01;

    //This is hackish, but will work fine
    while(pinlist) {
        if(pinlist & 0x01) {
            this->pinmask = datamask;
            digitalWrite(PIN_TRIGGER, HIGH);
            noInterrupts();

            unsigned char command = 0x01;
            clear_dump();
            this->send(&command, 1);
            // read in data and dump it to N64_raw_dump
            this->get();

            interrupts();

            this->fillStatus(this->JoyStatus);
            digitalWrite(PIN_TRIGGER, LOW);
        }
        pinlist >>= 1;
        datamask <<= 1;
    }

    //Restore the pinmask
    this->pinmask = old_mask;
}

/**
 * This sends the given byte sequence to the controller
 * length must be at least 1
 * Oh, it destroys the buffer passed in as it writes it
 */
void N64Controller::send(unsigned char *buffer, char length) {
    // Send these bytes
    char bits;
    
    register unsigned char cmask asm("r3");
    register unsigned char invmask asm("r4");
    
    cmask = this->pinmask << DATA_SHIFT;
    invmask = ~cmask;

    // This routine is very carefully timed by examining the assembly output.
    // Do not change any statements, it could throw the timings off
    //
    // We get 16 cycles per microsecond, which should be plenty, but we need to
    // be conservative. Most assembly ops take 1 cycle, but a few take 2
    //
    // I use manually constructed for-loops out of gotos so I have more control
    // over the outputted assembly. I can insert nops where it was impossible
    // with a for loop
    
    asm volatile (";Starting outer for loop");
outer_loop:
    {
        asm volatile (";Starting inner for loop");
        //2 cycles to LD r18, Z
        bits=8; //1 cycle
inner_loop:
        {
            // Starting a bit, set the line low
            asm volatile (";Setting line to low");
            //Set low, in our control-freak way
            N64_LOW; //3 cycles
            
            asm volatile (";branching");
            //This comparison takes 7 cycles
            if (*buffer >> 7) {
                //2 cycles to branch to here
                asm volatile (";Bit is a 1");
                // 1 bit
                // remain low for 1us, then go high for 3us
                // nop block 1
                asm volatile ("nop\nnop\nnop\nnop\n"); //4 cycles nop
                
                asm volatile (";Setting line to high");
                N64_HIGH; //3 ops, 3 cycles (in/and/out)
                
                //7 + 2 + 4 + 3 = 16 cycles = 1us

                // nop block 2
                // we'll wait only 2us to sync up with both conditions
                // at the bottom of the if statement
                asm volatile ("nop\nnop\nnop\nnop\nnop\n"  
                              "nop\nnop\nnop\nnop\nnop\n"  
                              "nop\nnop\nnop\nnop\nnop\n"  
                              "nop\nnop\nnop\nnop\nnop\n"  
                              "nop\nnop\nnop\nnop\nnop\n"  
                              "nop\nnop\nnop\nnop\n"  
                              ); //6*5 - 1 = 29 nops
                //And then 2 cycles to RJMP to the --bits
                //For 31 cycles = 2us - 1c, leaving 1us + 1c remaining
            } else {
                //1 cycle to not branch + 2 cycles to RJMP = 3 cycles to get here
                asm volatile (";Bit is a 0");
                // 0 bit
                // remain low for 3us, then go high for 1us
                // nop block 3
                asm volatile ("nop\nnop\nnop\nnop\nnop\n"  
                              "nop\nnop\nnop\nnop\nnop\n"  
                              "nop\nnop\nnop\nnop\nnop\n"  
                              "nop\nnop\nnop\nnop\nnop\n"  
                              "nop\nnop\nnop\nnop\nnop\n"  
                              "nop\nnop\nnop\nnop\nnop\n"  
                              "nop\nnop\nnop\nnop\n"  
                              ); //7*5 - 1 = 34 nops

                asm volatile (";Setting line to high");
                N64_HIGH; //3 cycles
                //7 + 3 + 34 + 3 = 47 cycles = 3us - 1c

                asm volatile ("; end of conditional branch, need to wait 1us more before next bit");
                //Fall through, with 1us + 1c remaining
            }
            // end of the if, the line is high and needs to remain
            // high for exactly 16 more cycles, regardless of the previous
            // branch path

            asm volatile (";finishing inner loop body");
            --bits;  //1 cycle
            if (bits != 0) { //1 cycle to not branch
                // nop block 4
                // this block is why a for loop was impossible
                asm volatile ("nop\nnop\nnop\nnop\nnop\n"  
                              "nop\nnop\nnop\nnop\n");  //9c
                // rotate bits
                asm volatile (";rotating out bits");
                *buffer <<= 1;  //1c

                goto inner_loop;  //2c RJMP
                // = 14c + 3c at the top = 17c = 1us + 1c extra
              } // fall out of inner loop
        }
        //1 more cycle to branch to here
        asm volatile (";continuing outer loop");
        // In this case: the inner loop exits and the outer loop iterates,
        // there are /exactly/ 16 cycles taken up by the necessary operations.
        // So no nops are needed here (that was lucky!)
        
        // Damn, that extra cycle hurts here, we're at 17
        // Perhaps fudge it by a cycle and remove nops after the low bit to make up?
        // Investigate
        //2 cycles to ST
        --length; //1c
        if (length != 0) { //1c to compare
            ++buffer; //2c
            goto outer_loop; //2c
        } // fall out of outer loop
    }

    // send a single stop (1) bit
    // nop block 5
    asm volatile ("nop\nnop\nnop\nnop\n");
    N64_LOW;
    // wait 1 us, 16 cycles, then raise the line 
    // 16-3=13
    // nop block 6
    asm volatile ("nop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\n"  
                  "nop\nnop\nnop\n");
    N64_HIGH;
}

void N64Controller::get() {
    // listen for the expected 8 bytes of data back from the controller and
    // blast it out to the N64_raw_dump array, one bit per byte for extra speed.
    // Afterwards, call translate_raw_data() to interpret the raw data and pack
    // it into the N64_status struct.

    asm volatile (";Starting to listen");
    unsigned char timeout;
    char bitcount = 32;
    char *bitbin = this->N64_raw_dump;
    
    //char cmask = this->pinmask;
    short int cmask = this->pinmask << DATA_SHIFT;
    short int invmask = ~cmask;

    DATA_DIR &= invmask;

    // Again, using gotos here to make the assembly more predictable and
    // optimization easier (please don't kill me)

    //For this to work, we have to start querying the line during the stop bit
    //So we have < 3us (48c) to get to the N64_query below
read_loop:
    timeout = 0x3f;
    // wait for line to go low

    while((PIND & cmask) > 0) {
        if (!--timeout)
            return;
    }
    // wait approx 2us and poll the line
    asm volatile (
                  "nop\nnop\nnop\nnop\nnop\n"  
                  "nop\nnop\nnop\nnop\nnop\n"  
                  "nop\nnop\nnop\nnop\nnop\n"  
                  "nop\nnop\nnop\nnop\nnop\n"  
                  //"nop\nnop\nnop\nnop\nnop\n"  
                  //"nop\nnop\nnop\nnop\nnop\n"  
                  "nop\n"
            );
    *bitbin = PIND & cmask;
    ++bitbin;
    --bitcount;
    if (bitcount == 0)
        return;

    // wait for line to go high again
    // it may already be high, so this should just drop through
    timeout = 0x3f;
    while ((PIND & cmask) == 0) {
        if (!--timeout)
            return;
    }
    goto read_loop;
}

void N64Controller::fillStatus(struct JoystickStatusStruct *joylist) {
    short int pinlist = this->pinmask;
    short int datamask = 0x01;
    short int allpins = IO_MASK;
    int cnum = 0;
    char msg[100];
    char mult = AXIS_MAX/82;

    while(pinlist) {
        if(pinlist & 0x01) {
            Serial.println("Filling status: ");
            sprintf(msg, "%X %X %X %d", pinlist, allpins, datamask, cnum);
            Serial.println(msg);
            // The get_N64_status function sloppily dumps its data 1 bit per byte
            // into the get_status_extended char array. It's our job to go through
            // that and put each piece neatly into the struct N64_status
            int i;
            char xaxis = 0;
            char yaxis = 0;

            memset(&joylist[cnum], 0, sizeof(JoystickStatusStruct));
            // line 1
            // bits: A, B, Z, Start, Dup, Ddown, Dleft, Dright
            for (i=0; i<8; i++) {
                sprintf(msg, "%X%X%X%X", this->N64_raw_dump[i],this->N64_raw_dump[i+8],this->N64_raw_dump[i+16],this->N64_raw_dump[i+24]);
                Serial.println(msg);
                joylist[cnum].buttonset[0] |= (this->N64_raw_dump[i] & datamask) ? (0x01 << i) : 0;
                joylist[cnum].buttonset[1] |= (this->N64_raw_dump[8+i] & datamask) ? (0x01 << i) : 0;
                xaxis |= (this->N64_raw_dump[16+i] & datamask) ? (0x01 << i) : 0;
                yaxis |= (this->N64_raw_dump[24+i] & datamask) ? (0x01 << i) : 0;
            }
            // Safely translate the axis values from [-82, 82] to [AXIS_MIN, AXIS_MAX]
            joylist[cnum].axis[0] = max(min((int)xaxis * (mult), AXIS_MAX), AXIS_MIN);
            joylist[cnum].axis[1] = max(min((int)yaxis * (-mult), AXIS_MAX), AXIS_MIN);
        }
        if(allpins & 0x01) { cnum++; }

        allpins >>= 1;
        pinlist >>= 1;
        datamask <<= 1;
    }
}

/*
void N64Controller::print_status(short int cnum) {
    // bits: A, B, Z, Start, Dup, Ddown, Dleft, Dright
    // bits: 0, 0, L, R, Cup, Cdown, Cleft, Cright
    Serial.println();
    Serial.print("Start: ");
    Serial.println(N64_status[cnum].data1 & 16 ? 1:0);

    Serial.print("Z:     ");
    Serial.println(N64_status[cnum].data1 & 32 ? 1:0);

    Serial.print("B:     ");
    Serial.println(N64_status[cnum].data1 & 64 ? 1:0);

    Serial.print("A:     ");
    Serial.println(N64_status[cnum].data1 & 128 ? 1:0);

    Serial.print("L:     ");
    Serial.println(N64_status[cnum].data2 & 32 ? 1:0);
    Serial.print("R:     ");
    Serial.println(N64_status[cnum].data2 & 16 ? 1:0);

    Serial.print("Cup:   ");
    Serial.println(N64_status[cnum].data2 & 0x08 ? 1:0);
    Serial.print("Cdown: ");
    Serial.println(N64_status[cnum].data2 & 0x04 ? 1:0);
    Serial.print("Cright:");
    Serial.println(N64_status[cnum].data2 & 0x01 ? 1:0);
    Serial.print("Cleft: ");
    Serial.println(N64_status[cnum].data2 & 0x02 ? 1:0);
    
    Serial.print("Dup:   ");
    Serial.println(N64_status[cnum].data1 & 0x08 ? 1:0);
    Serial.print("Ddown: ");
    Serial.println(N64_status[cnum].data1 & 0x04 ? 1:0);
    Serial.print("Dright:");
    Serial.println(N64_status[cnum].data1 & 0x01 ? 1:0);
    Serial.print("Dleft: ");
    Serial.println(N64_status[cnum].data1 & 0x02 ? 1:0);

    Serial.print("Stick X:");
    Serial.println(N64_status[cnum].stick_x, DEC);
    Serial.print("Stick Y:");
    Serial.println(N64_status[cnum].stick_y, DEC);
}
*/
