#include <stdio.h>
#include <stdint.h>

#include "pin_config.h"

#define TESTCTL 0x04
#define MSG_LEN 100

char msg[MSG_LEN];
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

int curval;
int mode;
int cycle = 0;
void setup() {
    //Do some port setup
    //Set N64 to output (high)
    //And initialize as NES for safety
    N64_PORT |= TESTCTL << N64_SHIFT;
    N64_DIR |= TESTCTL << N64_SHIFT;
    //Set up SNES DIR
    //Initialize to low (NES)
    SNES_PORT &= ~(TESTCTL << SNES_SHIFT);
    SNES_DIR |= TESTCTL << SNES_SHIFT;
    //Set up clock/latch
    CLOCK_DIR &= ~CLOCK_MASK;
    LATCH_DIR &= ~LATCH_MASK;

    //Set up data port to be input, pull-up
    DATA_PORT |= TESTCTL << DATA_SHIFT;
    DATA_DIR &= ~(TESTCTL << DATA_SHIFT);
}

void loop() {
    curval = cycle/2;
    mode = cycle%2;

    if(curval) {
        SNES_PORT |= TESTCTL << SNES_SHIFT;
    } else {
        SNES_PORT &= ~(TESTCTL << SNES_SHIFT);
    }

    if(mode) {
        LATCH_PORT |= LATCH_MASK;
        CLOCK_PORT |= CLOCK_MASK;
        DATA_PORT |= TESTCTL << DATA_SHIFT;
    } else {
        LATCH_PORT &= ~(LATCH_MASK);
        CLOCK_PORT &= ~(CLOCK_MASK);
        DATA_PORT &= ~(TESTCTL << DATA_SHIFT);
    }

    printMsg("Set for %s, %s", curval ? "SNES" : "NES", mode ? "pull-up" : "Hi-Z");
    printMsg("Clock: %d\r\nLatch:%d\r\nData:%d",
        !!(CLOCK_IN & CLOCK_MASK),
        !!(LATCH_IN & LATCH_MASK),
        !!(DATA_IN & TESTCTL)
    );

    cycle++; if(cycle > 3) { cycle = 0; }

    delay(1000); 
}
