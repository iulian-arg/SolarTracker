
#ifndef BluetoothManager_H
#define BluetoothManager_H

#include "BluetoothSerial.h"
#include "PositionManager.h"
#include "WifiManager.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;
extern PositionManager *positionManager;
extern WifiManager *wifiManager;

class BluetoothManager
{
private:
public:
    void SetupBT()
    {
        SerialBT.begin("SolarTrackerBT"); // Bluetooth device name
        Serial.println("The device started, now you can pair it with bluetooth!");
    }

    String BT_ReadLine()
    {
        String line = "";
        char c;
        auto startMS = millis();
        while (millis() - startMS < 1000)
        {
            if (SerialBT.available())
            {
                c = SerialBT.read();
                if (c == '\n')
                {
                    break;
                }
                line += c;
            }
        }
        return line;
    }

    void BT_WriteLine(const String &line)
    {
        if (SerialBT.connected() == false)
        {
            return;
        }
        SerialBT.println(line);
    }

    void BT_doWork()
    {
        auto command = BT_ReadLine();
        command.trim();
        if (command.length() > 0)
        {
            Serial.printf("BT Command Received: _%s_\n", command.c_str());

            if (command == String("NORTH"))
            {
                BT_WriteLine("Moving North");
                positionManager->SetPositioningMode(PositionMode::Manual);
                positionManager->AddMoveEventQueue(MoveDirection::MoveNorth, 30);
            }
            else if (command == String("SOUTH"))
            {
                BT_WriteLine("Moving South");
                positionManager->SetPositioningMode(PositionMode::Manual);
                positionManager->AddMoveEventQueue(MoveDirection::MoveSouth, 31);
            }
            else if (command == String("AUTO"))
            {
                BT_WriteLine("Setting to Auto Mode");
                positionManager->SetPositioningMode(PositionMode::Automatic);
                positionManager->AddMoveEventQueue(MoveDirection::NoMove, 32);
            }
            else if (command == String("MANUAL"))
            {
                BT_WriteLine("Setting to Manual Mode");
                positionManager->SetPositioningMode(PositionMode::Manual);
                positionManager->AddMoveEventQueue(MoveDirection::NoMove, 33);
            }
            else if (command == String("RESET"))
            {
                BT_WriteLine("Resetting Position");
                positionManager->AddMoveEventQueue(MoveDirection::NoMove, 34);
            }
            else if (command == String("maxN"))
            {
                BT_WriteLine("Setting Max North Position");
                positionManager->SetPositioningMode(PositionMode::Manual);
                positionManager->AddMoveEventQueue(MoveDirection::MaxNorth, 35);
            }
            else if (command == String("maxS"))
            {
                BT_WriteLine("Setting Max South Position");
                positionManager->SetPositioningMode(PositionMode::Manual);
                positionManager->AddMoveEventQueue(MoveDirection::MaxSouth, 36);
            }
            else if (command == String("RESTART"))
            {
                BT_WriteLine("Restarting SolarTracker...");
                ESP.restart();
            }
            else if (command == String("WF_STATUS"))
            {
                String wifiStatus = wifiManager->GetWifiIpAndSSID();
                BT_WriteLine(wifiStatus);
            }            
            else if (command == String("WF_RECONNECT"))
            {
                BT_WriteLine("Reconnecting to WiFi...");
                wifiManager->WifiConnect();

            }          
            else if (command == String("WF_DISCONNECT"))
            {
                BT_WriteLine("Disconnecting from WiFi...");
                wifiManager->WifiDisconnect();

            }            
            else if (command == String("UPDATE"))
            {
                BT_WriteLine("Updating Positioning...");
                positionManager->UpdatePositioning();
            }
            else
            {
                BT_WriteLine("Unknown Command");
            }
        }
    };
};

#endif