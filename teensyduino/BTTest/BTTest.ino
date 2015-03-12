#include <SoftwareSerial.h>
#define HWSERIAL Serial1

int last_rx = 0;
int last_tx = 0;
int wait_cyc = 10;

//SoftwareSerial HWSERIAL(8,7);

void setup() {
    delay(1000);
    Serial.begin(9600);
    HWSERIAL.begin(115200);  // The Bluetooth Mate defaults to 115200bps
    HWSERIAL.print("$");  // Print three times individually
    HWSERIAL.print("$");
    HWSERIAL.print("$");  // Enter command mode
    delay(100);  // Short delay, wait for the Mate to send back CMD
    HWSERIAL.print("SH, 0217");  // Enter command mode
}

void loop() {
    int rx;
    int tx;
    if(HWSERIAL.available() > 0) {
        rx = HWSERIAL.read(); 
        Serial.print((char)rx);
    }

    if(Serial.available() > 0) {
        tx = Serial.read();
        Serial.print((char)tx);
        HWSERIAL.write(tx);
    }
}
