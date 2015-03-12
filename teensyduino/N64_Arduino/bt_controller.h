#ifndef BTCONTROLLER_H
#define BTCONTROLLER_H

#include <stdint.h>

#include "common.h"

#define BT_FACT ((AXIS_MAX-AXIS_MIN+1)/256)
#define BT_OFFSET (128)

#define HWSERIAL Serial1

void init_bt();

void send_bt(struct JoystickStatusStruct *JoyStatus);

#endif /* BTCONTROLLER_H */

