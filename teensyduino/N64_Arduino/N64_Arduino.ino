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
// low.
// They have to be all complicated in order to accomodate arguments.  They are 3 instructions, 1 cycle each
#define N64_HIGH(cmask) asm volatile ("in __tmp_reg__, %[port]\nand __tmp_reg__, r4\nout %[port], __tmp_reg__\n"::[port] "I" (_SFR_IO_ADDR(DDRD)))
#define N64_LOW(cmask) asm volatile ("in __tmp_reg__, %[port]\nor __tmp_reg__, r3\nout %[port], __tmp_reg__\n"::[port] "I" (_SFR_IO_ADDR(DDRD)))
#define N64_QUERY(cmask) (PIND & cmask)

// 8 bytes of data that we get from the controller
struct N64_status_struct {
    // bits: 0, 0, 0, start, y, x, b, a
    unsigned char data1;
    // bits: 1, L, R, Z, Dup, Ddown, Dright, Dleft
    unsigned char data2;
    char stick_x;
    char stick_y;
};
struct N64_status_struct N64_status[2];
char N64_raw_dump[2][33]; // 1 received bit per byte
char cmask_list[2] = {0x04,0x08};


void N64_send(short int cnum, unsigned char *buffer, char length);
void N64_get(short int cnum);
void print_N64_status(short int cnum);
void translate_raw_data(short int cnum);

#include "crc_table.h"

void setup()
{
  Serial.begin(9600);

  Joystick.useManualSend(true);

  // Communication with gamecube controller on this pin
  // Don't remove these lines, we don't want to push +5V to the controller
  digitalWrite(N64_PIN(0), LOW);
  digitalWrite(N64_PIN(1), LOW);
  pinMode(N64_PIN(0), INPUT);
  pinMode(N64_PIN(1), INPUT);

  // Initialize the gamecube controller by sending it a null byte.
  // This is unnecessary for a standard controller, but is required for the
  // Wavebird.
  unsigned char initialize = 0x00;
  noInterrupts();
  N64_send(0, &initialize, 1);
  N64_send(1, &initialize, 1);

  // Stupid routine to wait for the gamecube controller to stop
  // sending its response. We don't care what it is, but we
  // can't start asking for status if it's still responding
  int x;
  for (x=0; x<64; x++) {
      // make sure the lines are idle for 64 iterations, should
      // be plenty.
      for(short int cnum=0; cnum < 2; cnum++) {
        if (!N64_QUERY(cmask_list[cnum])) { x = 0; }
      }
  }

  // Query for the gamecube controller's status. We do this
  // to get the 0 point for the control stick.
  unsigned char command[] = {0x01};
  N64_send(0, command, 1);
  N64_send(1, command, 1);
  // read in data and dump it to N64_raw_dump
  N64_get(0);
  N64_get(1);
  interrupts();
  translate_raw_data(0);  
  translate_raw_data(1);  
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
        N64_status[cnum].data1 |= N64_raw_dump[cnum][i] ? (0x80 >> i) : 0;
    }
    // line 2
    // bits: 0, 0, L, R, Cup, Cdown, Cleft, Cright
    for (i=0; i<8; i++) {
        N64_status[cnum].data2 |= N64_raw_dump[cnum][8+i] ? (0x80 >> i) : 0;
    }
    // line 3
    // bits: joystick x value
    // These are 8 bit values centered at 0x80 (128)
    for (i=0; i<8; i++) {
        N64_status[cnum].stick_x |= N64_raw_dump[cnum][16+i] ? (0x80 >> i) : 0;
    }
    for (i=0; i<8; i++) {
        N64_status[cnum].stick_y |= N64_raw_dump[cnum][24+i] ? (0x80 >> i) : 0;
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
    
    bool bit;
    
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
        bits=8;
inner_loop:
        {
            // Starting a bit, set the line low
            asm volatile (";Setting line to low");
            //Set low, in our control-freak way
            N64_LOW(cmask);
            
            asm volatile (";branching");
            if (*buffer >> 7) {
                asm volatile (";Bit is a 1");
                // 1 bit
                // remain low for 1us, then go high for 3us
                // nop block 1
                asm volatile ("nop\nnop\nnop\nnop\nnop\n");
                
                asm volatile (";Setting line to high");
                N64_HIGH(cmask); //3 ops, 3 cycles (in/and/out)

                // nop block 2
                // we'll wait only 2us to sync up with both conditions
                // at the bottom of the if statement
                asm volatile ("nop\nnop\nnop\nnop\nnop\n"  
                              "nop\nnop\nnop\nnop\nnop\n"  
                              "nop\nnop\nnop\nnop\nnop\n"  
                              "nop\nnop\nnop\nnop\nnop\n"  
                              "nop\nnop\nnop\nnop\nnop\n"  
                              "nop\nnop\nnop\nnop\nnop\n"  
                              );

            } else {
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
                              "nop\nnop\nnop\nnop\nnop\n"  
                              "nop\n");

                asm volatile (";Setting line to high");
                N64_HIGH(cmask);

                // wait for 1us
                asm volatile ("; end of conditional branch, need to wait 1us more before next bit");
                
            }
            // end of the if, the line is high and needs to remain
            // high for exactly 16 more cycles, regardless of the previous
            // branch path

            asm volatile (";finishing inner loop body");
            --bits;
            if (bits != 0) {
                // nop block 4
                // this block is why a for loop was impossible
                asm volatile ("nop\nnop\nnop\nnop\nnop\n"  
                              "nop\nnop\nnop\nnop\n");
                // rotate bits
                asm volatile (";rotating out bits");
                *buffer <<= 1;

                goto inner_loop;
            } // fall out of inner loop
        }
        asm volatile (";continuing outer loop");
        // In this case: the inner loop exits and the outer loop iterates,
        // there are /exactly/ 16 cycles taken up by the necessary operations.
        // So no nops are needed here (that was lucky!)
        --length;
        if (length != 0) {
            ++buffer;
            goto outer_loop;
        } // fall out of outer loop
    }

    // send a single stop (1) bit
    // nop block 5
    asm volatile ("nop\nnop\nnop\nnop\n");
    N64_LOW(cmask);
    // wait 1 us, 16 cycles, then raise the line 
    // 16-2=14
    // nop block 6
    asm volatile ("nop\nnop\nnop\nnop\nnop\n"
                  "nop\nnop\nnop\nnop\nnop\n"  
                  "nop\nnop\nnop\nnop\n");
    N64_HIGH(cmask);

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
    char *bitbin = N64_raw_dump[cnum];
    unsigned char cmask = cmask_list[cnum];

    // Again, using gotos here to make the assembly more predictable and
    // optimization easier (please don't kill me)
read_loop:
    timeout = 0x3f;
    // wait for line to go low
    while (N64_QUERY(cmask)) {
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
    *bitbin = N64_QUERY(cmask);
    ++bitbin;
    --bitcount;
    if (bitcount == 0)
        return;

    // wait for line to go high again
    // it may already be high, so this should just drop through
    timeout = 0x3f;
    while (!N64_QUERY(cmask)) {
        if (!--timeout)
            return;
    }
    goto read_loop;

}

void print_N64_status(short int cnum)
{
    int i;
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
    unsigned char data, addr;

    for(short int cnum=0; cnum < 2; cnum++) {
      // Command to send to the gamecube
      // The last bit is rumble, flip it to rumble
      // yes this does need to be inside the loop, the
      // array gets mutilated when it goes through N64_send
      unsigned char command[] = {0x01};

      // don't want interrupts getting in the way
      noInterrupts();
      // send those 3 bytes
      N64_send(cnum, command, 1);
      // read in data and dump it to N64_raw_dump
      N64_get(cnum);
      // end of time sensitive code
      interrupts();
  
      // translate the data in N64_raw_dump to something useful
      translate_raw_data(cnum);
  
      //Update each button
      char mask = 0x01;
      for (i=0; i<8; i++) {
          Joystick.button(8-i+cnum*16,N64_status[cnum].data1 & mask ? 1 : 0);
          Joystick.button(16-i+cnum*16,N64_status[cnum].data2 & mask ? 1 : 0);
          mask = mask << 1;
      }
    
      //The array is given as approx -81 to 81
      //Joystick funcitons need 0 to 1023
      unsigned int joyx, joyy;
      joyx = max(min((int)N64_status[cnum].stick_x * 6, 511), -512) + 512;
      joyy = max(min((int)N64_status[cnum].stick_y * 6, 511), -512) + 512;
       
      switch(cnum) {
        case 0:
          Joystick.X(joyx);
          Joystick.Y(joyy);
          break;
        case 1:
          Joystick.Z(joyx);
          Joystick.Zrotate(joyy);
          break;
        case 2:
          Joystick.sliderLeft(joyx);
          Joystick.sliderRight(joyy);
          break;
      }
    }

    Joystick.send_now();

    // DEBUG: print it
    //print_N64_status(0);
    //print_N64_status(1);
    delay(25);
}


