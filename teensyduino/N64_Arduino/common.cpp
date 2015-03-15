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

