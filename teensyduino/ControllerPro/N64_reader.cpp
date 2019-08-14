#include "N64_reader.h"
#include <stdio.h>

void N64Reader::init() {
  BaseReader::init();

  this->controller_type = N64;

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

void N64Reader::detect_controllers(uint8_t pins_avail) {
  //NES and SNES pull low on idle, so check for that
  //(N64 maintains high, and we use pull-up)
  
  //SNES/NES port doesn't matter
  
  //Just send the ID command and see who answers
  //This also initializes some controllers (Wavebird, I guess?)
  uint8_t command;
  command = 0x01;
  int num_tries = 5;
  pinMode(PIN_TRIGGER, OUTPUT);

  for(int i=0; i < NUMSLOTS; i++) {
    if(!(pins_avail & (0x01 << i))) { continue; }

    bool responded = false;
    for(int t=0; t < num_tries; t++) {
      printMsg("Sending command for controller %d...", i+1);
      this->send(i, &command, 1);

      uint8_t cur_pin = this->fast_pins[i];
      pinMode(cur_pin, INPUT_PULLUP);
      printMsg("Waiting for response...");
      for (int x=0; x<200; x++) {
        if(!digitalReadFast(cur_pin)) {
          responded = true;
          // Reset so we keep going until we've
          // swallowed the whole response
          x = 0;
        }
      }
      if(responded) { break; }
    }
    if(responded) {
      printMsg("Reader found in port %d", i+1);
      this->pinmask |= (0x01 << i);
    } else {
      printMsg("No controller found in port %d", i+1);
    }
    digitalWriteFast(PIN_TRIGGER, LOW);
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
    uint8_t low_len = 0;
    uint8_t val = HIGH;

    this->reset_isr_data();
    this->isr_data.cur_pin = this->fast_pins[i];
    this->isr_data.buf[0] = 0x01;
    this->isr_data.end_byte = this->isr_data.buf;
    this->isr_data.read_bits = 32;
    pinMode(this->isr_data.cur_pin, OUTPUT);
    Timer1.initialize();
    digitalWriteFast(PIN_TRIGGER, HIGH);
    Timer1.attachInterrupt(&this->isr_write, 1);
    // Spin our wheels
    uint16_t j=0;
    while(this->isr_data.mode == 0) { j++; }
    //printMsg("Blooped for %d loops", j);

    int hung = 0;
    noInterrupts();
    // Wait for initial low...
    while(bits < this->isr_data.read_bits) {
      loops = 0;
      digitalWriteFast(PIN_TRIGGER, HIGH);
      while(val == HIGH && loops < max_loops) {
        val = digitalReadFast(BaseReader::isr_data.cur_pin);
        loops++;
      }
      if(loops == max_loops) {
        digitalWrite(PIN_TRIGGER, LOW);
        hung = 1;
        break;
      }
      low_len = 0;
      while(val == LOW && loops < max_loops) {
        val = digitalReadFast(BaseReader::isr_data.cur_pin);
        low_len++;
      }
      *BaseReader::isr_data.cur_byte = low_len;
      BaseReader::isr_data.cur_byte++;
      bits++;
    }
    interrupts();

    if(hung) {
      printMsg("Read %02d bits /!\\", bits);
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
    if(min > max/2) {
      min = max/2;
    }
    // Remap to 0/1 based on average
    uint8_t mid = (min+max)/2;

    printMsg("First/min/mid/max: %d/%d/%d/%d",
        this->isr_data.buf[0],
        min, mid, max
    );
    for(k=0; k < this->isr_data.read_bits; k++) {
      if(this->isr_data.buf[k]) {
        this->isr_data.buf[k] = (this->isr_data.buf[k] < mid);
      }
    }

    // Don't fill status if we got a bad read
    if(!hung) {
      for(int b=0; b<TBUFSIZE; b++) {
        raw_dump[b] |= this->isr_data.buf[b] << i;
      }
      //memcpy(raw_dump, (void*)this->isr_data.buf, TBUFSIZE);
    }
    delay(1);
    digitalWrite(PIN_TRIGGER, LOW);
  }
  this->fillStatus(this->JoyStatus);
}

void N64Reader::isr_write() {
  switch(BaseReader::isr_data.cur_stage) {
  case 0:
    //digitalWriteFast(PIN_TRIGGER, HIGH);
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
  this->isr_data.cur_pin = this->fast_pins[pin];
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
  // Axis 0/1 is the dpad
  joystick->axis[2] = this->safe_axis(xaxis);
  joystick->axis[3] = -this->safe_axis(yaxis);
}

signed short int N64Reader::safe_axis(int8_t rawval) {
  return max(min(
    max(min(rawval, N64_AXIS_MAX), -N64_AXIS_MAX) * (AXIS_MAX/N64_AXIS_MAX), 
    AXIS_MAX), AXIS_MIN);
}
