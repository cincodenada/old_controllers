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
#include <stdio.h>
#include <stdint.h>

#include "pins_arduino.h"

#include "pin_config.h"
#include "common.h"
#include "crc_table.h"
#include "N64Controller.h"
#include "SNESController.h"
#include "NESController.h"

#define NUMCTL 3
#define BITS 8

struct JoystickStatusStruct JoyStatus[4];
BaseController* clist[NUMCTL];
char msg[MSG_LEN];
uint8_t pins_used = 0;
uint8_t num_joys;

char binstr[BITS+1];

void printMsg(const char* format, ...) {
    va_list args;
    va_start(args, format);
    if(true) {
        vsnprintf(msg, MSG_LEN, format, args);
        Serial.flush();
        Serial.println(msg);
    }
    va_end(args);
}

void printBin(char* dest, char input) {
    unsigned char mask = 0x80;

    if(input > 255) { 
        strncpy(dest, "ERR:>255", BITS+1); 
        return; 
    }
    for(int i=0; i < BITS; i++) {
        dest[i] = (input & mask) ? '1' : '0';
        mask >>= 1;
    }
    //Terminate the string
    dest[BITS] = 0;
}

void setup() {
    Serial.begin(9600);

    printMsg("Initiating controllers");

    MultiJoystick.setJoyNum(0);
    MultiJoystick.useManualSend(true); 
    num_joys = MultiJoystick.num_joys();

    //Do some port setup
    //Set up SNES DIR
    //Initialize to low (NES)
    //SNES_PORT &= ~(IO_MASK << SNES_SHIFT);
    SNES_PORT |= IO_MASK << SNES_SHIFT;
    SNES_DIR |= IO_MASK << SNES_SHIFT;
    //Set up clock/latch
    CLOCK_DIR |= CLOCK_MASK;
    LATCH_DIR |= LATCH_MASK;

    //Set up 5V out port to be input, pull-up (for detection)
    DATA5_PORT |= IO_MASK << DATA5_SHIFT;
    DATA5_DIR &= ~(IO_MASK << DATA5_SHIFT);

    //Set up 3V out port to be input, no pull-up (use the 3.3V pull-up on-board)
    DATA3_PORT &= ~(IO_MASK << DATA3_SHIFT);
    DATA3_DIR &= ~(IO_MASK << DATA3_SHIFT);

    DDRC |= 0xC0;

    //We have to detect SNES before NES 
    //(see NESController::detect_controllers)
    //But read NES before SNES 
    //(otherwise SNES will still be responding to our 
    //NES pulse when we try to query it, because
    //SNES sends 16 bits but NES only sends 8)
    //Hence the weird ordering
    //N64 are separate lines, so it doesn't matter
    clist[0] = new N64Controller(JoyStatus, &pins_used, "N64");
    clist[2] = new SNESController(JoyStatus, &pins_used, "SNES");
    clist[1] = new NESController(JoyStatus, &pins_used, "NES");

    clist[0]->init();
    clist[2]->init();
    clist[1]->init();
}

void loop()
{
    int i;
    uint8_t joynum, joypos;

    printBin(binstr, pins_used);
    printMsg("Pins used: 0x%X (%s)\n", pins_used, binstr);
    printMsg("%lu: Polling Controllers...", millis());
    for(i=0;i<NUMCTL;i++) { 
        clist[i]->read_state();
        printBin(binstr, clist[i]->pinmask);
        printMsg("%s: 0x%X (%s)\n", clist[i]->controller_name, clist[i]->pinmask, binstr);
    }
    for(short int cnum=0; cnum < 4; cnum++) {
      //Set joystick parameters
      if(cnum < num_joys) {
        joypos = 0;
        joynum = cnum;
      } else if(cnum < num_joys*2) {
        joypos = 1;
        joynum = cnum - num_joys;
      } else {
        continue;
      }

      printMsg("Setting joystick to %d pos %d", joynum, joypos);
      printMsg("Joystick button data: %X %X", JoyStatus[cnum].buttonset[0], JoyStatus[cnum].buttonset[1]);

      MultiJoystick.setJoyNum(joynum);
      //Update each button
      uint8_t mask = 0x01;
      for (i=0; i<8; i++) {
          MultiJoystick.button(8-i+joypos*16,JoyStatus[cnum].buttonset[0] & mask ? 1 : 0);
          MultiJoystick.button(16-i+joypos*16,JoyStatus[cnum].buttonset[1] & mask ? 1 : 0);
          mask = mask << 1;
      }
    
      //The array is given as AXIS_MIN to AXIS_MAX
      //Joystick funcitons need 0 to 1023
      unsigned int joyx, joyy;
      joyx = JoyStatus[cnum].axis[0]/JOY_FACT + JOY_OFFSET;
      joyy = JoyStatus[cnum].axis[1]/JOY_FACT + JOY_OFFSET;

      MultiJoystick.axis(joypos*2+1,joyx);
      MultiJoystick.axis(joypos*2+2,joyy);
      MultiJoystick.send_now();
    }

    // DEBUG: print it
    //controllers.print_status(0);
    //controllers.print_status(1);
    delay(25);

    Serial.write(27);       // ESC command
    Serial.print("[2J");    // clear screen command
    Serial.write(27);
    Serial.print("[H");     // cursor to home command
}
