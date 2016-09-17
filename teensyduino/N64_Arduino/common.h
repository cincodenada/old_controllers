#ifndef COMMON_H
#define COMMON_H

#define MSG_LEN 100
extern char msg[MSG_LEN];

#define BIN_BITS 8

#include "Arduino.h"

void printMsg(const char* format, ...);
void printBin(char* dest, char input);
void blink_binary(int num, uint8_t bits);

#endif /* COMMON_H */
