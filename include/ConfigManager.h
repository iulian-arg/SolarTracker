
#ifndef ConfigManager_H
#define ConfigManager_H

#include <ArduinoJson.h>
#include "SPIFFS.h"

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
    uint8_t R1_pin_MoveLeft;
    uint8_t R2_pin_MoveRight;
    uint8_t R3_pin;
    uint8_t POT1_pin_MaxAngl;
    uint8_t R0_pin_Power;
    uint8_t B1_pin_Auto;
    uint8_t B2_pin_MoveRight;
    uint8_t B3_pin_MoveLeft;
    uint8_t LED1_pin_Auto;
    uint16_t POT_Max_Left_Val;
    uint16_t POT_Max_Right_Val;
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

class ConfigManager
{
public:
    DynamicJsonDocument GetJsonDocument()
    {
        Serial.println("Reading file from SPIFFS");
        File file;
        if (!SPIFFS.begin(true))
        {
            Serial.println("An Error has occurred while mounting SPIFFS");
            return DynamicJsonDocument(0);
        }
        file = SPIFFS.open(filename, "r+");
        if (!file)
        {
            Serial.println("Failed to open file for reading");
            return DynamicJsonDocument(0);
        }
        if (!file)
        {
            Serial.println("Returning default config");
            return DynamicJsonDocument(0);
        }
        Serial.println("Getting JSON document");
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, file);
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
        }
        file.close();
        Serial.println("File closed.");
        return doc;
    }

    Config ReadConfigFromDoc(DynamicJsonDocument doc)
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
            Serial.printf("\n __wifis: %s __ %s", ssid, pass);
            i++;
        }
        cfg.RetryCount = doc["wifis"]["RetryCount"].as<uint8_t>();
        cfg.RetryDelay = doc["wifis"]["RetryDelay"].as<uint16_t>();

        cfg.R1_pin_MoveLeft = doc["pinSettings"]["R1_pin_MoveLeft"].as<uint8_t>();
        cfg.R2_pin_MoveRight = doc["pinSettings"]["R2_pin_MoveRight"].as<uint8_t>();
        cfg.R3_pin = doc["pinSettings"]["R3_pin"].as<uint8_t>();
        cfg.POT1_pin_MaxAngl = doc["pinSettings"]["POT1_pin_MaxAngl"].as<uint8_t>();
        cfg.R0_pin_Power = doc["pinSettings"]["R0_pin_Power"].as<uint8_t>();
        cfg.B1_pin_Auto = doc["pinSettings"]["B1_pin_Auto"].as<uint8_t>();
        cfg.B2_pin_MoveRight = doc["pinSettings"]["B2_pin_MoveRight"].as<uint8_t>();
        cfg.B3_pin_MoveLeft = doc["pinSettings"]["B3_pin_MoveLeft"].as<uint8_t>();
        cfg.LED1_pin_Auto = doc["pinSettings"]["LED1_pin_Auto"].as<uint8_t>();
        cfg.POT_Max_Left_Val = doc["pinSettings"]["POT_Max_Left_Val"].as<uint16_t>();
        cfg.POT_Max_Right_Val = doc["pinSettings"]["POT_Max_Right_Val"].as<uint16_t>();

        Serial.println("Config read from doc:");
        
        return cfg;
    }

    Config readConfig()
    {
        Config config;

        DynamicJsonDocument doc = GetJsonDocument();
        if (doc.isNull())
        {
            Serial.println("Returning default config");
            return config;
        }
        config = ReadConfigFromDoc(doc);

        Serial.println("printJson");
        printJson(doc);
        Serial.println("end printJson");

        return config;
    }

    void printJson(DynamicJsonDocument doc)
    {
        serializeJsonPretty(doc, Serial);
        Serial.println();
    }

    void WriteToSPIFFS(Config config)
    {
        if (!SPIFFS.begin(true))
        {
            Serial.println("An Error has occurred while mounting SPIFFS");
            return;
        }
        File file = SPIFFS.open(filename, "w+");
        if (!file)
        {
            Serial.println("Failed to open file for writing");
            return;
        }

        DynamicJsonDocument doc(1024);

        JsonArray pairs = doc.createNestedObject("wifis").createNestedArray("pairs");
        for (int i = 0; i < 5; i++)
        {
            JsonArray pair = pairs.createNestedArray();
            pair.add(config.wifis[i].ssid);
            pair.add(config.wifis[i].password);
        }

        serializeJsonPretty(doc, Serial);
        Serial.println();

        if (serializeJsonPretty(doc, file) == 0)
        {
            Serial.println(F("Failed to write to file"));
        }
        file.close();
        Serial.println("File closed.");
    }
};
#endif