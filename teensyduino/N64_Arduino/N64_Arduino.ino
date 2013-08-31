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

#include "pin_config.h"
#include "crc_table.h"
#include "N64Controller.h"
#include "NESController.h"
#include <stdio.h>

NESController controllers;
struct JoystickStatusStruct JoyStatus[4];

void setup()
{
  Serial.begin(9600);
  
  Serial.println("Initiating controllers");
  
  DualJoystick.setJoyNum(1);

  DualJoystick.useManualSend(true); 

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

  DDRC |= 0x0C;

  controllers = NESController(JoyStatus);

  controllers.init();
}

void loop()
{
    int i;
    unsigned char joynum, joypos;

    Serial.println("Polling Controllers...");
    controllers.read_state(); 
    for(short int cnum=0; cnum < 2; cnum++) {
      //Set joystick parameters
      joynum = cnum % 2;
      joypos = cnum / 2;
      
      DualJoystick.setJoyNum(joynum);
      //Update each button
      char mask = 0x01;
      for (i=0; i<8; i++) {
          DualJoystick.button(8-i+joypos*16,JoyStatus[cnum].buttonset[0] & mask ? 1 : 0);
          DualJoystick.button(16-i+joypos*16,JoyStatus[cnum].buttonset[1] & mask ? 1 : 0);
          mask = mask << 1;
      }
    
      //The array is given as AXIS_MIN to AXIS_MAX
      //Joystick funcitons need 0 to 1023
      unsigned int joyx, joyy;
      joyx = JoyStatus[cnum].axis[0]/JOY_FACT + JOY_OFFSET;
      joyy = JoyStatus[cnum].axis[1]/JOY_FACT + JOY_OFFSET;
       
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

    // DEBUG: print it
    //controllers.print_status(0);
    //controllers.print_status(1);
    delay(25);
}
