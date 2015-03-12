#ifndef COMMON_H
#define COMMON_H

#define AXIS_MAX (32767)
#define AXIS_MIN (-32768)
#define JOY_FACT ((AXIS_MAX-AXIS_MIN+1)/1024)
#define JOY_OFFSET (512)

#define MSG_LEN 100
extern char msg[MSG_LEN];

#include "Arduino.h"

enum controller_type_t {
    NES = 0,
    SNES,
    N64
}

// 8 bytes of data that we get from the controller
struct JoystickStatusStruct {
    signed short int axis[3];
    signed short int hat;
    uint8_t buttonset[2];
    controller_type_t controller_type;
};

void printMsg(const char* format, ...);

#endif /* COMMON_H */
