#pragma once
#include <stdint.h>

#define NUM_BITS 8
extern char binstr[NUM_BITS+1];

void printBin(char* dest, char input, unsigned char num_bits = NUM_BITS);
void blink_binary(int num, uint8_t bits);
