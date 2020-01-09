#include "serial_console.h"

#include "pin_config.h"

SerialConsole console;

void SerialConsole::out_impl(int level, const char* format, va_list args) {
#ifdef USB_SERIAL_MULTIJOY
  if(level > LOG_LEVEL) { return; }
  if(enabled) {
    int cur_len, i;
    cur_len = vsnprintf(msg, MSG_LEN, format, args);
    if(cur_len > max_len) { max_len = cur_len; }
    for(i=cur_len; i < max_len && i < MSG_LEN; i++) {
      msg[i] = ' ';
    }
    msg[i] = '\0';
    // This could maybe be >= but why risk it
    if (Serial.availableForWrite() > max_len) {
      Serial.flush();
      Serial.println(msg);
      Serial.flush();
    } else {
      // Don't fill our buffer
    }
  }
#endif
}

void SerialConsole::cls() {
#ifdef USB_SERIAL_MULTIJOY
  if(enabled && Serial.availableForWrite() > 10) {
  Serial.print("\033[2J");    // clear screen command
  Serial.print("\033[0;0H");   // cursor to home command
  Serial.flush();
  }
#endif
}
