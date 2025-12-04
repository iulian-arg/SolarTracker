#pragma once
// #include <Arduino.h>
#include <sys/time.h>
#include <deque>

class Logger {
public:
    // Initialize with max queue size
    static void begin(size_t maxSize = 100) {
        maxQueueSize = maxSize;
    }

    static void info(const char* tag, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log("INFO", tag, fmt, args);
        va_end(args);
    }

    static void warn(const char* tag, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log("WARN", tag, fmt, args);
        va_end(args);
    }

    static void error(const char* tag, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log("ERROR", tag, fmt, args);
        va_end(args);
    }

    static void debug(const char* tag, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log("DEBUG", tag, fmt, args);
        va_end(args);
    }

    // Retrieve all logs in bulk
    static std::deque<String> getLogs() {
        return logQueue;
    }

    // Clear the queue
    static void clearLogs() {
        logQueue.clear();
    }

private:
    static std::deque<String> logQueue;
    static size_t maxQueueSize;

    static void log(const char* level, const char* tag, const char* fmt, va_list args) {
        char message[256];
        vsnprintf(message, sizeof(message), fmt, args);

        // Timestamp
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        time_t now = tv.tv_sec;
        struct tm* timeinfo = localtime(&now);

        char timestamp[32];
        strftime(timestamp, sizeof(timestamp), "%H:%M:%S", timeinfo);

        String logEntry = String("[") + timestamp + "] [" + level + "] " + tag + ": " + message;

        // Print immediately to Serial
        Serial.println(logEntry);

        // Store in queue
        if (logQueue.size() >= maxQueueSize) {
            logQueue.pop_front();  // remove oldest
        }
        logQueue.push_back(logEntry);
    }
};

// Static member initialization
std::deque<String> Logger::logQueue;
size_t Logger::maxQueueSize = 100;
