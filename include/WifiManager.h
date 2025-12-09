
#ifndef WifiManager_H
#define WifiManager_H

#include <WiFi.h>
#include "ConfigManager.h"
#include "Logger.h"
#include <WiFiMulti.h>
#include "TimeManager.h"

WiFiMulti wifiMulti;

extern const char *TAG;
extern TimeManager *timeManager;

extern Config config;

class WifiManager
{

  std::vector<String> availableSSIDs;

public:
  WifiManager() {}

  void scanNetworks()
  {
    availableSSIDs.clear();
    // scan for nearby networks:
    Logger::info(TAG, "** Scan Networks **");
    byte numSsid = WiFi.scanNetworks();

    // print the list of networks seen:
    Logger::info(TAG, "SSID List: %d networks found", numSsid);

    // print the network number and name for each network found:
    for (int thisNet = 0; thisNet < numSsid; thisNet++)
    {
      Logger::info(TAG, "%d) Network: %s", thisNet, WiFi.SSID(thisNet).c_str());
      availableSSIDs.push_back(WiFi.SSID(thisNet));
    }
  }

  void WifiConnect()
  {
    WiFi.mode(WIFI_STA);
    WiFi.hostname("SolarTracker");

    scanNetworks();
    String ssid;
    String password;

    for (int i = 0; i <= 10; i++)
    {
      ssid = config.wifis[i].ssid;
      password = config.wifis[i].password;
      if (ssid.length() == 0)
      {
        continue;
      }
      wifiMulti.addAP(ssid.c_str(), password.c_str());
    }

    if (wifiMulti.run(config.RetryTotalInterval) == WL_CONNECTED)
    {
      Logger::info(TAG, "WiFi connected. SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());

      timeManager->initTime();
    }
    else
    {
      Logger::error(TAG, "WiFi not connected");
    }
  }

  const char *getWifiStatus(int status)
  {
    switch (status)
    {
    case WL_NO_SHIELD:
      return "WL_NO_SHIELD";
    case WL_IDLE_STATUS:
      return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL:
      return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED:
      return "WL_SCAN_COMPLETED";
    case WL_CONNECTED:
      return "WL_CONNECTED";
    case WL_CONNECT_FAILED:
      return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST:
      return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED:
      return "WL_DISCONNECTED";
    default:
      return "UNKNOWN_STATUS";
    }
    return "UNKNOWN_STATUS";
  }

  String GetWifiIpAndSSID()
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      return String("SSID: ") + WiFi.SSID() + String(", IP: ") + WiFi.localIP().toString();
    }
    else
    {
      return String("Not connected to WiFi");
    }
  }
};
#endif