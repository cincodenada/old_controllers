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

bool N64Reader::recv(size_t read_bits) {
  noInterrupts();
  // Wait for initial low...
  uint8_t val = HIGH;
  uint8_t loops = 0;
  uint8_t max_loops = 200;

  while(val == HIGH && loops < max_loops) {
    val = digitalReadFast(BaseReader::isr_data.cur_pin);
    loops++;
  }

  uint8_t low_len = 0;
  uint8_t high_len = 0;
  bool hung = false;
  uint8_t* end = this->isr_data.buf + read_bits;
  while(this->isr_data.cur_byte < end && !hung) {
    low_len = high_len = 0;
    //digitalWriteFast(TRIGGER_PIN, HIGH);
    while(val == LOW) {
      val = digitalReadFast(BaseReader::isr_data.cur_pin);
      low_len++;
      if(low_len > max_loops) {
        hung = true;
        goto hung;
      }
    }
    while(val == HIGH) {
      val = digitalReadFast(BaseReader::isr_data.cur_pin);
      high_len++;
      if(high_len > max_loops) {
        hung = true;
        goto hung;
      }
    }
    *BaseReader::isr_data.cur_byte = (high_len > low_len);
    BaseReader::isr_data.cur_byte++;
  }

hung:
  interrupts();

  return hung;
}

void N64Reader::read_state() {
  // Clear raw_dump so we can set individual bits to it
  memset(raw_dump, 0, TBUFSIZE);
  // Run through our controllers one at a time
  for(int i=0; i<NUMSLOTS; i++) {
    if(!(this->pinmask & (0x01 << i))) { continue; }

    uint8_t cmd = 1;
    send(i, &cmd, 1);
    bool hung = recv(32);
    size_t bits = this->isr_data.cur_byte - this->isr_data.buf;

    if(hung) {
      console.log("Read %02d bits /!\\", bits);
    } else {
      console.log("Read %02d bits   ", bits);
    }

    if(this->isr_data.buf[0] == 1 && !hung) {
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

size_t N64Reader::read_mem(uint8_t slot, uint16_t address, uint8_t* buf) {
  address &= ~0x1F; // These should be blank anyway, but make sure

  uint16_t crc = address;
  constexpr uint8_t addr_bits = 16-5;
  uint16_t poly = (0x15 + 0x20) << addr_bits;  // 5-bit CRC; 0x20 = 100000 
  uint16_t mask = 0x8000;
  for(int i=0; i<addr_bits; i++) {
    if(crc & mask) {
      crc ^= poly;
    }
    poly >>= 1;
    mask >>= 1;
  }

  address |= crc;

  console.log("Reading address %04x", address);
  uint8_t command[3];
  command[0] = 2;
  command[1] = (address & 0xFF00) >> 8;
  command[2] = address & 0x00FF;
  send(slot, command, 3);
  bool hung = recv(33*8); // 32 bytes + CRC
  size_t bits = isr_data.cur_byte - isr_data.buf;

  if(hung) {
    console.log("Read %02d bits /!\\", bits);
  } else {
    console.log("Read %02d bits   ", bits);
  }

  if(hung) {
    return 0;
  } else {
    for(int byte=0; byte<33; byte++) {
      buf[byte] = 0;
      for(int bit=0; bit < 8; bit++) {
        buf[byte] |= buf[byte*8+bit] << bit;
      }
    }

    return 33;
  }
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
    console.log("%.2X%.2X%.2X%.2X",
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
