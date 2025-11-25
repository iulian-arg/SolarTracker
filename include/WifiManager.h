
#ifndef WifiManager_H
#define WifiManager_H

#include <WiFi.h>
#include "ConfigManager.h"
#include "Logger.h"

extern const char *TAG;

extern Config config;

class WifiManager
{

public:
  WifiManager() {}
  void WifiConnect()
  {
    for (int i = 0; i <= 1; i++)
    {
      auto ssid = config.wifis[i].ssid;
      auto password = config.wifis[i].password;
      Logger::info(TAG, "--connecting to ssid: %s \n", ssid.c_str());
      WiFi.hostname("SolarTracker");
      WiFi.begin(ssid, password);

      int connAttempts = 0;
      while (WiFi.status() != WL_CONNECTED && connAttempts < config.RetryCount)
      {
        Logger::info(TAG, ".%d ", WiFi.status());
        connAttempts++;
        delay(config.RetryDelay);
      }

      if (WiFi.status() == WL_CONNECTED)
      {
        Logger::info(TAG, "WiFi connected");
        Logger::info(TAG, "IP address:  %s", WiFi.localIP().toString().c_str());
        Logger::info(TAG, "MAC address: %s", WiFi.macAddress().c_str());
        break;
      }
    }
    if (WiFi.status() != WL_CONNECTED)
    {
      Logger::error(TAG, "Failed to connect to WiFi");
    }
  }
};
#endif
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