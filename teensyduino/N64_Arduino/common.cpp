#include "pin_config.h"
#include "common.h"

void printMsg(const char* format, ...) {
    va_list args;
    va_start(args, format);
    if(true) {
        vsnprintf(msg, MSG_LEN, format, args);
        Serial.flush();
        Serial.println(msg);
    }
    va_end(args);
}

void blink_binary(int num, uint8_t bits) {
    int mask = 1 << (bits-1);
    digitalWrite(PIN_TRIGGER, HIGH);
    delay(300);
    while(mask) {
        digitalWrite(PIN_TRIGGER, LOW);
        delay(100);
        digitalWrite(PIN_TRIGGER, HIGH);
        delay(100 + 200 * (num & mask));
        mask >>= 1;
    }
    digitalWrite(PIN_TRIGGER, LOW);
    delay(300);
    digitalWrite(PIN_TRIGGER, HIGH);
}

void translate_buttons(
    struct JoystickStatusStruct *dest,
    struct JoystickStatusStruct *source,
    uint8_t *button_map
) {
    memcpy(dest, source, sizeof(struct JoystickStatusStruct));
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
                if(source->buttonset[byte] && (0x01 << bit)) {
                    btn_num -= 1;
                    int dest_byte = btn_num/8;
                    int dest_bit = btn_num%8;
                    dest->buttonset[dest_byte] |= (0x01 << dest_bit);
                }
            }
        }
    }
}
