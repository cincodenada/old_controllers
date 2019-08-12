#include "common.h"

#include "pin_config.h"

char msg[MSG_LEN];
char binstr[NUM_BITS+1];

int max_len = 30;
int messages_enabled = true;

void enableMessages(bool enabled) {
  messages_enabled = enabled;
}

void vprintMsg(int level, const char* format, va_list args) {
  if(level > LOG_LEVEL) { return; }
  if(messages_enabled) {
    int cur_len, i;
    cur_len = vsnprintf(msg, MSG_LEN, format, args);
    if(cur_len > max_len) { max_len = cur_len; }
    for(i=cur_len; i < max_len && i < MSG_LEN; i++) {
      msg[i] = ' ';
    }
    msg[i] = '\0';
    Serial.flush();
    Serial.println(msg);
    Serial.flush();
  }
}
void printMsg(int level, const char* format, ...) {
  va_list args;
  va_start(args, format);
  vprintMsg(level, format, args);
  va_end(args);
}
void printMsg(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vprintMsg(3, format, args);
  va_end(args);
}

void cls() {
  if(messages_enabled) {
  //Serial.print("\033[2J");  // clear screen command
  Serial.print("\033[0;0H");   // cursor to home command
  Serial.flush();
  }
}

void printBin(char* dest, char input, unsigned char num_bits) {
  unsigned char mask = 1 << (num_bits-1);
  if(num_bits > NUM_BITS) {
    strncpy(dest, "ERR:>8BT", NUM_BITS+1);
    return;
  }

  if(!messages_enabled) { return; }

  if(input > 255) {
    strncpy(dest, "ERR:>255", NUM_BITS+1);
    return;
  }
  for(int i=0; i < num_bits; i++) {
    dest[i] = (input & mask) ? '1' : '0';
    mask >>= 1;
  }
  //Terminate the string
  dest[num_bits] = 0;
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

