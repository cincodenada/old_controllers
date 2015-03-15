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

#include "common.h"
#include "JoystickStatus.h"
#include "crc_table.h"
#include "N64Controller.h"
#include "SNESController.h"
#include "NESController.h"

#include "bt_controller.h"

#define NUMCTL 3
#define BITS 8

JoystickStatus JoyStatus[4];
BaseController* clist[NUMCTL];
char msg[MSG_LEN];
uint8_t pins_used = 0;
uint8_t num_joys;

char binstr[BITS+1];

uint8_t button_map[3][NUM_BUTTONS] = {
    { //NES
        2,1,7,8,
        AXIS(1,1),AXIS(1,-1),
        AXIS(0,-1),AXIS(0,1),
        3,4,5,6,0,0,0,0
    },{ //SNES
        1,3,7,8,129,130,131,132,
        2,4,5,6,0,0,0,0
    },{ //N64
        1,2,3,4,11,12,13,14,
        0,0,5,6,7,8,9,10
    }
};

uint8_t button_map_bt[3][NUM_BUTTONS] = {
    { //NES
        2,1,7,8,
        AXIS(1,1),AXIS(1,-1),
        AXIS(0,-1),AXIS(0,1),
        3,4,5,6,0,0,0,0
    },{ //SNES
        1,3,7,8,129,130,131,132,
        2,4,5,6,0,0,0,0
    },{ //N64
        1,2,4,11,0,0,0,0,
        0,0,7,8,0,0,0,0
    }
};


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
    init_bt();

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
    clist[N64] = new N64Controller(JoyStatus, &pins_used, "N64");
    clist[SNES] = new SNESController(JoyStatus, &pins_used, "SNES");
    clist[NES] = new NESController(JoyStatus, &pins_used, "NES");

    clist[N64]->init();
    clist[SNES]->init();
    clist[NES]->init();
}

void loop()
{
    int i;
    uint8_t joynum, joypos;
    JoystickStatus curStatus;

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

      JoyStatus[cnum].translate_buttons(&curStatus, button_map[JoyStatus[cnum].controller_type]);

      printMsg("Setting joystick to %d pos %d", joynum, joypos);
      printMsg("Joystick button data: %X %X", curStatus.buttonset[0], curStatus.buttonset[1]);

      MultiJoystick.setJoyNum(joynum);
      //Update each button
      uint8_t mask = 0x01;
      for (i=1; i<=8; i++) {
          MultiJoystick.button(i+joypos*16,curStatus.buttonset[0] & mask ? 1 : 0);
          MultiJoystick.button((i+8)+joypos*16,curStatus.buttonset[1] & mask ? 1 : 0);
          mask = mask << 1;
      }
    
      //The array is given as AXIS_MIN to AXIS_MAX
      //Joystick funcitons need 0 to 1023
      unsigned int joyx, joyy;
      joyx = curStatus.axis[0]/JOY_FACT + JOY_OFFSET;
      joyy = curStatus.axis[1]/JOY_FACT + JOY_OFFSET;

      MultiJoystick.axis(joypos*2+1,joyx);
      MultiJoystick.axis(joypos*2+2,joyy);
      MultiJoystick.send_now();
    }
    
    //For now, just send controller 0 via BT
    JoyStatus[0].translate_buttons(&curStatus, button_map_bt[JoyStatus[0].controller_type]);
    send_bt(&curStatus);

    // DEBUG: print it
    //controllers.print_status(0);
    //controllers.print_status(1);
    delay(25);

    Serial.write(27);       // ESC command
    Serial.print("[2J");    // clear screen command
    Serial.write(27);
    Serial.print("[H");     // cursor to home command
}

void remap_buttons(uint8_t cnum) {
    int curbtn = 0;
    uint8_t new_map[NUM_BUTTONS];
    controller_type_t ctype;
    while(curbtn < NUM_BUTTONS) {
        ctype=JoyStatus[cnum].controller_type;
        //Wait for a button
        while(true) {
            clist[ctype]->read_state();
            if(JoyStatus[cnum].button_pressed()) {
                int mask = 1;
                for(int bit=1; bit <= 8; bit++) {
                    if(JoyStatus[cnum].buttonset[0] & mask) {
                        new_map[curbtn] = bit;
                        break;
                    } else if(JoyStatus[cnum].buttonset[1] & mask) {
                        new_map[curbtn] = bit + 8;
                        break;
                    }
                    mask <<= 1;
                }
                break;
            } else if(JoyStatus[cnum].axis_pressed()) {
                break;
            } else if(JoyStatus[cnum].hat_pressed()) {
                break;
            }
        }
        while(JoyStatus[cnum].input_pressed()) {}

        curbtn++;
    }
}
