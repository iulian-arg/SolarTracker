#pragma once
#include "esp_log.h"
#include <Arduino.h>  // Required for Serial

class Logger {
public:
    static void info(const char* tag, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        esp_log_write(ESP_LOG_INFO, tag, fmt, args);
        va_end(args);

        va_start(args, fmt);
        printToSerial("INFO", tag, fmt, args);
        va_end(args);
    }

    static void warn(const char* tag, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        esp_log_write(ESP_LOG_WARN, tag, fmt, args);
        va_end(args);

        va_start(args, fmt);
        printToSerial("WARN", tag, fmt, args);
        va_end(args);
    }

    static void error(const char* tag, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        esp_log_write(ESP_LOG_ERROR, tag, fmt, args);
        va_end(args);

        va_start(args, fmt);
        printToSerial("ERROR", tag, fmt, args);
        va_end(args);
    }

    static void debug(const char* tag, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        esp_log_write(ESP_LOG_DEBUG, tag, fmt, args);
        va_end(args);

        va_start(args, fmt);
        printToSerial("DEBUG", tag, fmt, args);
        va_end(args);
    }

private:
    static void printToSerial(const char* level, const char* tag, const char* fmt, va_list args) {
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        Serial.printf("[%s] %s: %s\n", level, tag, buffer);
    }
};