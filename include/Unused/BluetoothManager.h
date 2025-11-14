
#ifndef BluetoothManager_H
#define BluetoothManager_H

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

enum CommandType
{
    relayChange = 0,
    genericCommand = 1
};

class BluetoothManager
{
private:
public:
    void SetupBT()
    {
        SerialBT.begin("ESP32test"); // Bluetooth device name
        Serial.println("The device started, now you can pair it with bluetooth!");
    }

    String BT_ReadCommand()
    {
        auto startMS = millis();
        if (SerialBT.available())
        {

            // Serial.print("--111-- ");
            // Serial.println(millis() - startMS);

            char buf[10] = "";
            SerialBT.readBytes(buf, 4);
            // Serial.print("--222-- ");
            // Serial.println(millis() - startMS);

            return buf;
        }
    }

    void BT_doWork()
    {
        if (Serial.available())
        {
            SerialBT.write(Serial.read());
        }
        if (SerialBT.available())
        {
            Serial.write(SerialBT.read());
        }
        delay(100);
    }
};

#endif