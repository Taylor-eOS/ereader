#include "utils.h"
#include <Arduino.h>

void safeLog(const char* format, ...) {
    if (Serial && Serial.availableForWrite() > 32) {
        va_list args;
        va_start(args, format);
        char buf[256];
        vsnprintf(buf, sizeof(buf), format, args);
        Serial.print(buf);
        va_end(args);
    }
}
