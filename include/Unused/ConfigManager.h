
#ifndef ConfigManager_H
#define ConfigManager_H

// #include <ArduinoJson.h>
// #include "SPIFFS.h"
#include "LITTLEFS.h"
#include "Logger.h"
extern const char *TAG;

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

    float lightDiffTreshold;
    int lowLightTreshold;
    int positioningUpdateIntervalMs;
    int lightTrackingQueueSize;
    int gmtOffset_sec;
    int daylightOffset_sec;
    wifiPairs wifis[10];
};
const char *filename = "/config.json";
const char *configConfig;
class ConfigManager
{
private:
    void readFile(fs::FS &fs, const char *path)
    {
        Serial.printf("Reading file: %s\r\n", path);

        File file = fs.open(path);
        if (!file || file.isDirectory())
        {
            Serial.println("- failed to open file for reading");
            return;
        }

        Serial.println("- read from file:");
        while (file.available())
        {
            Serial.write(file.read());
        }
        file.close();
    }

    void writeFile(fs::FS &fs, const char *path, const char *message)
    {
        Serial.printf("Writing file: %s\r\n", path);

        File file = fs.open(path, FILE_WRITE);
        if (!file)
        {
            Serial.println("- failed to open file for writing");
            return;
        }
        if (file.print(message))
        {
            Serial.println("- file written");
        }
        else
        {
            Serial.println("- write failed");
        }
        file.close();
    }

public:
    DynamicJsonDocument GetJsonDocument()
    {
        if (!LITTLEFS.begin())
        {
            Logger::error(TAG, "LITTLEFS Mount Failed");
            return DynamicJsonDocument(0);
        }
        Logger::info(TAG, "Reading file from LITTLEFS");
        File file;
        file = LITTLEFS.open(filename, "r+");
        if (!file)
        {
            Logger::error(TAG, "Failed to open file for reading");
            return DynamicJsonDocument(0);
        }
        Logger::info(TAG, "Getting JSON document");
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, file);
        if (error)
        {
            Logger::error(TAG, "deserializeJson() failed: ");
            Logger::error(TAG, error.c_str());
        }
        file.close();
        Logger::info(TAG, "File closed.");
        return doc;
    }

    Config ReadConfigFromFile(DynamicJsonDocument doc)
    {
        Config cfg;
        cfg.ntpServer = doc["tmSet"]["ntpServer"].as<const char *>();
        cfg.gmtOffset_sec = doc["tmSet"]["gmtOffset_sec"].as<int>();
        cfg.daylightOffset_sec = doc["tmSet"]["daylightOffset_sec"].as<int>();
        cfg.ledBlinkIntervalMs = 5000;
        cfg.ledBlinkDurationMs = 500;

        cfg.lightDiffTreshold = doc["lightSensorSettings"]["lightDiffTreshold"].as<float>();
        cfg.lowLightTreshold = doc["lightSensorSettings"]["lowLightTreshold"].as<int>();
        cfg.lightTrackingQueueSize = doc["lightSensorSettings"]["lightTrackingQueueSize"].as<int>();
        cfg.positioningUpdateIntervalMs = doc["lightSensorSettings"]["positioningUpdateIntervalMs"].as<int>();

        int i = 0;
        for (JsonVariant v : doc["wifis"]["pairs"].as<JsonArray>())
        {
            auto ssid = v[0].as<const char *>();
            auto pass = v[1].as<const char *>();
            cfg.wifis[i].ssid = ssid;
            cfg.wifis[i].password = pass;
            Logger::info(TAG, "\n __wifis: %s __ %s", ssid, pass);
            i++;
        }
        cfg.RetryCount = doc["wifis"]["RetryCount"].as<uint8_t>();
        cfg.RetryDelay = doc["wifis"]["RetryDelay"].as<uint16_t>();

        cfg.R1_pin_MoveSouth = doc["pinSettings"]["R1_pin_MoveSouth"].as<uint8_t>();
        cfg.R2_pin_MoveNorth = doc["pinSettings"]["R2_pin_MoveNorth"].as<uint8_t>();
        cfg.R3_pin = doc["pinSettings"]["R3_pin"].as<uint8_t>();
        cfg.POT1_pin_MaxAngl = doc["pinSettings"]["POT1_pin_MaxAngl"].as<uint8_t>();
        cfg.R0_pin_Power = doc["pinSettings"]["R0_pin_Power"].as<uint8_t>();
        cfg.B1_pin_Auto = doc["pinSettings"]["B1_pin_Auto"].as<uint8_t>();
        cfg.B2_pin_MoveNorth = doc["pinSettings"]["B2_pin_MoveNorth"].as<uint8_t>();
        cfg.B3_pin_MoveSouth = doc["pinSettings"]["B3_pin_MoveSouth"].as<uint8_t>();
        cfg.LED1_pin_Auto = doc["pinSettings"]["LED1_pin_Auto"].as<uint8_t>();
        cfg.POT_Max_South_Val = doc["pinSettings"]["POT_Max_South_Val"].as<uint16_t>();
        cfg.POT_Max_North_Val = doc["pinSettings"]["POT_Max_North_Val"].as<uint16_t>();

        Logger::info(TAG, "Config read from doc:");

        return cfg;
    }

    Config readConfig()
    {
        Config config;

        DynamicJsonDocument doc = GetJsonDocument();
        if (doc.isNull())
        {
            Logger::warn(TAG, "Returning default config");
            return config;
        }
        config = ReadConfigFromFile(doc);

        Logger::info(TAG, "printJson");
        printJson(doc);
        Logger::info(TAG, "end printJson");

        return config;
    }

    void printJson(DynamicJsonDocument doc)
    {
        serializeJsonPretty(doc, Serial);
        void *buffer = malloc(2048);
        size_t size = serializeJsonPretty(doc, buffer, 2048);
        configConfig = static_cast<const char *>(buffer);
        Logger::info(TAG, configConfig);
        Logger::info(TAG, "Serialized JSON printed to Serial");
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