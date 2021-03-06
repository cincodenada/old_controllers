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

#include "config_editor.h"
#include "pin_config.h"
#include "serial_console.h"
#include "joystick_status.h"
#include "settings.h"
#include "crc_table.h"
#include "N64_reader.h"
#include "SNES_reader.h"
#include "NES_reader.h"

#include "bt_controller.h"

namespace std {
  void __throw_length_error(char const* msg) {
    console.log("Length error: %s, halting", msg);
    while(true);
  }
}

#define NUMCTL 3

#ifdef USB_MULTIJOY
auto& Controller = MultiJoystick;
#elif USB_SERIAL_MULTIJOY
auto& Controller = MultiJoystick;
#elif USB_GAMEPAD
auto& Controller = Gamepad;
#endif

Settings settings;
JoystickStatus JoyStatus[NUMSLOTS];
BaseReader* clist[NUMCTL];
ConfigEditor config;
uint8_t pins_used = 0;
uint8_t num_joys;
int cycle_count=0, check_count=100;
int cycle_delay = 1;

void detect_ports(char portmask, BaseReader** clist) {
  for(int slot = 0; slot < NUMSLOTS; slot++) {
    console.log("Detecting %d", slot);
    if(portmask & 0x01) {
      pinMode(fast_pins[slot], INPUT);
      pinMode(slow_pins[slot], INPUT);
      pinMode(extra_pins[slot], INPUT);

#ifdef BUFFERED_MODE
      pinMode(s_nes_pins[slot], OUTPUT);
      digitalWrite(s_nes_pins[slot], MODE_SNES);
#else
      pinMode(s_nes_pins[slot], INPUT);
#endif

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

      console.log(INFO, "Checked port %d: %d/%d", slot, fast, nes);

      delay(1);
    }
    portmask >>= 1;
  }
}

void safe_detect() {
  pinMode(TRIGGER_PIN, OUTPUT);
  //digitalWrite(TRIGGER_PIN, HIGH);
  pinMode(CLOCK_PIN, INPUT);
  pinMode(LATCH_PIN, INPUT);

  uint8_t SNES_before = clist[SNES]->pinmask;
  detect_ports(~pins_used, clist);
  uint8_t SNES_new = clist[SNES]->pinmask & ~SNES_before;

  //digitalWrite(TRIGGER_PIN, LOW);

  for(int i=0; i<NUMCTL; i++) {
    clist[i]->setup_pins();
  }

  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);

  for(int i=0; i<NUMCTL; i++) {
    console.log(INFO, "%s pinmask: %02x", clist[i]->controller_name, clist[i]->pinmask);
    // For now, only SNES needs pruning
    if(i == SNES) {
      clist[i]->prune(SNES_new);
      console.log(INFO, "%s pinmask: %02x", clist[i]->controller_name, clist[i]->pinmask);
    }
  }
}

void setup() {
  //init_bt();
  console.set_level(TRACE);
  console.log("Loading settings...");

  settings.init();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  Controller.setJoyNum(0);
  Controller.useManualSend(true);
  num_joys = Controller.num_joys();

  console.log("Initiated joystick lib");
  digitalWrite(LED_PIN, LOW);

  clist[N64] = new N64Reader(JoyStatus, &pins_used, "N64");
  clist[SNES] = new SNESReader(JoyStatus, &pins_used, "SNES");
  clist[NES] = new NESReader(JoyStatus, &pins_used, "NES");

  console.log("Created controllers");
  digitalWrite(LED_PIN, LOW);

  for(int i=0; i<10; i++) {
    console.log("");
  }

  console.enable(false);
}

void loop()
{
  int i;
  uint8_t joynum, joypos;

  // Detect controllers
  if(cycle_count >= check_count) {
    console.enable(true);
    console.cls();
    cycle_count = 0;
    safe_detect();
  } else {
    console.enable(false);
    delay(cycle_delay);
    cycle_count++;
  }

  // Determine how many controllers we're using
  printBin(binstr, pins_used);
  console.log("Pins used: 0x%X (%s)", pins_used, binstr);
  console.log("%lu: Polling Controllers...", millis());

  uint8_t data[33] = {0};
  for(int i=0; i < 4; i++) {
    if(clist[N64]->pinmask & (1 << i)) {
      size_t len = ((N64Reader*)clist[N64])->read_mem(i, 32, data);

      uint16_t* curword = (uint16_t*)data;
      // Print 4 words at a time
      for(int i=0; i<4; i++) {
        console.log("%04x %04x %04x %04x",
          *curword,
          *(curword+1),
          *(curword+2),
          *(curword+3));
        curword += 4;
      }
    }
  }
  return;

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
    console.log("%s mask: 0x%X (%s)", clist[i]->controller_name, clist[i]->pinmask, binstr);
  }

  pinMode(CLOCK_PIN, INPUT);
  pinMode(LATCH_PIN, INPUT);

  /************************
   * Output to joysticks
   ************************/

  int controllers_sent = 0;
  if(config.state == ConfigState::DISABLED || config.state == ConfigState::ENTERING) {
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

      config.update(JoyStatus[cnum]);
      auto curStatus = settings.get_map(0).remap(JoyStatus[cnum]);

      console.log(5, "Setting joystick to %u pos %u", joynum, joypos);
      console.log(5, "Joystick button data: %X %X", curStatus.buttonset[0], curStatus.buttonset[1]);
      console.log(5, "Axes: %d %d %d", curStatus.axis[0], curStatus.axis[1], curStatus.axis[2]);

      Controller.setJoyNum(joynum);
      //Update each button
      uint8_t mask = 0x01;
      for (i=1; i<=8; i++) {
        Controller.button(i+joypos*16,curStatus.buttonset[0] & mask ? 1 : 0);
        Controller.button((i+8)+joypos*16,curStatus.buttonset[1] & mask ? 1 : 0);
        mask = mask << 1;
      }

      for(int i=0; i<NUM_AXES; i++) {
        //The array is given as AXIS_MIN to AXIS_MAX
        //Joystick funcitons need 0 to 1023
        unsigned int val = (i < 4)
          ? curStatus.axis[i]/JOY_FACT + JOY_OFFSET
          : JOY_OFFSET;

        Controller.axis(joypos*2+i+1, val);
      }

      Controller.hat(curStatus.hat);

      Controller.send_now();

      // Increment the controller number for next time
      // (Separate from slot number, cause we skip empty slots)
      cnum++;
    }

    controllers_sent = cnum;

    //For now, just send controller 0 via BT
    /*
    JoyStatus[0].translate_buttons(&curStatus, button_map_bt[JoyStatus[0].controller_type]);
    send_bt(&curStatus);
    */
  } else {
    // Config logic
    int cnum = 0;
    for(short int slotnum=0; slotnum < NUMSLOTS; slotnum++) {
      // Skip this slot if it's not used
      if(!(pins_used & (0x01 << slotnum))) { continue; }
      config.update(JoyStatus[cnum]);
      cnum++;
    }
  }

  // If we have empty joysticks, set axes to centered
  // Otherwise we might get axes all off to one corner which is bad
  while(controllers_sent < num_joys) {
    Controller.setJoyNum(controllers_sent);
    for (i=1; i<=16; i++) {
      Controller.button(i,0);
    }
    for(int axis=1; axis <= 5; axis++) {
      Controller.axis(axis, JOY_OFFSET);
    }
    Controller.hat(-1);
    Controller.send_now();
    controllers_sent++;
  }

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
