#include <stdint.h>
#include "common.h"
#include "joystick_status.h"
#include "bt_controller.h"

#define PACKET_BYTES 8
uint8_t btdata[PACKET_BYTES];

void init_bt() {
  delay(1000);
  HWSERIAL.begin(115200);  // The Bluetooth Mate defaults to 115200bps
  HWSERIAL.print("$");  // Print three times individually
  HWSERIAL.print("$");
  HWSERIAL.print("$");  // Enter command mode
  delay(100);  // Short delay, wait for the Mate to send back CMD
  while(HWSERIAL.available()) {
    Serial.print((char)HWSERIAL.read());
  }
  HWSERIAL.print("SH,0247\r");  // Set as joystick
  delay(100);  // Short delay, wait for the Mate to send back CMD
  while(HWSERIAL.available()) {
    Serial.print((char)HWSERIAL.read());  // Set as joystick
  }
  HWSERIAL.print("-");  // Print three times individually
  HWSERIAL.print("-");
  HWSERIAL.print("-");  // Enter command mode
  HWSERIAL.print("\r");
  delay(100);  // Short delay, wait for the Mate to send back CMD
  while(HWSERIAL.available()) {
    Serial.print((char)HWSERIAL.read());
  }
}

void send_bt(JoystickStatus *JoyStatus) {
  btdata[0] = 0xFD;
  btdata[1] = 0x06;
  btdata[2] = JoyStatus->axis[0] >> 8;
  btdata[3] = JoyStatus->axis[1] >> 8;
  btdata[4] = JoyStatus->axis[2] >> 8;
  btdata[5] = JoyStatus->axis[3] >> 8;
  btdata[6] = JoyStatus->buttonset[0];
  btdata[7] = JoyStatus->buttonset[1];

  printMsg("Axes: %d %d", JoyStatus->axis[0], JoyStatus->axis[1]);
  printMsg("Sending BT packet: ");
  for(int i=0;i<PACKET_BYTES;i++) {
    HWSERIAL.write((byte)btdata[i]);
    printMsg("%.2X", btdata[i]);
  }
  printMsg("");
}
