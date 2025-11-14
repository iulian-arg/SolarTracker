
#ifndef WifiManager_H
#define WifiManager_H

#include <WiFi.h>

// const char *ssid = "TP-Link_22F4";
// const char *password = "14756450";

// const char *ssid2 = "POCO X3 Pro";
// const char *password2 = "18273645";

// const char *ssid3 = "Asus";
// const char *password3 = "18273645";
/**
      [
        "POCO X3 Pro",
        "18273645"
      ],
      [
        "VIVOBOOK",
        "18273645"
      ],
      [
        "Asus",
        "18273645"
      ]*/
class WifiManager
{

public:
    WifiManager() {}
    void WifiConnect(Config config)
    {
        Serial.println();

        // Serial.printf("Default hostname: %s\n", WiFi.hostname().c_str());
        for (int i = 0; i <= 4; i++)
        {
            Serial.println();
            auto ssid = config.wifis[i].ssid;
            auto password = config.wifis[i].password;
            Serial.printf("\n_______connecting to ssid: %s \n", ssid.c_str());
            WiFi.hostname("fosa");
            WiFi.begin(ssid, password);

            int connAttempts = 0;
            while (WiFi.status() != WL_CONNECTED && connAttempts < config.RetryCount)
            {
                Serial.printf(".%d ", WiFi.status());
                connAttempts++;
                delay(config.RetryDelay);
            }

            if (WiFi.status() == WL_CONNECTED)
            {
                Serial.println("WiFi connected");
                Serial.println(WiFi.localIP());
                Serial.println(WiFi.macAddress());
                Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
                Serial.print(" | ");
                Serial.printf("%4d", WiFi.RSSI(i));
                Serial.print(" | ");
                Serial.printf("%2d", WiFi.channel(i));
                Serial.print(" | ");
                break;
            }
        }
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Failed to connect to WiFi");
        }
    }
};
#endif