

#ifndef TimeManager_H
#define TimeManager_H

#include <time.h>
// #include <Arduino.h>

const char *ntpServer = "pool.ntp.org";
int gmtOffset_sec = 7200;
int daylightOffset_sec = 3600;
struct tm timeinfo;

class TimeManager
{

public:
    TimeManager() {}
    void initTime(Config config)
    {
        // Initialize NTP

        configTime(config.gmtOffset_sec, config.daylightOffset_sec, config.ntpServer.c_str());
        if (!getLocalTime(&timeinfo))
        {
            Serial.println("Failed to obtain time from: " + config.ntpServer);
            return;
        }
        else
        {
            Serial.println("Time obtained from: " + config.ntpServer);
        }
        printCurrentTime();
    }

    void updateTime()
    {
        if (!getLocalTime(&timeinfo))
        {
            Serial.println("Failed to obtain time");
            return;
        }
        printCurrentTime();
    }

    void printCurrentTime()
    {
        time_t now;
        time(&now);
        struct tm *t_info = localtime(&now);
        // Serial.println(t_info, "%A, %B %d %Y %H:%M:%S");
        Serial.print(t_info, "%H:%M:%S");

        // Serial.printf("Current hour: %02d\n", t_info->tm_hour);
        // Serial.printf("Current minute: %02d\n", t_info->tm_min);
        // Serial.printf("Current second: %02d\n", t_info->tm_sec);
    }
    void printTime(struct tm *t_info)
    {
        Serial.println(t_info, "%A, %B %d %Y %H:%M:%S");
    }
};
#endif