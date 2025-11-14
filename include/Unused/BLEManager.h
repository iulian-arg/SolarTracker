
#ifndef BLEManager_H
#define BLEManager_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp32-hal.h>

// BLE server name
#define bleServerName "BME280_ESP32"

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

bool deviceConnected = false;

#define SERVICE_UUID "8a21a3bc-209c-4341-b993-218cd7162316"

// Temperature Characteristic and Descriptor
BLECharacteristic bmeTemperatureCelsiusCharacteristics("cba1d466-344c-4be3-ab3f-189f80dd7518", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeTemperatureCelsiusDescriptor(BLEUUID((uint16_t)0x2902));

// Humidity Characteristic and Descriptor
BLECharacteristic bmeHumidityCharacteristics("ca73b3ba-39f6-4ab3-91ae-186dc9577d99", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeHumidityDescriptor(BLEUUID((uint16_t)0x2903));

// Setup callbacks onConnect and onDisconnect
class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        //Serial.println("device connected");
        deviceConnected = true;
    };
    void onDisconnect(BLEServer *pServer)
    {
        //Serial.println("device disconnected");
        deviceConnected = false;
    }
};

class BLEManager
{
private:
public:
    void setupBLE()
    {
        // Create the BLE Device
        BLEDevice::init(bleServerName);

        // Create the BLE Server
        BLEServer *pServer = BLEDevice::createServer();
        pServer->setCallbacks(new MyServerCallbacks());

        // Create the BLE Service
        BLEService *bmeService = pServer->createService(SERVICE_UUID);

        // Create BLE Characteristics and Create a BLE Descriptor
        // Temperature
        bmeService->addCharacteristic(&bmeTemperatureCelsiusCharacteristics);
        bmeTemperatureCelsiusDescriptor.setValue("BME temperature Celsius");
        bmeTemperatureCelsiusCharacteristics.addDescriptor(&bmeTemperatureCelsiusDescriptor);

        // Humidity
        bmeService->addCharacteristic(&bmeHumidityCharacteristics);
        bmeHumidityDescriptor.setValue("BME humidity");
        bmeHumidityCharacteristics.addDescriptor(new BLE2902());

        // Start the service
        bmeService->start();

        // Start advertising
        BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(SERVICE_UUID);
        pServer->getAdvertising()->start();

        //Serial.println("Waiting a client connection to notify...");
    }

    void loopBLE()
    {
        if (deviceConnected)
        {
            if ((millis() - lastTime) > timerDelay)
            {
                // Read temperature as Celsius (the default)
                auto temp = 10;
                // Read humidity
                auto hum = 80;

                // Notify temperature reading from BME sensor
                static char temperatureCTemp[6];
                // dtostrf(temp, 6, 2, temperatureCTemp);
                // Set temperature Characteristic value and notify connected client
                bmeTemperatureCelsiusCharacteristics.setValue(temperatureCTemp);
                bmeTemperatureCelsiusCharacteristics.notify();
                //Serial.print("Temperature Celsius: ");
                //Serial.print(temp);
                //Serial.print(" ºC");

                // Notify humidity reading from BME
                static char humidityTemp[6];
                //   dtostrf(hum, 6, 2, humidityTemp);
                // Set humidity Characteristic value and notify connected client
                bmeHumidityCharacteristics.setValue(humidityTemp);
                bmeHumidityCharacteristics.notify();
                //Serial.print(" - Humidity: ");
                //Serial.print(hum);
                //Serial.println(" %");

                lastTime = millis();
            }
        }
    }
};
#endif