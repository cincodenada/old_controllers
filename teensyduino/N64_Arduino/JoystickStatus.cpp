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

    int num_bytes = NUM_BUTTONS/8;
    for(int byte=0; byte < num_bytes; byte++) {
        for(int bit=0; bit < 8; bit++) {
            int btn_num = button_map[bit + byte*8];
            if(btn_num > 128) {
                // Deal with axes etc
            } else if(btn_num) {
                if(this->buttonset[byte] & (0x80 >> bit)) {
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
    this->hat = 0;
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
    return (this->hat > 0);
}
bool JoystickStatus::input_pressed() {
    return button_pressed() || axis_pressed() || hat_pressed();
}
