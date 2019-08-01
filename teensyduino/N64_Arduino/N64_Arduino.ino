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

#include "pin_config.h"
#include "common.h"
#include "JoystickStatus.h"
#include "crc_table.h"
#include "N64Controller.h"
#include "SNESController.h"
#include "NESController.h"

#include "bt_controller.h"

// We only use the first numjoys slots of this
JoystickStatus JoyStatus[NUM_SLOTS];
BaseController* clist[NUM_TYPES];
uint8_t pins_used = 0;
uint8_t num_joys;

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
        AXIS(1,-1),AXIS(1,1),
        AXIS(0,-1),AXIS(0,1),
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



void truth_table() {
  pinMode(CLOCK_PIN, INPUT);
  for(int i=0; i < NUM_SLOTS; i++) {
    pinMode(slow_pins[i], INPUT_PULLUP);
    digitalWrite(s_nes_pins[i], LOW);
  }
  while(true) {
    for(int latch=0; latch <=1; latch++) {
      for(int snes=0; snes <=1; snes++) {
        for(int i=0; i<NUM_SLOTS; i++) {
          digitalWrite(latch_pins[i], latch);
          digitalWrite(s_nes_pins[i], snes);
        }
        for(int i=0; i<NUM_SLOTS; i++) {
          printMsg("%d: %d%d%d%d",
            i,
            digitalRead(slow_pins[i]),
            latch,
            digitalRead(CLOCK_PIN),
            snes);
          delay(1);
        }
      }
    }
    delay(500);
    cls();
  }
}

// Results:
//          S/NES | LATCH | DATA
// NES    |   0   |   0   |  1
//        |   1   |   1   |  1
// SNES   |   0   |   0   |  0
//        |   1   |   1   |  1
// EMPTY  |   0   |   0   |  0
//        |   1   |   0   |  0
// sensitive to timing??
//
// w/ pullup on LATCH
//          S/NES | LATCH | DATA
// NES    |   0   |   1   |  1
//        |   1   |   1   |  1
// SNES   |   0   |   0   |  0
//        |   1   |   1   |  1
// EMPTY  |   0   |   0   |  0
//        |   1   |   0   |  0
// 
void debug_detect() {
  for(int i=0; i < NUM_SLOTS; i++) {
      pinMode(s_nes_pins[i], OUTPUT);
      pinMode(latch_pins[i], INPUT);
      pinMode(slow_pins[i], INPUT);
  }

  while(true) {
    for(int i=0; i < NUM_SLOTS; i++) {
      digitalWrite(s_nes_pins[i], LOW);
    }
    delay(1);
    for(int i=0; i < NUM_SLOTS; i++) {
      digitalWrite(s_nes_pins[i], HIGH);
    }
    delay(1);
  }
}
void debug_detect2() {
  for(int i=0; i < NUM_SLOTS; i++) {
      pinMode(s_nes_pins[i], OUTPUT);
  }

  while(true) {
    printMsg("");
    printMsg("");
    printMsg("");
    printMsg("");
    cls();
    pins_used = 0;
    digitalWrite(LED_PIN, LOW);
    clist[N64]->init();
    digitalWrite(LED_PIN, HIGH);
    clist[NES]->init();
    digitalWrite(LED_PIN, LOW);
    clist[SNES]->init();
    digitalWrite(LED_PIN, HIGH);

    for(int i=0; i < NUM_SLOTS; i++) {
      unsigned char mask = 1 << i;
      char controller[] = "None";
      bool matched = false;
      for(int i=0; i < NUM_TYPES; i++) {
        if(clist[i]->pinmask & mask) {
          if(matched) {
            printMsg("Duplicate pinmask! Also on %s", clist[i]->controller_name);
          } else {
            strncpy(controller, clist[i]->controller_name, 5);
          }
        }
      }
      printMsg("Slot %d: %s", i+1, controller);
    }
    delay(20);
  }
}

void setup() {
    Serial.begin(9600);

    //init_bt();

    pinMode(LED_PIN, OUTPUT);

/*
    int pinval = HIGH;
    int waitsecs = 5;
    printMsg("Waiting %d seconds for serial connection");
    for(int i=waitsecs; i>0; --i) {
        Serial.print(i);
        Serial.print("...");
        Serial.flush();
        digitalWrite(LED_PIN, pinval);
        delay(1000);
        pinval = (pinval == HIGH) ? LOW : HIGH;
    }
    Serial.println();
*/

    digitalWrite(LED_PIN, HIGH);

    MultiJoystick.setJoyNum(0);
    MultiJoystick.useManualSend(true); 
    num_joys = MultiJoystick.num_joys();

    printMsg("Initiated joystick lib");
    digitalWrite(LED_PIN, LOW);

    printMsg("Set up pins");
    digitalWrite(LED_PIN, HIGH);

    // We have to detect SNES before NES
    // (see NESController::detect_controllers)
    clist[N64] = new N64Controller(JoyStatus, &pins_used, "N64");
    clist[SNES] = new SNESController(JoyStatus, &pins_used, "SNES");
    clist[NES] = new NESController(JoyStatus, &pins_used, "NES");

    printMsg("Created controllers");
    digitalWrite(LED_PIN, LOW);

    for(int i=0; i < NUM_SLOTS; i++) {
        pinMode(s_nes_pins[i], OUTPUT);
    }

    debug_detect2();

    for(int i=0; i < NUM_SLOTS; i++) {
        pinMode(clist[N64]->slow_pins[i], INPUT_PULLUP);
        pinMode(clist[N64]->fast_pins[i], INPUT_PULLUP);
        pinMode(latch_pins[i], OUTPUT);
    }

    // Now that S/NES mode is set, we can set
    // CLOCK/LATCH to outputs. Doing so earlier
    // would expose the controllers to odd voltages
    pinMode(CLOCK_PIN, OUTPUT);

    printMsg("Initiated controller pins");
    digitalWrite(LED_PIN, HIGH);

    clist[N64]->init();

    printMsg("Initiated N64");
    digitalWrite(LED_PIN, LOW);

    clist[SNES]->init();
    clist[NES]->init();

    printMsg("Initiated NES/SNES");
    digitalWrite(LED_PIN, HIGH);

    for(int i=0; i<10; i++) {
        Serial.println();
    }

    enableMessages(false);
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
    for(i=0;i<NUM_TYPES;i++) {
        clist[i]->read_state();
        printBin(binstr, clist[i]->pinmask);
        printMsg("%s mask: 0x%X (%s)", clist[i]->controller_name, clist[i]->pinmask, binstr);
    }
    int cnum = 0;
    for(short int slotnum=0; slotnum < NUM_SLOTS; slotnum++) {
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

      printMsg(5, "Setting joystick to %u pos %u", joynum, joypos);
      printMsg(5, "Joystick button data: %X %X", curStatus.buttonset[0], curStatus.buttonset[1]);
      printMsg(5, "Axes: %d %d %d", curStatus.axis[0], curStatus.axis[1], curStatus.axis[2]);

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

      // X/Y
      joyx = curStatus.axis[0]/JOY_FACT + JOY_OFFSET;
      joyy = curStatus.axis[1]/JOY_FACT + JOY_OFFSET;

      MultiJoystick.axis(joypos*2+1,joyx);
      MultiJoystick.axis(joypos*2+2,joyy);

      // Throttle/Z (constant center)
      MultiJoystick.axis(joypos*2+3, JOY_OFFSET);

      // X2/Y2
      joyx = curStatus.axis[2]/JOY_FACT + JOY_OFFSET;
      joyy = curStatus.axis[3]/JOY_FACT + JOY_OFFSET;

      MultiJoystick.axis(joypos*2+4,joyx);
      MultiJoystick.axis(joypos*2+5,joyy);

      MultiJoystick.hat(curStatus.hat);

      MultiJoystick.send_now();

      // Increment the controller number for next time
      // (Separate from slot number, cause we skip empty slots)
      cnum++;
    }

    // If we have empty joysticks, set axes to centered
    // Otherwise we might get axes all off to one corner which is bad
    while(cnum < num_joys) {
      MultiJoystick.setJoyNum(cnum);
      for(int axis=1; axis <= 5; axis++) {
        MultiJoystick.axis(axis, JOY_OFFSET);
      }
      MultiJoystick.hat(-1);
      MultiJoystick.send_now();
      cnum++;
    }

    
    //For now, just send controller 0 via BT
    JoyStatus[0].translate_buttons(&curStatus, button_map_bt[JoyStatus[0].controller_type]);
    send_bt(&curStatus);

    // DEBUG: print it
    //controllers.print_status(0);
    //controllers.print_status(1);
    delay(25);
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
