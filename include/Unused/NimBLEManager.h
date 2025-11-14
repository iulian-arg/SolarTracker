/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#ifndef NimBLEManager_H
#define NimBLEManager_H

#include <NimBLEDevice.h>
#include <RelayManager.h>
#include <ProgramManager.h>
// #include "../../../../../Users/iulia/.platformio/packages/toolchain-xtensa32/xtensa-esp32-elf/include/c++/5.2.0/bits/basic_string.h"

// #include <BLEDevice.h>
// #include <BLEUtils.h>
// #include <BLEServer.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define LPG_SERVICE_NAME "LPGESP32"

bool deviceConnected = false;
bool oldDeviceConnected = false;
NimBLECharacteristic *pCharacteristic = nullptr;
static Program staticProgram;
static bool updatingBLE;

class MyServerCallbacks : public NimBLEServerCallbacks
{
    void onConnect(NimBLEServer *pServer)
    {
        Serial.println("BLE - Device connected");

        deviceConnected = true;
    };

    void onDisconnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
    {
        // Peer disconnected, add them to the whitelist
        // This allows us to use the whitelist to filter connection attempts
        // which will minimize reconnection time.
        Serial.println("BLE - Device disconnected");
        NimBLEDevice::whiteListAdd(NimBLEAddress(desc->peer_ota_addr));
        deviceConnected = false;
    }

    // void onWrite(NimBLECharacteristic *pCharacteristic)
    // {
    //     Serial.println("BLE - onWrite");
    // }
    void onRead(NimBLECharacteristic *pCharacteristic)
    {
        Serial.println("BLE - onRead");
    }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
    Program *localProgram;

public:
    MyCallbacks(Program *Program) : localProgram(Program) {}

    void onWrite(BLECharacteristic *pCharacteristic)
    {
        auto RelayValue = pCharacteristic->getValue();

        if (RelayValue.length() > 0)
        {
            updatingBLE = true;
            String command = "";
            Serial.print("------------ RECEIVE ------------ ");
            Serial.println(millis());
            uint8_t bRelayValues = (uint8_t)RelayValue.data()[0];
            uint8_t bCommandValue = (uint8_t)RelayValue.data()[1];
            uint8_t bTempValue = (uint8_t)RelayValue.data()[2];
            Serial.printf("BLE - received values: relays= %d, command= %d, temp= %d\n",
                          bRelayValues, bCommandValue, bTempValue);

            localProgram->UpdateProgramFromRemote(bRelayValues, bCommandValue);
            staticProgram = *localProgram;
            Serial.println("^^");
            updatingBLE = false;
        }
    }
};

void onAdvComplete(NimBLEAdvertising *pAdvertising)
{
    Serial.println("BLE - Advertising stopped");
    if (deviceConnected)
    {
        return;
    }
    // If advertising timed out without connection start advertising without whitelist filter
    pAdvertising->setScanFilter(false, false);
    pAdvertising->start();
}

class NimBLEManager
{

private:
    ulong prevMS = 0;

public:
    void setupNimBLE(Program *currentProgram)
    {
        Serial.println("BLE - Starting BLE work!");
        prevMS = millis() / 1000;

        BLEDevice::init(LPG_SERVICE_NAME);
        BLEServer *pServer = BLEDevice::createServer();
        pServer->setCallbacks(new MyServerCallbacks());
        pServer->advertiseOnDisconnect(true);

        BLEService *pService = pServer->createService(SERVICE_UUID);

        pCharacteristic = pService->createCharacteristic(
            CHARACTERISTIC_UUID,
            NIMBLE_PROPERTY::READ |
                // NIMBLE_PROPERTY::READ_ENC |
                // NIMBLE_PROPERTY::READ_AUTHEN |
                // NIMBLE_PROPERTY::READ_AUTHOR |
                NIMBLE_PROPERTY::WRITE |
                // NIMBLE_PROPERTY::WRITE_NR |
                // NIMBLE_PROPERTY::WRITE_ENC |
                // NIMBLE_PROPERTY::WRITE_AUTHEN |
                // NIMBLE_PROPERTY::WRITE_AUTHOR |
                // NIMBLE_PROPERTY::BROADCAST |
                NIMBLE_PROPERTY::NOTIFY //|
            // NIMBLE_PROPERTY::INDICATE
        );

        pCharacteristic->setValue("Hello");
        pCharacteristic->notify(true);
        pCharacteristic->setCallbacks(new MyCallbacks(currentProgram));

        pService->start();
        // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
        BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(SERVICE_UUID);
        // pAdvertising->setScanResponse(true);
        pAdvertising->setScanResponse(false);
        // pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
        // pAdvertising->setMaxPreferred(0x12);
        pAdvertising->start();
        // BLEDevice::startAdvertising();
        Serial.println("BLE - Characteristic defined! Now you can read it in your phone!");
    }

    void loopNimBLE(Program *currentProgram)
    {
        ulong currentMs = millis(); // / 1000
        // ------------ SEND ------------
        if (deviceConnected)
        {
            uint8_t returnValue[4];

            returnValue[0] = currentProgram->relayStates.GetRelaysStates();
            returnValue[1] = (u8_t)currentProgram->type;
            returnValue[2] = currentProgram->relayStates.GetTempReturn();
            Serial.print("------------ SEND ------------ ");
            Serial.println(currentMs);
            Serial.printf("BLE - sent values: relays= %d, command= %d, temp= %d \n",
                          (int)returnValue[0], (int)returnValue[1], (int)returnValue[2]);

            pCharacteristic->setValue((uint8_t *)&returnValue, 4);
            pCharacteristic->notify();
            prevMS = currentMs;
        }
        // disconnecting
        if (!deviceConnected && oldDeviceConnected)
        {
            Serial.println("BLE - !deviceConnected && oldDeviceConnected");

            NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
            if (NimBLEDevice::getWhiteListCount() > 0)
            {
                // Allow anyone to scan but only whitelisted can connect.
                pAdvertising->setScanFilter(false, true);
            }
            // advertise with whitelist for 30 seconds
            pAdvertising->start(30, onAdvComplete);
            Serial.println("BLE - start advertising");
            oldDeviceConnected = deviceConnected;
        }
        // connecting
        if (deviceConnected && !oldDeviceConnected)
        {
            Serial.println("BLE - new device deviceConnected");
            // do stuff here on connecting
            oldDeviceConnected = deviceConnected;
        }
    }
};
#endif