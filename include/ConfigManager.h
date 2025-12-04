
#ifndef ConfigManager_H
#define ConfigManager_H

#include "LITTLEFS.h"
#include "FS.h"
#include "Logger.h"
#include <array>
#include <vector>
extern const char *TAG;
#define FORMAT_LITTLEFS_IF_FAILED true
struct wifiPairs
{
    String ssid;
    String password;
};
struct Config
{
    uint16_t RetryDelay;
    uint8_t RetryCount;
    String ntpServer;
    uint8_t R1_pin_MoveSouth;
    uint8_t R2_pin_MoveNorth;
    uint8_t R3_pin;
    uint8_t POT1_pin_MaxAngl;
    uint8_t R0_pin_Power;
    uint8_t B1_pin_Auto;
    uint8_t B2_pin_MoveNorth;
    uint8_t B3_pin_MoveSouth;
    uint8_t LED1_pin_Auto;
    uint16_t POT_Max_South_Val;
    uint16_t POT_Max_North_Val;
    uint16_t ledBlinkIntervalMs;
    uint16_t ledBlinkDurationMs;

    uint8_t lightDiffTreshold;
    int lowLightTreshold;
    int positioningUpdateIntervalMs;
    int lightTrackingQueueSize;
    int gmtOffset_sec;
    int daylightOffset_sec;
    wifiPairs wifis[10];
};
const char *filename = "/config.txt";
const char *configConfig;
std::vector<String> configLines;

class ConfigManager
{
private:
    void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
    {
        Serial.printf("Listing directory: %s\r\n", dirname);

        File root = fs.open(dirname);
        if (!root)
        {
            Serial.println("- failed to open directory");
            return;
        }
        if (!root.isDirectory())
        {
            Serial.println(" - not a directory");
            return;
        }

        File file = root.openNextFile();
        while (file)
        {
            if (file.isDirectory())
            {
                Serial.print("  DIR : ");
                Serial.println(file.name());
                if (levels)
                {
                    listDir(fs, file.path(), levels - 1);
                }
            }
            else
            {
                Serial.print("  FILE: ");
                Serial.print(file.name());
                Serial.print("\tSIZE: ");
                Serial.println(file.size());
            }
            file = root.openNextFile();
        }
    }


    void readConfigLines()
    {
        File file = LITTLEFS.open(filename, "r");
        if (!file)
        {
            Logger::error(TAG, "Failed to open file for reading");
            return;
        }
        int index = 0;
        String line = "";
        char c = 0;
        while (file.available())
        {
            line = "";
            while (file.available())
            {
                c = file.read();
                if (c == '\n')
                {
                    break;
                }
                line += c;
            }
            configLines.push_back(line);
            index++;
        }
        file.close();
    }

    String getConfigValue(const String &key)
    {
        for (const auto &line : configLines)
        {
            if (line.startsWith(key + " = "))
            {
                // Serial.printf("Found config for key %s: %s\n", key.c_str(), line.substring(key.length() + 3));
                String value = line.substring(key.length() + 3);
                value = value.substring(0, value.indexOf(';'));
                value.trim();
                Serial.printf("Config value for key %s:%s\n", key.c_str(), value.c_str());
                return value;
            }
        }
        return "";
    }

    void getWifiConfig(const String &key, String &ssid, String &password)
    {
        for (const auto &line : configLines)
        {
            if (line.startsWith(key + " = "))
            {
                String wifiConfig = line.substring(key.length() + 3);
                ssid = wifiConfig.substring(0, wifiConfig.indexOf(':'));
                password = wifiConfig.substring(wifiConfig.indexOf(':') + 1);
                password = password.substring(0, password.indexOf(';'));

                Logger::info(TAG, "WIFI Config: %s", wifiConfig.c_str());
            }
        }
    }

public:
    Config readConfig()
    {
        if (!LITTLEFS.begin(FORMAT_LITTLEFS_IF_FAILED))
        {
            Serial.println("LittleFS Mount Failed");
        }
        listDir(LITTLEFS, "/", 1);

        readConfigLines();

        Config cfg;

        cfg.ntpServer = getConfigValue("tmSet_ntpServer").c_str();
        cfg.gmtOffset_sec = atoi(getConfigValue("tmSet_gmtOffset_sec").c_str());
        cfg.daylightOffset_sec = atoi(getConfigValue("tmSet_daylightOffset_sec").c_str());
        cfg.ledBlinkIntervalMs = 5000;
        cfg.ledBlinkDurationMs = 500;

        cfg.lightDiffTreshold = atoi(getConfigValue("lightSensorSettings_lightDiffTreshold").c_str());
        cfg.lowLightTreshold = atoi(getConfigValue("lightSensorSettings_lowLightTreshold").c_str());
        cfg.lightTrackingQueueSize = atoi(getConfigValue("lightSensorSettings_lightTrackingQueueSize").c_str());
        cfg.positioningUpdateIntervalMs = atoi(getConfigValue("lightSensorSettings_positioningUpdateIntervalMs").c_str());

        cfg.RetryCount = (uint8_t)atoi(getConfigValue("wifis_RetryCount").c_str());
        cfg.RetryDelay = (uint16_t)atoi(getConfigValue("wifis_RetryDelay").c_str());

        cfg.R1_pin_MoveSouth = (uint8_t)atoi(getConfigValue("pinSettings_R1_pin_MoveSouth").c_str());
        cfg.R2_pin_MoveNorth = (uint8_t)atoi(getConfigValue("pinSettings_R2_pin_MoveNorth").c_str());
        cfg.R3_pin = (uint8_t)atoi(getConfigValue("pinSettings_R3_pin").c_str());
        cfg.POT1_pin_MaxAngl = (uint8_t)atoi(getConfigValue("pinSettings_POT1_pin_MaxAngl").c_str());
        cfg.R0_pin_Power = (uint8_t)atoi(getConfigValue("pinSettings_R0_pin_Power").c_str());
        cfg.B1_pin_Auto = (uint8_t)atoi(getConfigValue("pinSettings_B1_pin_Auto").c_str());
        cfg.B2_pin_MoveNorth = (uint8_t)atoi(getConfigValue("pinSettings_B2_pin_MoveNorth").c_str());
        cfg.B3_pin_MoveSouth = (uint8_t)atoi(getConfigValue("pinSettings_B3_pin_MoveSouth").c_str());
        cfg.LED1_pin_Auto = (uint8_t)atoi(getConfigValue("pinSettings_LED1_pin_Auto").c_str());
        cfg.POT_Max_South_Val = (uint16_t)atoi(getConfigValue("pinSettings_POT_Max_South_Val").c_str());
        cfg.POT_Max_North_Val = (uint16_t)atoi(getConfigValue("pinSettings_POT_Max_North_Val").c_str());

        for (int i = 0; i < 10; i++)
        {
            String ssid, password;
            getWifiConfig("wifis_pair" + String(i), ssid, password);
            cfg.wifis[i].ssid = ssid;
            cfg.wifis[i].password = password;
        }

        Logger::info(TAG, "Config read from doc: END");

        return cfg;
    }

    void WriteToLITTLEFS(const char *jsonConf)
    {
        File file;
        if (!LITTLEFS.begin(true))
        {
            Logger::error(TAG, "An Error has occurred while mounting LITTLEFS");
            return;
        }
        file = LITTLEFS.open(filename, "w+");
        if (!file)
        {
            Logger::error(TAG, "Failed to open file for writing");
            return;
        }
        file.print(jsonConf);
        file.close();
        Logger::info(TAG, "File closed.");
    }

    void UpdateJSONFromLITTLEFS()
    {
        File file;
        if (!LITTLEFS.begin(true))
        {
            Logger::error(TAG, "An Error has occurred while mounting LITTLEFS");
            return;
        }
        file = LITTLEFS.open(filename, "r+");
        if (!file)
        {
            Logger::error(TAG, "Failed to open file for reading");
            return;
        }
        configConfig = file.readString().c_str();
        file.close();
        Logger::info(TAG, "Config from LITTLEFS: %s", configConfig);
        Logger::info(TAG, "File closed.");
    }
};
#endif