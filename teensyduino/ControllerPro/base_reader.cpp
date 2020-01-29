#include "base_reader.h"

interrupt_data_struct BaseReader::isr_data;

BaseReader::BaseReader(JoystickStatus *JoyStatus, uint8_t* global_pins, const char* controller_name) {
  this->JoyStatus = JoyStatus;
  this->globalmask = global_pins;
  strncpy(this->controller_name, controller_name, CNAME_LEN);

  this->isr_data = interrupt_data_struct();
}

void BaseReader::init() {
  console.log("Initiating %s controllers", this->controller_name);

  this->pinmask = 0;
  this->setup_pins();

  console.log("%s Pinmask: %X", this->controller_name, this->pinmask);
}

void BaseReader::claim_slot(int num) {
  uint8_t mask = 0x01 << num;
  this->pinmask |= mask;
  *(this->globalmask) |= mask;
}

bool BaseReader::is_fast() {
  // If newer than N64, use 3.3V
  return (this->controller_type >= N64);
}

void BaseReader::fillStatus(JoystickStatus *joylist) {
  uint8_t pinlist = this->pinmask;
  uint8_t datamask = 0x01;
  uint8_t allpins = *globalmask;
  int cnum = 0;

  while(pinlist) {
    if(pinlist & 0x01) {
      console.log("%lu: Filling status for %s:", millis(), this->controller_name);
      console.log("%X %X %d", allpins, datamask, cnum);

      this->fillJoystick(&joylist[cnum], datamask);
      joylist[cnum].controller_type = this->controller_type;
    }
    if(allpins & 0x01) { cnum++; }

    allpins >>= 1;
    pinlist >>= 1;
    datamask <<= 1;
  }
}

uint8_t BaseReader::read_pin(uint8_t pin) {
  return digitalReadFast((this->is_fast() ? fast_pins[pin] : slow_pins[pin]));
}

uint8_t BaseReader::get_deviants(uint8_t pins, uint8_t expected) {
  int x, resets = 0;
  uint8_t pinmask = 0;
  for (x=0; x<64; x++) {
    int reset = 0;
    int curval;
    uint8_t curmask = 0x01;
    for(int i=0; i < NUMSLOTS; i++) {
      if(pins & curmask) {
        curval = this->read_pin(i);
        if(curval != expected) {
          pinmask |= curmask;
          reset = 1;
        }
      }
      curmask <<= 1;
    }
    if(reset) {
      x = 0;
      resets++;
      if(resets > 10) { break; }
    }
  }
  return pinmask;
}

void BaseReader::reset_isr_data() {
  BaseReader::isr_data.cur_stage = 0;
  BaseReader::isr_data.mode = 0;
  BaseReader::isr_data.cur_bit = 0x80;
  // Reset buffer pointers, fill with zeroes
  BaseReader::isr_data.cur_byte = BaseReader::isr_data.buf;
  BaseReader::isr_data.end_byte = &BaseReader::isr_data.buf[TBUFSIZE-1];
  memset((void*)BaseReader::isr_data.buf, 0, TBUFSIZE);
}
