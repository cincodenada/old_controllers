#include "binutil.h"

#include "Arduino.h"
#include "pin_config.h"

char binstr[NUM_BITS+1];

void printBin(char* dest, char input, unsigned char num_bits) {
  unsigned char mask = 1 << (num_bits-1);
  if(num_bits > NUM_BITS) {
    strncpy(dest, "ERR:>8BT", NUM_BITS+1);
    return;
  }

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
  digitalWrite(LED_PIN, HIGH);
  delay(300);
  while(mask) {
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(100 + 200 * (num & mask));
    mask >>= 1;
  }
  digitalWrite(LED_PIN, LOW);
  delay(300);
  digitalWrite(LED_PIN, HIGH);
}

