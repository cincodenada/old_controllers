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

struct JoystickStatusStruct JoyStatus[4];
BaseController* clist[NUMCTL];
char msg[MSG_LEN];
uint8_t pins_used = 0;

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

void setup() {
    Serial.begin(9600);

    printMsg("Initiating controllers");

    MultiJoystick.setJoyNum(0);
    MultiJoystick.useManualSend(true); 

    //Do some port setup
    //Set N64 to output (high)
    //And initialize as NES for safety
    N64_PORT |= IO_MASK << N64_SHIFT;
    N64_DIR |= IO_MASK << N64_SHIFT;
    //Set up SNES DIR
    //Initialize to low (NES)
    SNES_PORT &= ~(IO_MASK << SNES_SHIFT);
    SNES_DIR |= IO_MASK << SNES_SHIFT;
    //Set up clock/latch
    CLOCK_DIR |= CLOCK_MASK;
    LATCH_DIR |= LATCH_MASK;

    //Set up data port to be input, pull-up
    DATA_PORT |= IO_MASK << DATA_SHIFT;
    DATA_DIR &= ~(IO_MASK << DATA_SHIFT);

    DDRC |= 0xC0;

    clist[2] = new NESController(JoyStatus, &pins_used, "NES");
    clist[1] = new SNESController(JoyStatus, &pins_used, "SNES");
    clist[0] = new N64Controller(JoyStatus, &pins_used, "N64");

    for(int i=0;i<NUMCTL;i++) { 
        clist[i]->init();
    }
}

void loop()
{
    int i;
    uint8_t joynum, joypos;

    printMsg("%lu: Polling Controllers...", millis());
    for(i=0;i<NUMCTL;i++) { 
        clist[i]->read_state();
    }
    for(short int cnum=0; cnum < 4; cnum++) {
      //Set joystick parameters
      joynum = cnum % 2;
      joypos = cnum / 2;
      joynum = cnum % 2 + 2;

      printMsg("Setting joystick number to %d", joynum);

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

      MultiJoystick.axis(joypos*2,joyx);
      MultiJoystick.axis(joypos*2+1,joyy);
      MultiJoystick.send_now();
    }

    // DEBUG: print it
    //controllers.print_status(0);
    //controllers.print_status(1);
    delay(25);
}
