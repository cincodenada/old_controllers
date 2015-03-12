#include <stdio.h>
#include <stdint.h>

#include "pin_config.h"

#define TESTCTL 0x0F
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

char states[4][6] = {
    "NES",
    "SNES",
    "???",
    "Empty"
};


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

    DATA_DIR &= ~(TESTCTL << DATA_SHIFT);
    DATA_PORT |= TESTCTL << DATA_SHIFT;

    LATCH_DIR &= ~LATCH_MASK;
    LATCH_PORT |= ~LATCH_MASK;
    CLOCK_DIR &= ~CLOCK_MASK;
    CLOCK_PORT |= ~CLOCK_MASK;
}

void loop() {
    int curval = cycle & 0x01;

    if(curval) {
        N64_PORT |= TESTCTL << N64_SHIFT;
    } else {
        N64_PORT &= ~TESTCTL << N64_SHIFT;
    }
    printMsg("Set for %s", curval ? "SNES/NES" : "N64");
    delay(1000);

    cycle++;
}
/*
void loop() {
    int curval = cycle & 0x01;
    int mode = cycle & 0x02;

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

void loop() {
    SNES_PORT &= ~(TESTCTL << SNES_SHIFT);

    LATCH_DIR &= ~LATCH_MASK;
    LATCH_PORT |= LATCH_MASK;

    int latchval = !!(LATCH_IN & LATCH_MASK);
    int dataval = !!(DATA_IN & TESTCTL);

    printMsg(states[dataval | (latchval << 1)]);

    delay(1000); 
}
*/
