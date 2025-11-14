// #include <SensorManager.h>
// #include <BluetoothManager.h>
// #include <PWMManager.h>
// #include <analogWrite.h> // This is a library, not a header file
// #include <WifiManager.h>
// #include <WebServerManager.h>
// #include <ButtonLEDManager.h>
// #include <SPIFFS.h>
// #include "NimBLEManager.h"
// #include <ProgramManager.h>

#include <Arduino.h>
#include <WiFi.h>
#include <Ticker.h>

#include "BoardPowerManager.h"
#include "SensorManager.h"
#include "ConfigManager.h"
#include "WifiManager.h"
#include "TimeManager.h"
#include "PositionManager.h"
// #include "ButtonLEDManager.h"
// #include "BtnLEDManager.h"
// #include "RelayManager.h"
#include "IOManager.h"

BoardPowerManager *boardPowerManager;
WifiManager *wifiManager;
SensorManager *sensorManager;
ConfigManager *configManager;
TimeManager *timeManager;
PositionManager *positioningManager;
// RelayManager *relayManager;
// ButtonLEDManager *buttonLEDManager;
// BtnLEDManager *btnLEDManager;
IOManager *ioManager;

ulong lastProgramTimestamp;
Config config;
Ticker myTicker;

void tick();

void setup()
{
    Serial.begin(115200);

    boardPowerManager = new BoardPowerManager();
    boardPowerManager->InitBoardPowerManager();

    // bleManager = new NimBLEManager();
    // bleManager->setupNimBLE(currentProgram);
    // lastProgramTimestamp = millis();

    configManager = new ConfigManager();
    config = configManager->readConfig();

    wifiManager = new WifiManager();
    wifiManager->WifiConnect(config);

    sensorManager = new SensorManager();
    sensorManager->SetupSensors();

    timeManager = new TimeManager();
    timeManager->initTime(config);

    // relayManager = new RelayManager(config);
    // relayManager->ResetRelays();
    ioManager = new IOManager(config);
    ioManager->ResetRelays();

    positioningManager = new PositionManager(config, sensorManager, ioManager);
    ioManager->SetPositionManager(positioningManager);

    // buttonLEDManager = new ButtonLEDManager(config, positioningManager);
    // relayManager->SetButtonLEDManager(buttonLEDManager);

    myTicker.attach(1.0, tick);
}
unsigned long previousPositioningMillis = 0;
unsigned long previousButtonMillis = 0;

void tick()
{
    // Serial.println();
    // timeManager->printCurrentTime();
    // positioningManager->UpdatePositioning();
}
void loop()
{
    auto positioningInterval = positioningManager->GetPositioningMode() == PositionMode::Manual
                                   ? 10000
                                   : config.positioningUpdateIntervalMs;
    if (millis() - previousPositioningMillis >= positioningInterval)
    {
        previousPositioningMillis = millis();

        Serial.println();
        timeManager->printCurrentTime();
        positioningManager->UpdatePositioning();
    }

    // if (millis() - previousButtonMillis >= 50)
    // {
    //     previousButtonMillis = millis();
    //     buttonLEDManager->MonitorButtonStates();
    //     buttonLEDManager->UpdateLEDStates();
    // }
}
