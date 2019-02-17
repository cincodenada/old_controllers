#ifndef COMMON_H
#define COMMON_H

#define MSG_LEN 70
#define NUM_BITS 8
#define LOG_LEVEL 3

extern char msg[MSG_LEN];
extern char binstr[NUM_BITS+1];

#include "Arduino.h"

void enableMessages(bool enabled);
void printMsg(int level, const char* format, ...);
void printMsg(const char* format, ...);
void cls();
void printBin(char* dest, char input, unsigned char num_bits = NUM_BITS);
void blink_binary(int num, uint8_t bits);

#endif /* COMMON_H */
