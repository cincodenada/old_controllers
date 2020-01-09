#include <string.h>
#include <stdint.h>
#include "serial_console.h"
#include "joystick_status.h"

auto console = SerialConsole::getInstance();

uint8_t ButtonMapping::get_btn(controller_type_t type, uint8_t byte, uint8_t bit) {
  switch(type) {
    case NES: return NES_map[byte*8 + bit];
    case SNES: return SNES_map[byte*8 + bit];
    case N64: return N64_map[byte*8 + bit];
    default: return 0;
  }
};

void JoystickStatus::translate_buttons(
  JoystickStatus *dest,
  ButtonMapping& button_map,
  controller_type_t type
) {
  // Copy our status
  dest->copyFrom(this);

  // Reset the buttons
  // Leave axes in place
  // unless we specifically overwrite them
  dest->buttonset[0] = 0;
  dest->buttonset[1] = 0;
  dest->hat = -1;

  int8_t hat_x = 0;
  int8_t hat_y = 0;

  int num_bytes = NUM_BUTTONS/8;
  for(int byte=0; byte < num_bytes; byte++) {
    for(int bit=0; bit < 8; bit++) {
      int btn_num = button_map.get_btn(type, byte, bit);
      if(this->buttonset[byte] & (0x80 >> bit)) {
        if(btn_num >= HAT_BASE) {
          hat_x += HAT_X(btn_num);
          hat_y += HAT_Y(btn_num);
        } else if(btn_num >= AXIS_BASE) {
          int axis_num = AXIS_NUM(btn_num);
          int axis_dir = AXIS_DIR(btn_num);
          console->out(5,"%d: Setting axis %d to %d", btn_num, axis_num, axis_dir);
          // TODO: Adjusted because Bluetooth seems to 
          // freak out if we're at the limits,
          // do some further testing here
          if(axis_dir < 0) {
            dest->axis[axis_num] = AXIS_MIN+512;
          } else {
            dest->axis[axis_num] = AXIS_MAX-512;
          }
        } else if(btn_num) {
          btn_num -= 1;
          int dest_byte = btn_num/8;
          int dest_bit = btn_num%8;
          console->out(5, "Mapping %d:%d to %d:%d...", byte, bit, dest_byte, dest_bit);
          dest->buttonset[dest_byte] |= (0x01 << dest_bit);
        }
      }
    }
  }

  // Set the combine dhat
  dest->hat = hat_map[hat_y+1][hat_x+1];
  console->out(5,"Setting hat to %d (%d, %d)", dest->hat, hat_x, hat_y);
}

void JoystickStatus::copyFrom(JoystickStatus* source) {
  memcpy(this->axis, source->axis, sizeof(this->axis));
  memcpy(this->buttonset, source->buttonset, sizeof(this->buttonset));
  this->hat = source->hat;
  this->controller_type = source->controller_type;
}

void JoystickStatus::clear() {
  memset(this->axis, 0, sizeof(this->axis));
  memset(this->buttonset, 0, sizeof(this->buttonset));
  this->hat = -1;
}

bool JoystickStatus::button_pressed() {
  return this->buttonset[0] || this->buttonset[1];
}
bool JoystickStatus::axis_pressed() {
  return (this->axis[0] > AXIS_THRESH
    || this->axis[0] < -AXIS_THRESH
    || this->axis[1] > AXIS_THRESH
    || this->axis[1] < -AXIS_THRESH
  );
}
bool JoystickStatus::hat_pressed() {
  return (this->hat >= 0);
}
bool JoystickStatus::input_pressed() {
  return button_pressed() || axis_pressed() || hat_pressed();
}
