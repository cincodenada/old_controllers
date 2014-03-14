#include "N64Controller.h"
#include <stdio.h>

//Assembly stub functions
//This function queries and masks the N64 ports
short int N64_query(uint8_t cmask) {
  uint8_t inbit;
  asm volatile ("in %[inbits], %[port]\n"
                "and %[inbits], %[cmask]\n"
                :[inbits] "=r"(inbit)
                :[port] "I" (_SFR_IO_ADDR(DATA_IN)), [cmask] "r" (cmask)
                );
  return inbit;
}

void N64Controller::init() {
    BaseController::init();

    // Query for the gamecube controller's status. We do this
    // to get the 0 point for the control stick.
    // TODO: Does this actually...do anything?
    this->read_state();
}
void N64Controller::setup_pins() {
    //For our pins, set N64 flag low (=N64)
    N64_PORT &= ~(this->pinmask << N64_SHIFT);
    //We don't care about S/NES
}

void N64Controller::clear_dump() {
  for(int i=0;i<33;i++) {
    this->raw_dump[i] = 0;
  }
}

void N64Controller::detect_controllers(uint8_t pins_avail) {
    //NES and SNES pull low on idle, so check for that
    //(N64 maintains high, and we use pull-up)
    
    //For our pins, set N64 flag low (=N64)
    N64_PORT &= ~(pins_avail << N64_SHIFT);
    //SNES/NES port doesn't matter
    
    //Just send the ID command and see who answers
    //This also initializes some controllers (Wavebird, I guess?)
    uint8_t command;
    command = 0x00;

    //Ports are set to Hi-Z here
    noInterrupts();
    this->send(&command, 1);
    interrupts();

    //At this point we're pull-up input
    //Wait for lines to remain high (quiet) for a bit (64 iterations) 
    //And note which ones pull low at any point
    //We don't care what they're actually saying

    int x;
    short int inpins;
    this->pinmask = 0;
    for (x=0; x<64; x++) {
        inpins = N64_query(pins_avail << DATA_SHIFT) >> DATA_SHIFT;
        //If any of the lines fall low
        if (inpins != pins_avail) {
            //Reset the counter
            x = 0; 
            //And take note of which ones talked back
            this->pinmask |= ((~inpins) & IO_MASK);
        }
    }

    //Put data ports back to pull-up
    DATA_PORT |= (~this->pinmask & IO_MASK) << DATA_SHIFT;
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

            uint8_t command = 0x01;
            clear_dump();
            this->send(&command, 1);
            // read in data and dump it to raw_dump
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
void N64Controller::send(uint8_t *buffer, uint8_t length) {
    // Send these bytes
    uint8_t bits;
    
    register uint8_t cmask asm("r3");
    register uint8_t invmask asm("r4");
    
    cmask = this->pinmask << DATA_SHIFT;
    invmask = ~cmask;

    //Set input to Hi-Z
    //Since we're using N64_HIGH/LOW macros
    DATA_PORT &= invmask;

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

    //Set back to input
    N64_HIGH;
}

void N64Controller::get() {
    // listen for the expected 8 bytes of data back from the controller and
    // blast it out to the raw_dump array, one bit per byte for extra speed.
    // Afterwards, call translate_raw_data() to interpret the raw data and pack
    // it into the N64_status struct.

    asm volatile (";Starting to listen");
    uint8_t timeout;
    uint8_t bitcount = 32;
    uint8_t *bitbin = this->raw_dump;
    
    //uint8_t cmask = this->pinmask;
    short int cmask = this->pinmask << DATA_SHIFT;
    short int invmask = ~cmask;

    //Ensure we're in Hi-Z
    DATA_DIR &= invmask;

    // Again, using gotos here to make the assembly more predictable and
    // optimization easier (please don't kill me)

    //For this to work, we have to start querying the line during the stop bit
    //So we have < 3us (48c) to get to the N64_query below
read_loop:
    timeout = 0x3f;
    // wait for line to go low

    while((DATA_IN & cmask) > 0) {
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
    *bitbin = DATA_IN & cmask;
    ++bitbin;
    --bitcount;
    if (bitcount == 0)
        return;

    // wait for line to go high again
    // it may already be high, so this should just drop through
    timeout = 0x3f;
    while ((DATA_IN & cmask) == 0) {
        if (!--timeout)
            return;
    }
    goto read_loop;
}

void N64Controller::fillJoystick(struct JoystickStatusStruct *joystick, uint8_t datamask) {
    int mult = AXIS_MAX/90;
    //TODO: Figure out why higher numbers are going negative

    int i;
    int8_t xaxis = 0;
    int8_t yaxis = 0;

    memset(joystick, 0, sizeof(JoystickStatusStruct));
    // line 1
    // bits: A, B, Z, Start, Dup, Ddown, Dleft, Dright
    for (i=0; i<8; i++) {
        printMsg("%X%X%X%X", this->raw_dump[i],this->raw_dump[i+8],this->raw_dump[i+16],this->raw_dump[i+24]);
        joystick->buttonset[0] |= (this->raw_dump[i] & datamask) ? (0x80 >> i) : 0;
        joystick->buttonset[1] |= (this->raw_dump[8+i] & datamask) ? (0x80 >> i) : 0;
        xaxis |= (this->raw_dump[16+i] & datamask) ? (0x80 >> i) : 0;
        yaxis |= (this->raw_dump[24+i] & datamask) ? (0x80 >> i) : 0;
    }

    // Safely translate the axis values from [-82, 82] to [AXIS_MIN, AXIS_MAX]
    joystick->axis[0] = max(min((int)xaxis * (mult), AXIS_MAX), AXIS_MIN);
    joystick->axis[1] = max(min((int)yaxis * (-mult), AXIS_MAX), AXIS_MIN);
}
