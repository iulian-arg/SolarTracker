#ifndef TemperatureManager_H
#define TemperatureManager_H

// #include <Arduino.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include <BH1750.h>
#include <Wire.h>

// #define TEMT6000_0 32 // PIN on TEMT6000
// #define TEMT6000_1 33

const int oneWireBus = 4;
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

struct SensorInfo
{
    float temperatureC;
    float lux_0;
    float lux_1;
    float luxAverage;
    float luxDiffPercent;
};

class SensorManager
{
private:
    BH1750 lightMeter_0;
    BH1750 lightMeter_1;

    void SetupTeperatureSensor()
    {
        sensors.begin();
    }

    void SetupBH1750()
    {
        Wire.begin();
        if (lightMeter_0.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23))
        {
            Serial.println(F("BH1750_0 initialised"));
        }
        else
        {
            Serial.println(F("Error initialising BH1750_0"));
        }
        if (lightMeter_1.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x5C))
        {
            Serial.println(F("BH1750_1 initialised"));
        }
        else
        {
            Serial.println(F("Error initialising BH1750_1"));
        }
    }

    // float GetLuxDiff()
    // {
    //     float lux0 = ReadBH1750_0();
    //     float lux1 = ReadBH1750_1();

    //     return (lux0 / lux1) - 1.0;
    // }
    // float GetCurrentLuxAverage()
    // {
    //     float lux0 = ReadBH1750_0();
    //     float lux1 = ReadBH1750_1();
    //     return (lux0 + lux1) / 2.0;
    // }
    float ReadBH1750_0()
    {
        float lux = lightMeter_0.readLightLevel();
        return lux;
    }
    float ReadBH1750_1()
    {
        float lux = lightMeter_1.readLightLevel();
        return lux;
    }

    void ReadTemp()
    {
        sensors.requestTemperatures();
        float temperatureC = sensors.getTempCByIndex(0);
        Serial.print(temperatureC);
        Serial.println("ºC");
        // delay(5);
    }
    float GetSensorTemperature()
    {
        sensors.requestTemperatures();
        float temperatureC = sensors.getTempCByIndex(0);
        return temperatureC;
    }
    /**
        int ReadTEMT6K_0()
        {
            int lightLevel = analogRead(TEMT6000_0);
            return lightLevel;
        }

        int ReadTEMT6K_1()
        {
            int lightLevel = analogRead(TEMT6000_1);
            return lightLevel;
        }
    */
public:
    void SetupSensors()
    {
        SetupTeperatureSensor();
        SetupBH1750();
    }

    SensorInfo GetSensorInfo()
    {
        SensorInfo sensorInfo;
        sensorInfo.temperatureC = GetSensorTemperature();
        sensorInfo.lux_0 = ReadBH1750_0();
        sensorInfo.lux_0 = sensorInfo.lux_0 <= 0 ? 0.1 : sensorInfo.lux_0;
        sensorInfo.lux_1 = ReadBH1750_1();
        sensorInfo.lux_1 = sensorInfo.lux_1 <= 0 ? 0.1 : sensorInfo.lux_1;
        sensorInfo.luxAverage = (sensorInfo.lux_0 + sensorInfo.lux_1) / 2.0;
        sensorInfo.luxDiffPercent =
            sensorInfo.lux_0 >= sensorInfo.lux_1 ? -sensorInfo.lux_1 / sensorInfo.lux_0 * 100.0
                                                 : sensorInfo.lux_0 / sensorInfo.lux_1 * 100.0;
        Serial.println("");
        Serial.printf("<0, 1, avg, diff, temp>: <%.2f, %.2f, %.2f, %.2f, %.2f>\n",
                      sensorInfo.lux_0,
                      sensorInfo.lux_1,
                      sensorInfo.luxAverage,
                      sensorInfo.luxDiffPercent,
                      sensorInfo.temperatureC);
        // sensorInfo.luxDiffPercent =GetLuxDiff();
        return sensorInfo;
    }
};

#endif