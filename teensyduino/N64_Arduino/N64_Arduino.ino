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

#include "TimerOne.h"

#include "pins_arduino.h"

#include "common.h"
#include "JoystickStatus.h"
#include "crc_table.h"
#include "N64Controller.h"
#include "SNESController.h"
#include "NESController.h"

#include "bt_controller.h"

#define NUMCTL 3
#define NUMSLOTS 4

JoystickStatus JoyStatus[NUM_CONTROLLERS];
BaseController* clist[NUMCTL];
char msg[MSG_LEN];
uint8_t pins_used = 0;
uint8_t num_joys;
uint8_t s_nes_pins[NUM_CONTROLLERS] = {S_NES_PINS};

char binstr[NUM_BITS+1];

uint8_t button_map[3][NUM_BUTTONS] = {
    { //NES
        // A B Sel St U D L R
        2,1,7,8,
        AXIS(1,-1),AXIS(1,1),
        AXIS(0,-1),AXIS(0,1),
        3,4,5,6,0,0,0,0
    },{ //SNES
        // B Y Sel St U D L R
        1,3,7,8,
        AXIS(1,-1),AXIS(1,1),
        AXIS(0,-1),AXIS(0,1),
        // A X L R
        2,4,5,6,0,0,0,0
    },{ //N64
        // A B Z St U D L R
        1,2,3,4,
        AXIS(3,-1),AXIS(3,1),
        AXIS(2,-1),AXIS(2,1),
        // X X L R U D L R <-(c)
        0,0,5,6,
        HAT(0,-1),HAT(0,1),HAT(-1,0),HAT(1,0)
    }
};

uint8_t button_map_bt[3][NUM_BUTTONS] = {
    { //NES
        2,1,7,8,
        AXIS(1,-1),AXIS(1,1),
        AXIS(0,-1),AXIS(0,1),
        3,4,5,6,0,0,0,0
    },{ //SNES
        1,3,7,8,
        AXIS(1,-1),AXIS(1,1),
        AXIS(0,-1),AXIS(0,1),
        2,4,5,6,0,0,0,0
    },{ //N64
        1,2,6,12,
        AXIS(3,-1),AXIS(3,1),
        AXIS(2,-1),AXIS(2,1),
        0,0,7,8,
        14,15,9,10,
    }
};


void setup() {
    Serial.begin(9600);

    //init_bt();

    delay(1000);
    Serial.println("Testing");

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    printMsg("Initiating controllers");

    MultiJoystick.setJoyNum(0);
    MultiJoystick.useManualSend(true); 
    num_joys = MultiJoystick.num_joys();

    delay(500);
    digitalWrite(LED_PIN, LOW);

    //Set up clock/latch
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);

    delay(500);
    digitalWrite(LED_PIN, HIGH);

    //We have to detect SNES before NES 
    //(see NESController::detect_controllers)
    clist[N64] = new N64Controller(JoyStatus, &pins_used, "N64");
    clist[SNES] = new SNESController(JoyStatus, &pins_used, "SNES");
    clist[NES] = new NESController(JoyStatus, &pins_used, "NES");

    delay(500);
    digitalWrite(LED_PIN, LOW);

    for(int i=0; i < NUM_CONTROLLERS; i++) {
        pinMode(s_nes_pins[i], OUTPUT);
        pinMode(clist[N64]->slow_pins[i], INPUT_PULLUP);
        pinMode(clist[N64]->fast_pins[i], INPUT_PULLUP);
    }

    delay(500);
    digitalWrite(LED_PIN, HIGH);

    clist[N64]->init();

    delay(500);
    digitalWrite(LED_PIN, HIGH);

    clist[SNES]->init();
    clist[NES]->init();

    delay(500);
    digitalWrite(LED_PIN, LOW);
}

void loop()
{
    int i;
    uint8_t joynum, joypos;
    JoystickStatus curStatus;

    // Determine how many controllers we're using
    printBin(binstr, pins_used);
    printMsg("Pins used: 0x%X (%s)", pins_used, binstr);
    printMsg("%lu: Polling Controllers...", millis());
    for(i=0;i<NUMCTL;i++) { 
        clist[i]->read_state();
        printBin(binstr, clist[i]->pinmask);
        printMsg("%s: 0x%X (%s)", clist[i]->controller_name, clist[i]->pinmask, binstr);
    }
    int cnum = 0;
    for(short int slotnum=0; slotnum < NUMSLOTS; slotnum++) {
      // Skip this slot if it's not used
      if(!(pins_used & (0x01 << slotnum))) { continue; }

      // Set joystick parameters
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

      printMsg("Setting joystick to %u pos %u", joynum, joypos);
      printMsg("Joystick button data: %X %X", curStatus.buttonset[0], curStatus.buttonset[1]);
      printMsg("Axes: %d %d %d", curStatus.axis[0], curStatus.axis[1], curStatus.axis[2]);

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

      joyx = curStatus.axis[2]/JOY_FACT + JOY_OFFSET;
      joyy = curStatus.axis[3]/JOY_FACT + JOY_OFFSET;

      MultiJoystick.axis(joypos*2+3,joyx);
      MultiJoystick.axis(joypos*2+4,joyy);

      MultiJoystick.hat(curStatus.hat);

      MultiJoystick.send_now();

      // Increment the controller number for next time
      // (Separate from slot number)
      cnum++;
    }
    
    //For now, just send controller 0 via BT
    JoyStatus[0].translate_buttons(&curStatus, button_map_bt[JoyStatus[0].controller_type]);
    send_bt(&curStatus);

    // DEBUG: print it
    //controllers.print_status(0);
    //controllers.print_status(1);
    delay(25);

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
