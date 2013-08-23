/**
 * Gamecube controller to Nintendo 64 adapter
 * by Andrew Brown
 * Rewritten for N64 to HID by Peter Den Hartog
 */

/**
 * To use, hook up the following to the Arduino Duemilanove:
 * Digital I/O 2: N64 serial line
 * All appropriate grounding and power lines
 */

#include "pins_arduino.h"

#define N64_PIN(cnum) (cnum + 2)
#define N64_PIN_DIR DDRD
// these two macros set arduino pin 2 to input or output, which with an
// external 1K pull-up resistor to the 3.3V rail, is like pulling it high or
// low.  These operations translate to 1 op code, which takes 2 cycles
#define N64_HIGH asm volatile ("in __tmp_reg__, %[port]\nand __tmp_reg__, %[invmask]\nout %[port], __tmp_reg__\n"::[invmask] "r" (invmask), [port] "I" (_SFR_IO_ADDR(DDRD)))
#define N64_LOW asm volatile ("in __tmp_reg__, %[port]\nor __tmp_reg__, %[cmask]\nout %[port], __tmp_reg__\n"::[cmask] "r" (cmask), [port] "I" (_SFR_IO_ADDR(DDRD)))
//Assembly stub functions
short int N64_query(char cmask) {
  short int inbit;
  asm volatile ("in %[inbit], %[port]\n"
                "and %[inbit], %[cmask]\n"
                :[inbit] "=r"(inbit)
                :[port] "I" (_SFR_IO_ADDR(PIND)), [cmask] "r" (cmask)
                );
  return inbit;
}

// 8 bytes of data that we get from the controller
struct N64_status_struct {
    // bits: 0, 0, 0, start, y, x, b, a
    unsigned char data1;
    // bits: 1, L, R, Z, Dup, Ddown, Dright, Dleft
    unsigned char data2;
    char stick_x;
    char stick_y;
};
struct N64_status_struct N64_status[4];
char N64_raw_dump[33]; // Temp dump location
char cmask_list[4] = {0x04,0x08,0x10,0x20};

void N64_send(short int cnum, unsigned char *buffer, char length);
void N64_get(short int cnum);
void print_N64_status(short int cnum);
void translate_raw_data(short int cnum);

#include "crc_table.h"

void setup()
{
  Serial.begin(9600);
  
  Serial.println("Initiating controllers");
  
  DualJoystick.setJoyNum(1);

  DualJoystick.useManualSend(true);
  
  short int cnum;

  unsigned char command;
  // Communication with gamecube controller on this pin
  // Don't remove these lines, we don't want to push +5V to the controller
  for(cnum=0; cnum < 4; cnum++) {
    Serial.print("Initiating controller");
    Serial.println(cnum);

    digitalWrite(N64_PIN(cnum), LOW);
    pinMode(N64_PIN(cnum), INPUT);
  
    // Initialize the gamecube controller by sending it a null byte.
    // This is unnecessary for a standard controller, but is required for the
    // Wavebird.
    command = 0;
    noInterrupts();
    
    N64_send(cnum, &command, 1);
  
    // Stupid routine to wait for the gamecube controller to stop
    // sending its response. We don't care what it is, but we
    // can't start asking for status if it's still responding
    int x;
    for (x=0; x<64; x++) {
        // make sure the lines are idle for 64 iterations, should
        // be plenty.
        if (!N64_query(cmask_list[cnum])) { x = 0; }
    }
  
    // Query for the gamecube controller's status. We do this
    // to get the 0 point for the control stick.
    command = 0x01;
    clear_dump();
    N64_send(cnum, &command, 1);
    // read in data and dump it to N64_raw_dump
    N64_get(cnum);
    interrupts();
    translate_raw_data(cnum);
  }

  digitalWrite(N64_PIN(4), LOW);
  pinMode(N64_PIN(4), OUTPUT);
}

void clear_dump() {
  for(int i=0;i<33;i++) {
    N64_raw_dump[i] = 0;
  }
}

void translate_raw_data(short int cnum)
{
    // The get_N64_status function sloppily dumps its data 1 bit per byte
    // into the get_status_extended char array. It's our job to go through
    // that and put each piece neatly into the struct N64_status
    int i;
    memset(&N64_status[cnum], 0, sizeof(N64_status_struct));
    // line 1
    // bits: A, B, Z, Start, Dup, Ddown, Dleft, Dright
    for (i=0; i<8; i++) {
        N64_status[cnum].data1 |= N64_raw_dump[i] ? (0x80 >> i) : 0;
    }
    // line 2
    // bits: 0, 0, L, R, Cup, Cdown, Cleft, Cright
    for (i=0; i<8; i++) {
        N64_status[cnum].data2 |= N64_raw_dump[8+i] ? (0x80 >> i) : 0;
    }
    // line 3
    // bits: DualJoystick x value
    // These are 8 bit values centered at 0x80 (128)
    for (i=0; i<8; i++) {
        N64_status[cnum].stick_x |= N64_raw_dump[16+i] ? (0x80 >> i) : 0;
    }
    for (i=0; i<8; i++) {
        N64_status[cnum].stick_y |= N64_raw_dump[24+i] ? (0x80 >> i) : 0;
    }
}

/**
 * This sends the given byte sequence to the controller
 * length must be at least 1
 * Oh, it destroys the buffer passed in as it writes it
 */
void N64_send(short int cnum, unsigned char *buffer, char length)
{
    // Send these bytes
    char bits;
    
    register unsigned char cmask asm("r3");
    register unsigned char invmask asm("r4");
    
    cmask = cmask_list[cnum];
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
                //7 + 3 + 35 + 3 = 48 cycles = 3us

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

void N64_get(short int cnum)
{
    // listen for the expected 8 bytes of data back from the controller and
    // blast it out to the N64_raw_dump array, one bit per byte for extra speed.
    // Afterwards, call translate_raw_data() to interpret the raw data and pack
    // it into the N64_status struct.

    asm volatile (";Starting to listen");
    unsigned char timeout;
    char bitcount = 32;
    char *bitbin = N64_raw_dump;
    
    char cmask = cmask_list[cnum];

    // Again, using gotos here to make the assembly more predictable and
    // optimization easier (please don't kill me)

    //For this to work, we have to start querying the line during the stop bit
    //So we have < 3us (48c) to get to the N64_query below
read_loop:
    timeout = 0x3f;
    // wait for line to go low
    while (N64_query(cmask)) {
        if (!--timeout)
            return;
    }
    // wait approx 2us and poll the line
    asm volatile (
                  "nop\nnop\nnop\nnop\nnop\n"  
                  "nop\nnop\nnop\nnop\nnop\n"  
                  "nop\nnop\nnop\nnop\nnop\n"  
                  "nop\nnop\nnop\nnop\nnop\n"  
                  "nop\nnop\nnop\nnop\nnop\n"  
                  "nop\nnop\nnop\nnop\nnop\n"  
            );
    *bitbin = N64_query(cmask);
    ++bitbin;
    --bitcount;
    if (bitcount == 0)
        return;

    // wait for line to go high again
    // it may already be high, so this should just drop through
    timeout = 0x3f;
    while (!N64_query(cmask)) {
        if (!--timeout)
            return;
    }
    goto read_loop;
}

void print_N64_status(short int cnum)
{
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

void loop()
{
    int i;
    unsigned char joynum, joypos, command;

    digitalWrite(N64_PIN(4), HIGH);
    for(short int cnum=0; cnum < 4; cnum++) {
      // Command to send to the gamecube
      // The last bit is rumble, flip it to rumble
      // yes this does need to be inside the loop, the
      // array gets mutilated when it goes through N64_send
      command = 0x01;
      clear_dump();

      // don't want interrupts getting in the way
      noInterrupts();
      // send those 3 bytes
      N64_send(cnum, &command, 1);
      // read in data and dump it to N64_raw_dump
      N64_get(cnum);
      // end of time sensitive code
      interrupts();
  
      // translate the data in N64_raw_dump to something useful
      translate_raw_data(cnum);
      
      //Set joystick parameters
      joynum = cnum % 2;
      joypos = cnum / 2;
      
      DualJoystick.setJoyNum(joynum);
      //Update each button
      char mask = 0x01;
      for (i=0; i<8; i++) {
          DualJoystick.button(8-i+joypos*16,N64_status[cnum].data1 & mask ? 1 : 0);
          DualJoystick.button(16-i+joypos*16,N64_status[cnum].data2 & mask ? 1 : 0);
          mask = mask << 1;
      }
    
      //The array is given as approx -81 to 81
      //Joystick funcitons need 0 to 1023
      unsigned int joyx, joyy;
      joyx = max(min((int)N64_status[cnum].stick_x * 6, 511), -512) + 512;
      joyy = max(min((int)N64_status[cnum].stick_y * -6, 511), -512) + 512;
       
      switch(joypos) {
        case 0:
          DualJoystick.X(joyx);
          DualJoystick.Y(joyy);
          break;
        case 1:
          DualJoystick.Z(joyx);
          DualJoystick.Zrotate(joyy);
          break;
        case 2:
          DualJoystick.sliderLeft(joyx);
          DualJoystick.sliderRight(joyy);
          break;
      }
      DualJoystick.send_now();
    }
    digitalWrite(N64_PIN(4), LOW);

    // DEBUG: print it
    print_N64_status(0);
    //print_N64_status(1);
    delay(25);
}
