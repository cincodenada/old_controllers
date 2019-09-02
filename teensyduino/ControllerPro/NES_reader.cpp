#include "NES_reader.h"
#include <stdio.h>

void NESReader::init() {
  BaseReader::init();

  this->controller_type = NES;
}

void NESReader::setup_pins() {
  //For our pins, set SNES flag to low (=NES)
  for(int i=0; i<NUMSLOTS; i++) {
    if(pinmask & (0x01 << i)) {
      pinMode(this->slow_pins[i], INPUT_PULLUP);
      pinMode(this->s_nes_pins[i], OUTPUT);
      digitalWrite(this->s_nes_pins[i], LOW);
    }
  }
}

void NESReader::clear_dump() {
  memset(this->raw_dump, 0, 16);
}

void NESReader::detect_controllers(uint8_t pins_avail) {
  // At this point any SNES controllers are out of the running
  // Enable the rest as NES controllers, and see which ones
  // pull the DATA line high (and are thus NES controllers)

  // Limit NES to slots 3/4
  pins_avail &= 0b1100;
  for(int i=0; i<NUMSLOTS; i++) {
    if(pins_avail & (0x01 << i)) {
      printMsg("Detecting NES on pin %d", i);
      pinMode(this->slow_pins[i], INPUT); // Hi-Z so the pulldown works
      digitalWrite(this->slow_pins[i], LOW); // Hi-Z so the pulldown works
      digitalWrite(this->s_nes_pins[i], LOW);
    }
  }

  // We're safe to use LATCH now, since SNES controllers
  // are where they want to be
  pinMode(LATCH_PIN, OUTPUT);
  digitalWrite(LATCH_PIN, HIGH);

  // Anyone that responds is an NES controller
  this->pinmask = this->get_deviants(pins_avail, 0);

  digitalWrite(LATCH_PIN, LOW); // Reset latch
}

void NESReader::read_state() {
  //digitalWrite(PIN_TRIGGER, HIGH);

  // read in data and dump it to raw_dump
  this->get();

  this->fillStatus(this->JoyStatus);
  //digitalWrite(PIN_TRIGGER, LOW);
}

void NESReader::prune() {
  pinMode(LATCH_PIN, OUTPUT);
  digitalWrite(LATCH_PIN, HIGH);
  delay(1);

  int missing = this->get_deviants(pinmask, 1);
  pinmask &= ~missing;
  *globalmask &= ~missing;

  pinMode(LATCH_PIN, INPUT);
}

void NESReader::get() {
  this->reset_isr_data();
  this->isr_data.mode = 1;
  this->isr_data.pins = this->slow_pins;
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
    printMsg(binstr);
    //If the button is pressed, set the bit
    if(raw_dump[i] & datamask) {
      joystick->buttonset[0] |= (0x80 >> i);

    }
  }
}
