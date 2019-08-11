#include "base_reader.h"

interrupt_data_struct BaseController::isr_data;

BaseController::BaseController(JoystickStatus *JoyStatus, uint8_t* global_pins, const char* controller_name) {
  this->JoyStatus = JoyStatus;
  this->globalmask = global_pins;
  strncpy(this->controller_name, controller_name, CNAME_LEN);

  this->isr_data = interrupt_data_struct();
}

void BaseController::init() {
  printMsg("Initiating %s controllers", this->controller_name);

  this->pinmask = 0;
  this->safe_detect_controllers();
  this->setup_pins();

  printMsg("%s Pinmask: %X", this->controller_name, this->pinmask);
}

bool BaseController::is_fast() {
  // If newer than N64, use 3.3V
  return (this->controller_type >= N64);
}

void BaseController::safe_detect_controllers() {
  uint8_t pins_avail = ~(*globalmask);

  printBin(binstr, pins_avail, 4);
  printMsg("Searching for %s on %s...", this->controller_name, binstr);

  this->detect_controllers(pins_avail);
  *globalmask |= this->pinmask;
}

void BaseController::fillStatus(JoystickStatus *joylist) {
  uint8_t pinlist = this->pinmask;
  uint8_t datamask = 0x01;
  uint8_t allpins = *globalmask;
  int cnum = 0;

  while(pinlist) {
    if(pinlist & 0x01) {
      printMsg("%lu: Filling status for %s:", millis(), this->controller_name);
      printMsg("%X %X %d", allpins, datamask, cnum);

      this->fillJoystick(&joylist[cnum], datamask);
      joylist[cnum].controller_type = this->controller_type;
    }
    if(allpins & 0x01) { cnum++; }

    allpins >>= 1;
    pinlist >>= 1;
    datamask <<= 1;
  }
}

uint8_t BaseController::read_pin(uint8_t pin) {
  return digitalReadFast((this->is_fast() ? this->fast_pins[pin] : this->slow_pins[pin]));
}

uint8_t BaseController::get_deviants(uint8_t pins_avail, uint8_t expected) {
  int x, resets = 0;
  uint8_t pinmask = 0;
  for (x=0; x<64; x++) {
    int reset = 0;
    int curval;
    uint8_t inpins = 0;
    uint8_t curmask = 0x01;
    for(int i=0; i < NUM_CONTROLLERS; i++) {
      curval = this->read_pin(i);
      inpins |= (curval << i);
      if((pins_avail & curmask) && curval != expected) {
        pinmask |= curmask;
        reset = 1;
      }
      curmask <<= 1;
    }
    if(reset) {
      x = 0;
      resets++;
      if(resets > 10) { break; }
    }
    printMsg("Inpins %X, pins_avail %X, pinmask %X...", inpins, pins_avail, pinmask);
  }
  return pinmask;
}

void BaseController::reset_isr_data() {
  BaseController::isr_data.cur_stage = 0;
  BaseController::isr_data.mode = 0;
  BaseController::isr_data.cur_bit = 0x80;
  // Reset buffer pointers, fill with zeroes
  BaseController::isr_data.cur_byte = BaseController::isr_data.buf;
  BaseController::isr_data.end_byte = &BaseController::isr_data.buf[TBUFSIZE-1];
  BaseController::isr_data.read_bits = TBUFSIZE;
  memset((void*)BaseController::isr_data.buf, 0, TBUFSIZE);
}
