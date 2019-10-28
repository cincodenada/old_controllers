#include "N64_reader.h"
#include <stdio.h>

void N64Reader::init() {
  BaseReader::init();

  // Query for the gamecube controller's status. We do this
  // to get the 0 point for the control stick.
  // TODO: Does this actually...do anything?
  this->read_state();
}
void N64Reader::setup_pins() {
  //Don't need to do anything, we don't care about S/NES
}

void N64Reader::clear_dump() {
  for(int i=0;i<33;i++) {
    this->raw_dump[i] = 0;
  }
}

void N64Reader::read_state() {
  // Clear raw_dump so we can set individual bits to it
  memset(raw_dump, 0, TBUFSIZE);
  // Run through our controllers one at a time
  for(int i=0; i<NUMSLOTS; i++) {
    if(!(this->pinmask & (0x01 << i))) { continue; }

    uint8_t bits = 0;
    uint8_t max_loops = 200;
    int loops = 0;
    uint8_t high_len = 0;
    uint8_t val = HIGH;

    this->reset_isr_data();
    this->isr_data.cur_pin = fast_pins[i];
    this->isr_data.buf[0] = 0x01;
    this->isr_data.end_byte = this->isr_data.buf;
    this->isr_data.read_bits = 32;
    pinMode(this->isr_data.cur_pin, OUTPUT);
    Timer1.initialize();
    //digitalWriteFast(TRIGGER_PIN, HIGH);
    Timer1.attachInterrupt(&this->isr_write, 1);
    // Spin our wheels
    uint16_t send_cycles=0;
    while(this->isr_data.mode == 0) { send_cycles++; }
    //printMsg("Blooped for %d loops", send_cycles);

    noInterrupts();
    // Wait for initial low...
    loops = 0;
    while(val == HIGH && loops < max_loops) {
      val = digitalReadFast(BaseReader::isr_data.cur_pin);
      loops++;
    }

    while(bits < this->isr_data.read_bits && loops < max_loops) {
      loops = high_len = 0;
      //digitalWriteFast(TRIGGER_PIN, HIGH);
      while(val == LOW && loops < max_loops) {
        val = digitalReadFast(BaseReader::isr_data.cur_pin);
        loops++;
      }
      while(val == HIGH && loops < max_loops) {
        val = digitalReadFast(BaseReader::isr_data.cur_pin);
        loops++;
        high_len++;
      }
      *BaseReader::isr_data.cur_byte = high_len;
      BaseReader::isr_data.cur_byte++;
      bits++;
    }
    interrupts();

    bool hung = false;
    if(loops == max_loops) {
      printMsg("Read %02d bits /!\\", bits);
      // Rewind and erase the last value, in case it was a long high
      BaseReader::isr_data.cur_byte--;
      BaseReader::isr_data.cur_byte = 0;
      hung = true;
    } else {
      printMsg("Read %02d bits   ", bits);
    }

    // Find min/max
    uint8_t curval, min=255, max=0;
    uint8_t k;
    for(k=0; k < this->isr_data.read_bits; k++) {
      curval = this->isr_data.buf[k];
      if(curval > 0) {
        if(curval > max) { max = curval; }
        if(curval < min) { min = curval; }
      }
    }
    // Account for all-zero instances
    if(max < min*2) {
      max = min*2;
    }
    // Remap to 0/1 based on average
    uint8_t mid = (min+max)/2;

    printMsg("First/min/mid/max: %d/%d/%d/%d",
        this->isr_data.buf[0],
        min, mid, max
    );
    for(k=0; k < this->isr_data.read_bits; k++) {
      if(this->isr_data.buf[k]) {
        this->isr_data.buf[k] = (this->isr_data.buf[k] > mid);
      }
    }
    if(this->isr_data.buf[0] == 1) {
      digitalWriteFast(TRIGGER_PIN, HIGH);
    }

    // Don't fill status if we got a bad read
    if(!hung) {
      for(int b=0; b<TBUFSIZE; b++) {
        raw_dump[b] |= this->isr_data.buf[b] << i;
      }
      //memcpy(raw_dump, (void*)this->isr_data.buf, TBUFSIZE);
    }
    delay(1);
    digitalWrite(TRIGGER_PIN, LOW);
  }
  this->fillStatus(this->JoyStatus);
}

void N64Reader::prune(uint8_t candidates) {}

void N64Reader::isr_write() {
  switch(BaseReader::isr_data.cur_stage) {
  case 0:
    //digitalWriteFast(TRIGGER_PIN, HIGH);
    digitalWriteFast(BaseReader::isr_data.cur_pin, LOW);
    // Reset bit mask if we finished the previous byte
    BaseReader::isr_data.cur_stage++;
    break;
  case 1:
    if(BaseReader::isr_data.cur_byte > BaseReader::isr_data.end_byte) {
      // Send stop bit
      digitalWriteFast(BaseReader::isr_data.cur_pin, HIGH);
      // Finish up
      Timer1.detachInterrupt();
      BaseReader::isr_data.mode = 1;
      pinMode(BaseReader::isr_data.cur_pin, INPUT_PULLUP);
      // Reset buf pointer for reading
      BaseReader::isr_data.cur_byte = BaseReader::isr_data.buf;
      break;
    }
    digitalWriteFast(BaseReader::isr_data.cur_pin, BaseReader::isr_data.cur_val);
    BaseReader::isr_data.cur_stage++;
    break;
  case 2:
    BaseReader::isr_data.cur_bit >>= 1;
    if(BaseReader::isr_data.cur_bit == 0) {
        BaseReader::isr_data.cur_bit = 0x80;
        BaseReader::isr_data.cur_byte++;
    }
    BaseReader::isr_data.cur_val = 1;

    BaseReader::isr_data.cur_val = *BaseReader::isr_data.cur_byte & BaseReader::isr_data.cur_bit;
    BaseReader::isr_data.cur_stage++;
    break;
  case 3:
    digitalWriteFast(BaseReader::isr_data.cur_pin, HIGH);
    BaseReader::isr_data.cur_stage = 0;
    break;
  }
}

/**
 * This sends the given byte sequence to the controller
 * length must be at least 1
 * Oh, it destroys the buffer passed in as it writes it
 */
void N64Reader::send(uint8_t pin, uint8_t *buffer, uint8_t length) {
  this->reset_isr_data();
  this->isr_data.cur_pin = fast_pins[pin];
  memcpy((void*)this->isr_data.buf, buffer, length);
  this->isr_data.end_byte = &this->isr_data.buf[length-1];
  pinMode(this->isr_data.cur_pin, OUTPUT);
  Timer1.initialize();
  Timer1.attachInterrupt(&this->isr_write, 1);
  // Spin our wheels
  uint16_t j=0;
  while(this->isr_data.mode == 0) { j++; }
}

void N64Reader::fillJoystick(JoystickStatus *joystick, uint8_t datamask) {
  int i, setnum;
  int8_t xaxis = 0;
  int8_t yaxis = 0;
  joystick->clear();

  // line 1
  // bits: A, B, Z, Start, Dup, Ddown, Dleft, Dright
  for (i=0; i<8; i++) {
    printMsg("%.2X%.2X%.2X%.2X",
      this->raw_dump[i],
      this->raw_dump[i+8],
      this->raw_dump[i+16],
      this->raw_dump[i+24]
    );
    // Fill the buttonsets with the first two bites
    for (setnum=0; setnum<2; setnum++) {
      //If the button is pressed, set the bit
      //N64s happen one at a time, so no need to check a specific bit
      if(raw_dump[i + setnum*8]) {
        joystick->buttonset[setnum] |= (0x80 >> i);
      }
    }
    // Fill the axes with the next two
    xaxis |= (this->raw_dump[16+i]) ? (0x80 >> i) : 0;
    yaxis |= (this->raw_dump[24+i]) ? (0x80 >> i) : 0;
  }

  // Safely translate the axis values from [-N64_AXIS_MAX, N64_AXIS_MAX] to [AXIS_MIN, AXIS_MAX]
  joystick->axis[0] = this->safe_axis(xaxis);
  joystick->axis[1] = -this->safe_axis(yaxis);
}

signed short int N64Reader::safe_axis(int8_t rawval) {
  return max(min(
    max(min(rawval, N64_AXIS_MAX), -N64_AXIS_MAX) * (AXIS_MAX/N64_AXIS_MAX), 
    AXIS_MAX), AXIS_MIN);
}
