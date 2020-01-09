#ifndef CONSOLE_H
#define CONSOLE_H

#define ERROR 1
#define WARN 2
#define INFO 3
#define DEBUG 4
#define TRACE 5

#define MSG_LEN 70
#define NUM_BITS 8
#define LOG_LEVEL TRACE

extern char msg[MSG_LEN];
extern char binstr[NUM_BITS+1];

#include "Arduino.h"

void enableMessages(bool enabled);
void printMsg(int level, const char* format, ...);
void printMsg(const char* format, ...);
void cls();
void printBin(char* dest, char input, unsigned char num_bits = NUM_BITS);
void blink_binary(int num, uint8_t bits);

#endif /* CONSOLE_H */
