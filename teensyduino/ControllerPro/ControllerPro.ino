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
#include "joystick_status.h"
#include "crc_table.h"
#include "N64_reader.h"
#include "SNES_reader.h"
#include "NES_reader.h"

#include "bt_controller.h"

#define NUMCTL 3

JoystickStatus JoyStatus[NUMSLOTS];
BaseReader* clist[NUMCTL];
uint8_t pins_used = 0;
uint8_t num_joys;
int cycle_count=0, check_count=10;
int cycle_delay = 25;

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

void detect_ports(char portmask, BaseReader** clist) {
  for(int slot = 0; slot < NUMSLOTS; slot++) {
    if(portmask & 0x01) {
      pinMode(s_nes_pins[slot], INPUT);
      pinMode(fast_pins[slot], INPUT);
      pinMode(slow_pins[slot], INPUT);
      pinMode(extra_pins[slot], INPUT);

      uint16_t fast, nes;
      fast = nes = 0;
      for(int i=0; i<1; i++) {
        fast = max(fast, touchRead(fast_pins[slot])/1);
        nes = max(nes, touchRead(extra_pins[slot])/1);
      }

      // Put high pin back
      pinMode(extra_pins[slot], OUTPUT);
      digitalWrite(extra_pins[slot], HIGH);

      if(nes > fast*3) {
        clist[NES]->claim_slot(slot);
      } else if(fast > nes*3) {
        clist[N64]->claim_slot(slot);
      } else {
        // If it's empty, this will take itself
        // off the list after the first try
        clist[SNES]->claim_slot(slot);
      }

      printMsg(INFO, "Checked port %d: %d/%d", slot, fast, nes);

      delay(10);
    }
    portmask >>= 1;
  }
}

void safe_detect() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  pinMode(CLOCK_PIN, INPUT);
  pinMode(LATCH_PIN, INPUT);

  detect_ports(~pins_used, clist);

  digitalWrite(LED_PIN, LOW);

  for(int i=0; i<NUMCTL; i++) {
    clist[i]->setup_pins();
  }

  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);

  for(int i=0; i<NUMCTL; i++) {
    printMsg(INFO, "%s pinmask: %02x", clist[i]->controller_name, clist[i]->pinmask);
    clist[i]->prune();
    printMsg(INFO, "%s pinmask: %02x", clist[i]->controller_name, clist[i]->pinmask);
  }
}

void setup() {
  Serial.begin(9600);

  //init_bt();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  delay(2000);

  MultiJoystick.setJoyNum(0);
  MultiJoystick.useManualSend(true); 
  num_joys = MultiJoystick.num_joys();

  printMsg("Initiated joystick lib");
  digitalWrite(LED_PIN, LOW);

  clist[N64] = new N64Reader(JoyStatus, &pins_used, "N64");
  clist[SNES] = new SNESReader(JoyStatus, &pins_used, "SNES");
  clist[NES] = new NESReader(JoyStatus, &pins_used, "NES");

  printMsg("Created controllers");
  digitalWrite(LED_PIN, LOW);

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

  // Detect controllers
  if(cycle_count >= check_count) {
    enableMessages(true);
    cls();
    cycle_count = 0;
    safe_detect();
  } else {
    enableMessages(false);
    delay(cycle_delay);
    cycle_count++;
  }

  // Determine how many controllers we're using
  printBin(binstr, pins_used);
  printMsg("Pins used: 0x%X (%s)", pins_used, binstr);
  printMsg("%lu: Polling Controllers...", millis());

  /************************
   * Read controller state
   ************************/

  // Keep these output as short as possible
  // So we don't break things
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  for(i=0;i<NUMCTL;i++) {
    clist[i]->read_state();
    printBin(binstr, clist[i]->pinmask);
    printMsg("%s mask: 0x%X (%s)", clist[i]->controller_name, clist[i]->pinmask, binstr);
  }

  pinMode(CLOCK_PIN, INPUT);
  pinMode(LATCH_PIN, INPUT);

  /************************
   * Output to joysticks
   ************************/

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

  delay(cycle_delay);
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