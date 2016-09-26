#include "N64Controller.h"
#include <stdio.h>

void N64Controller::init() {
    BaseController::init();

    this->controller_type = N64;

    // Query for the gamecube controller's status. We do this
    // to get the 0 point for the control stick.
    // TODO: Does this actually...do anything?
    this->read_state();
}
void N64Controller::setup_pins() {
    //Don't need to do anything, we don't care about S/NES
}

void N64Controller::clear_dump() {
  for(int i=0;i<33;i++) {
    this->raw_dump[i] = 0;
  }
}

void N64Controller::detect_controllers(uint8_t pins_avail) {
    //NES and SNES pull low on idle, so check for that
    //(N64 maintains high, and we use pull-up)
    
    //SNES/NES port doesn't matter
    
    //Just send the ID command and see who answers
    //This also initializes some controllers (Wavebird, I guess?)
    uint8_t command;
    command = 0x01;

    for(int i=0; i < NUM_CONTROLLERS; i++) {
        if(!(pins_avail & (0x01 << i))) { continue; }

        this->pinmask = 0x01;

        printMsg("Sending command for controller %d...", i);
        while(1) {
            //this->send(i, &command, 1);
            //cls();
            this->read_state();
            delay(1);
        }

        int x;
        uint8_t cur_pin = this->fast_pins[i];
        printMsg("Waiting for response...");
        for (x=0; x<64; x++) {
            if(!digitalReadFast(cur_pin)) {
                printMsg("Got 0, resetting...");
                //Reset the counter
                x = 0;
                //And take note of which ones talked back
                this->pinmask |= (0x01 << i);
            }
        }
    }
}

void N64Controller::read_state() {
    //Run through our controllers one at a time
    for(int i=0; i<NUM_CONTROLLERS; i++) {
        if(!(this->pinmask & (0x01 << i))) { continue; }


        this->reset_isr_data();
        this->isr_data.cur_pin = this->fast_pins[i];
        this->isr_data.buf[0] = 0x01;
        this->isr_data.end_byte = this->isr_data.buf;
        this->isr_data.read_bits = 32;
        pinMode(this->isr_data.cur_pin, OUTPUT);
        analogWriteResolution(3);
        Timer1.initialize();
        digitalWrite(PIN_TRIGGER, HIGH);
        Timer1.attachInterrupt(&this->isr_read, 1);
        // Spin our wheels
        volatile uint16_t j=0;
        while(this->isr_data.mode == 0) { j++; }
        Timer1.detachInterrupt();
        printMsg("Blooped for %d loops", j);

        uint8_t bits = 0;
        uint8_t max_loops = 40;
        noInterrupts();
        while(bits <= this->isr_data.read_bits) {
            volatile uint8_t val = HIGH;
            int loops = 0;
            long int low_len = 0, high_len = 0;
            while(val == HIGH && loops < max_loops) {
                val = digitalReadFast(BaseController::isr_data.cur_pin);
                loops++;
            }
            if(loops == max_loops) {
                digitalWrite(PIN_TRIGGER, LOW);
                printMsg("No response from controller!");
                break;
            }
            loops = 0;
            while(val == LOW && loops < max_loops) {
                val = digitalReadFast(BaseController::isr_data.cur_pin);
                low_len++;
                loops++;
            }
            if(loops == max_loops) {
                digitalWrite(PIN_TRIGGER, LOW);
                printMsg("Hung waiting for LOW");
                break;
            }
            loops = 0;
            while(val == HIGH && loops < max_loops) {
                val = digitalReadFast(BaseController::isr_data.cur_pin);
                high_len++;
                loops++;
            }
            if(loops == max_loops) {
                digitalWrite(PIN_TRIGGER, LOW);
                printMsg("Hung waiting for HIGH");
                break; 
            }
            *BaseController::isr_data.cur_byte = (high_len > low_len);
            BaseController::isr_data.cur_byte++;
            bits++;
        }
        interrupts();

        //memcpy(raw_dump, (void*)this->isr_data.buf, TBUFSIZE);
        //this->fillStatus(this->JoyStatus);
        digitalWrite(PIN_TRIGGER, LOW);
    }
}

void N64Controller::isr_read() {
    noInterrupts();
    analogWrite(A14, BaseController::isr_data.mode);
    if(BaseController::isr_data.mode == 0) {
        switch(BaseController::isr_data.cur_stage) {
        case 0:
            /*
            printMsg("cur_byte: %x, end_byte: %x, cur_bit: %.2x",
                BaseController::isr_data.cur_byte,
                BaseController::isr_data.end_byte,
                BaseController::isr_data.cur_bit
            );
            */
            digitalWriteFast(BaseController::isr_data.cur_pin, LOW);
            // Reset bit mask if we finished the previous byte
            if(BaseController::isr_data.cur_bit == 0) {
                BaseController::isr_data.cur_bit = 0x80;
                BaseController::isr_data.cur_byte++;
            }
            BaseController::isr_data.cur_stage++;
            break;
        case 1:
            if(BaseController::isr_data.cur_byte > BaseController::isr_data.end_byte
                || (*BaseController::isr_data.cur_byte & BaseController::isr_data.cur_bit)) {
                digitalWriteFast(BaseController::isr_data.cur_pin, HIGH);
            }
            BaseController::isr_data.cur_stage++;
            break;
        case 2:
            if(BaseController::isr_data.cur_byte > BaseController::isr_data.end_byte) {
                Timer1.detachInterrupt();
                // Reset byte pointers
                BaseController::isr_data.cur_byte = BaseController::isr_data.buf;
                BaseController::isr_data.end_byte = &BaseController::isr_data.buf[BaseController::isr_data.read_bits-1];
                pinMode(BaseController::isr_data.cur_pin, INPUT_PULLUP);
                // Declare done
                BaseController::isr_data.mode = 1;
            }
            BaseController::isr_data.cur_stage++;
            break;
        case 3:
            digitalWriteFast(BaseController::isr_data.cur_pin, HIGH);
            // Set up to be back to zero after increment
            BaseController::isr_data.cur_stage = 0;
            BaseController::isr_data.cur_bit >>= 1;
            break;
        }
    } else {
        switch(BaseController::isr_data.cur_stage) {
        case 0:
            // Wait for line to be pulled low
            //if(digitalReadFast(BaseController::isr_data.cur_pin) == LOW) {
                BaseController::isr_data.cur_stage++;
            //}
            break;
        case 1:
            BaseController::isr_data.cur_stage++;
            break;
        case 2:
           *BaseController::isr_data.cur_byte = digitalReadFast(BaseController::isr_data.cur_pin);
           BaseController::isr_data.cur_byte++;
           BaseController::isr_data.cur_stage++;
           break;
        case 3:
           /*
            printMsg("cur_byte: %x, end_byte: %x, value: %d",
                BaseController::isr_data.cur_byte,
                BaseController::isr_data.end_byte,
                *(BaseController::isr_data.cur_byte-1)
            );
            */
            if(BaseController::isr_data.cur_byte >= BaseController::isr_data.end_byte) {
            }
            BaseController::isr_data.cur_stage = 0;
            break;
        }
    }
    interrupts();
}

void N64Controller::isr_write() {
    if(BaseController::isr_data.cur_stage == 0)  {
        digitalWriteFast(BaseController::isr_data.cur_pin, LOW);
        BaseController::isr_data.cur_stage++;
    } else if(BaseController::isr_data.cur_stage == 1) {
        digitalWriteFast(
            BaseController::isr_data.cur_pin,
            *BaseController::isr_data.cur_byte & (0x80 >> BaseController::isr_data.cur_bit)
        );
        BaseController::isr_data.cur_stage++;
    } else if(BaseController::isr_data.cur_stage == 2) {
        BaseController::isr_data.cur_stage++;
    } else if(BaseController::isr_data.cur_stage == 3) {
        digitalWriteFast(BaseController::isr_data.cur_pin, HIGH);
        if(BaseController::isr_data.cur_bit == 7) {
            BaseController::isr_data.cur_byte++;
            BaseController::isr_data.cur_bit = 0;
            if(BaseController::isr_data.cur_byte >= BaseController::isr_data.end_byte) {
                // Declare done and detach
                BaseController::isr_data.mode = 2;
                pinMode(BaseController::isr_data.cur_pin, INPUT_PULLUP);
                Timer1.detachInterrupt();
            }
        } else {
            BaseController::isr_data.cur_bit++;
        }
        BaseController::isr_data.cur_stage = 0;
    }
}

/**
 * This sends the given byte sequence to the controller
 * length must be at least 1
 * Oh, it destroys the buffer passed in as it writes it
 */
void N64Controller::send(uint8_t pin, uint8_t *buffer, uint8_t length) {
    printMsg("Clearing ISR data...");
    this->reset_isr_data();
    this->isr_data.cur_pin = this->fast_pins[pin];
    printMsg("Setting pin mode");
    pinMode(this->isr_data.cur_pin, OUTPUT);
    printMsg("Initializing timer...");
    Timer1.initialize();
    digitalWrite(LED_PIN, LOW);
    printMsg("Attaching interupt...");
    digitalWrite(LED_PIN, HIGH);
    interrupts();
    Timer1.attachInterrupt(&this->isr_write, 1);
    printMsg("Off we go!");
    // Spin our wheels
    volatile int i=0;
    //TODO: This only seems to work if we take a lot of time between loops?
    //Like do a printMsg or a delay()?
    //But we can't do that if we're watching for a response...
    while(this->isr_data.mode < 2) { i++; delay(1); }
    printMsg("Blooped for %d loops", i);
    //printMsg("ISR mode: %d", this->isr_data.mode);
}
/*
    // Send these bytes
    uint8_t bits;
    
    register uint8_t cmask asm("r3");
    register uint8_t invmask asm("r4");
    
    cmask = this->pinmask << DATA3_SHIFT;
    invmask = ~cmask;

    N64_OUT;
    N64_HIGH;

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
    N64_IN;
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
    short int cmask = this->pinmask << DATA3_SHIFT;
    short int invmask = ~cmask;

    //Set to input, pull-up
    N64_HIGH;
    N64_IN;

    // Again, using gotos here to make the assembly more predictable and
    // optimization easier (please don't kill me)

    //For this to work, we have to start querying the line during the stop bit
    //So we have < 3us (48c) to get to the N64_query below
read_loop:
    timeout = 0x3f;
    // wait for line to go low

    while((DATA3_IN & cmask) > 0) {
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
    *bitbin = DATA3_IN & cmask;
    ++bitbin;
    --bitcount;
    if (bitcount == 0)
        return;

    // wait for line to go high again
    // it may already be high, so this should just drop through
    timeout = 0x3f;
    while ((DATA3_IN & cmask) == 0) {
        if (!--timeout)
            return;
    }
    goto read_loop;
}
*/
void N64Controller::fillJoystick(JoystickStatus *joystick, uint8_t datamask) {
    int i, setnum;
    int8_t xaxis = 0;
    int8_t yaxis = 0;
    joystick->clear();

    // line 1
    // bits: A, B, Z, Start, Dup, Ddown, Dleft, Dright
    for (i=0; i<8; i++) {
        printMsg("%.2X%.2X%.2X%.2X",
            this->raw_dump[i],
            this->raw_dump[i+8],
            this->raw_dump[i+16],
            this->raw_dump[i+24]
        );
        // Fill the buttonsets with the first two bites
        for (setnum=0; setnum<2; setnum++) {
            //If the button is pressed, set the bit
            if(raw_dump[i + setnum*8] & datamask) {
                joystick->buttonset[setnum] |= (0x80 >> i);
            }
        }
        // Fill the axes with the next two
        xaxis |= (this->raw_dump[16+i] & datamask) ? (0x80 >> i) : 0;
        yaxis |= (this->raw_dump[24+i] & datamask) ? (0x80 >> i) : 0;
    }

    // Safely translate the axis values from [-N64_AXIS_MAX, N64_AXIS_MAX] to [AXIS_MIN, AXIS_MAX]
    joystick->axis[0] = this->safe_axis(xaxis);
    joystick->axis[1] = -this->safe_axis(yaxis);
}

signed short int N64Controller::safe_axis(int8_t rawval) {
    return max(min(
        max(min(rawval, N64_AXIS_MAX), -N64_AXIS_MAX) * (AXIS_MAX/N64_AXIS_MAX), 
    AXIS_MAX), AXIS_MIN);
}
