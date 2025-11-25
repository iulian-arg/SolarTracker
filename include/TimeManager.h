

#ifndef TimeManager_H
#define TimeManager_H

#include <time.h>
// #include <Arduino.h>

extern Config config;
const char *ntpServer = "pool.ntp.org";
int gmtOffset_sec = 7200;
int daylightOffset_sec = 3600;
struct tm timeinfo;

class TimeManager
{

public:
    TimeManager() {}
    void initTime()
    {
        // Initialize NTP

        configTime(config.gmtOffset_sec, config.daylightOffset_sec, config.ntpServer.c_str());
        if (!getLocalTime(&timeinfo))
        {
            Logger::error(TAG, "Failed to obtain time from: %c", config.ntpServer.c_str());
            return;
        }
        else
        {
            Logger::info(TAG, "Time obtained from: %c", config.ntpServer.c_str());
        }
        // printCurrentTime();
    }

    void updateTime()
    {
        if (!getLocalTime(&timeinfo))
        {
            Logger::error(TAG, "Failed to obtain time");
            return;
        }
        // printCurrentTime();
    }

    // void printCurrentTime()
    // {
    //     time_t now;
    //     time(&now);
    //     struct tm *t_info = localtime(&now);
    //     Serial.print(t_info, "%H:%M:%S");
    // }
    // void printTime(struct tm *t_info)
    // {
    //     Serial.println(t_info, "%A, %B %d %Y %H:%M:%S");
    // }
};
#endif