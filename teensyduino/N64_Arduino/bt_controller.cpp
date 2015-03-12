#include "bt_controller.h"

#define PACKET_BYTES 8
uint8_t btdata[PACKET_BYTES];

void init_bt() {
    delay(1000);
    Serial.begin(9600);
    HWSERIAL.begin(115200);  // The Bluetooth Mate defaults to 115200bps
    HWSERIAL.print("$");  // Print three times individually
    HWSERIAL.print("$");
    HWSERIAL.print("$");  // Enter command mode
    delay(100);  // Short delay, wait for the Mate to send back CMD
    while(HWSERIAL.available()) {
      Serial.print((char)HWSERIAL.read());  // Set as joystick
    }
    HWSERIAL.print("SH,0217\r");  // Set as joystick
    delay(100);  // Short delay, wait for the Mate to send back CMD
    while(HWSERIAL.available()) {
      Serial.print((char)HWSERIAL.read());  // Set as joystick
    }
    HWSERIAL.print("-");  // Print three times individually
    HWSERIAL.print("-");
    HWSERIAL.print("-");  // Enter command mode
    HWSERIAL.print("\r");  // Set as joystick
    delay(100);  // Short delay, wait for the Mate to send back CMD
    while(HWSERIAL.available()) {
      Serial.print((char)HWSERIAL.read());  // Set as joystick
    }
}

void send_bt(struct JoystickStatusStruct *JoyStatus) {
    btdata[0] = 0xFD;
    btdata[1] = 0x06;
    btdata[2] = JoyStatus->axis[0]/BT_FACT + BT_OFFSET;
    btdata[3] = JoyStatus->axis[1]/BT_FACT + BT_OFFSET;
    btdata[4] = JoyStatus->axis[0]/BT_FACT + BT_OFFSET;
    btdata[5] = JoyStatus->axis[1]/BT_FACT + BT_OFFSET;
    btdata[6] = JoyStatus->buttonset[0];
    btdata[7] = JoyStatus->buttonset[1];
    Serial.print("Sending BT packet: ");
    for(int i=0;i<PACKET_BYTES;i++) {
      HWSERIAL.write(btdata[i]);
      printMsg("%.2X", btdata[i]);
    }
    Serial.println();
}
