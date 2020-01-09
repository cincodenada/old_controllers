#include "NES_reader.h"
#include <stdio.h>

auto console = SerialConsole::getInstance();

void NESReader::init() {
  BaseReader::init();
}

void NESReader::setup_pins() {
  //For our pins, set SNES flag to low (=NES)
  for(int i=0; i<NUMSLOTS; i++) {
    if(pinmask & (0x01 << i)) {
      pinMode(slow_pins[i], INPUT);
      pinMode(s_nes_pins[i], OUTPUT);
      digitalWriteFast(s_nes_pins[i], MODE_NES);
    }
  }
}

void NESReader::clear_dump() {
  memset(this->raw_dump, 0, 16);
}

void NESReader::read_state() {
  //digitalWrite(PIN_TRIGGER, HIGH);

  // read in data and dump it to raw_dump
  this->get();

  this->fillStatus(this->JoyStatus);
  //digitalWrite(PIN_TRIGGER, LOW);
}

void NESReader::prune(uint8_t candidates) {}

void NESReader::get() {
  this->reset_isr_data();
  this->isr_data.mode = 1;
  this->isr_data.pins = slow_pins;
  this->isr_data.end_byte = &this->isr_data.buf[8];
  Timer1.initialize();
  Timer1.attachInterrupt(&this->isr_read, 6);

  // Wait for it to do its thing
  while(this->isr_data.mode == 1) {}

  // Transfer to raw_dump
  for(int i=0; i < 8; i++) {
    this->raw_dump[i] = ~this->isr_data.buf[i];
  }
}

void NESReader::isr_read() {
  int mask = 0x01;
  switch(BaseReader::isr_data.cur_stage) {
    case 0:
      digitalWriteFast(LATCH_PIN, HIGH);
      digitalWriteFast(CLOCK_PIN, LOW);
      break;
    case 1:
      // First bit is on latch, so read it
      for(int i=0; i < NUMSLOTS; i++) {
        if(digitalReadFast(BaseReader::isr_data.pins[i])) {
          *BaseReader::isr_data.cur_byte |= mask;
        }
        mask <<= 1;
      }
      break;
    case 3:
      digitalWriteFast(LATCH_PIN, LOW);
      break;
    case 4:
      // Do nothing
      break;
    case 5:
      if(BaseReader::isr_data.cur_byte >= BaseReader::isr_data.end_byte) {
        BaseReader::isr_data.mode = 2;
        Timer1.detachInterrupt();
      } else {
        digitalWriteFast(CLOCK_PIN, HIGH);
        BaseReader::isr_data.cur_byte++;
      }
      break;
    case 6:
      for(int i=0; i < NUMSLOTS; i++) {
        if(digitalReadFast(BaseReader::isr_data.pins[i])) {
          *BaseReader::isr_data.cur_byte |= mask;
        }
        mask <<= 1;
      }
      digitalWriteFast(CLOCK_PIN, LOW);
      // Set back to 4 so it gets incremented to 5
      BaseReader::isr_data.cur_stage = 4;
      break;
  }
  BaseReader::isr_data.cur_stage++;
}

void NESReader::fillJoystick(JoystickStatus *joystick, uint8_t datamask) {
  joystick->clear();

  // line 1
  // bits: A, B, Select, Start, Dup, Ddown, Dleft, Dright
  // (reversed)
  for (int i=0; i<8; i++) {
    printBin(binstr, this->raw_dump[i]);
    console->out(binstr);
    //If the button is pressed, set the bit
    if(raw_dump[i] & datamask) {
      joystick->buttonset[0] |= (0x80 >> i);

    }
  }
}
