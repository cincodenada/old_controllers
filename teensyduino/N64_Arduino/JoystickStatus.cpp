#include <string.h>
#include <stdint.h>
#include "common.h"
#include "JoystickStatus.h"

void JoystickStatus::translate_buttons(
    JoystickStatus *dest,
    uint8_t *button_map
) {
    // Copy our status
    dest->copyFrom(this);

    // Reset the buttons
    // Leave axes in place
    // unless we specifically overwrite them
    dest->buttonset[0] = 0;
    dest->buttonset[1] = 0;
    dest->hat = -1;

    int num_bytes = NUM_BUTTONS/8;
    for(int byte=0; byte < num_bytes; byte++) {
        for(int bit=0; bit < 8; bit++) {
            int btn_num = button_map[bit + byte*8];
            if(this->buttonset[byte] & (0x80 >> bit)) {
                if(btn_num >= HAT_BASE) {
                    dest->hat = hat_map[HAT_Y(btn_num)+1][HAT_X(btn_num)+1];
                    printMsg("%d: Setting hat to %d (%d, %d)", btn_num, dest->hat, HAT_X(btn_num), HAT_Y(btn_num));
                } else if(btn_num >= AXIS_BASE) {
                    int axis_num = AXIS_NUM(btn_num);
                    int axis_dir = AXIS_DIR(btn_num);
                    printMsg("%d: Setting axis %d to %d", btn_num, axis_num, axis_dir);
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
                    printMsg("Mapping %d:%d to %d:%d...", byte, bit, dest_byte, dest_bit);
                    dest->buttonset[dest_byte] |= (0x01 << dest_bit);
                }
            }
        }
    }
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
