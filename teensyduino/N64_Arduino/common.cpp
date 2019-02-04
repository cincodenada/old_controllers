#include "common.h"

#include "pin_config.h"

char msg[MSG_LEN];
char binstr[NUM_BITS+1];

int max_len = 0;
int messages_enabled = true;

void enableMessages(bool enabled) {
  messages_enabled = enabled;
}

void printMsg(const char* format, ...) {
    va_list args;
    int cur_len, i;
    va_start(args, format);
    if(messages_enabled) {
        cur_len = vsnprintf(msg, MSG_LEN, format, args);
        if(cur_len > max_len) { max_len = cur_len; }
        for(i=cur_len; i < max_len && i < MSG_LEN; i++) {
            msg[i] = ' ';
        }
        msg[i] = '\0';
        Serial.flush();
        Serial.println(msg);
    }
    va_end(args);
}

void cls() {
  if(messages_enabled) {
    //Serial.print("\033[2J");    // clear screen command
    Serial.print("\033[0;0H");     // cursor to home command
    Serial.flush();
  }
}

void printBin(char* dest, char input) {
    unsigned char mask = 0x80;

    if(!messages_enabled) { return; }

    if(input > 255) {
        strncpy(dest, "ERR:>255", NUM_BITS+1); 
        return; 
    }
    for(int i=0; i < NUM_BITS; i++) {
        dest[i] = (input & mask) ? '1' : '0';
        mask >>= 1;
    }
    //Terminate the string
    dest[NUM_BITS] = 0;
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

