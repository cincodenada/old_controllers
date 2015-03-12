#include "common.h";

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

