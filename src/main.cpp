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
#include "AsyncWebServerManager.h"
#include "PositionManager.h"

BoardPowerManager *boardPowerManager;
WifiManager *wifiManager;
SensorManager *sensorManager;
ConfigManager *configManager;
TimeManager *timeManager;
PositionManager *positioningManager;
AsyncWebServerManager *asyncWebServerManager;
// RelayManager *relayManager;
// ButtonLEDManager *buttonLEDManager;
// BtnLEDManager *btnLEDManager;
// IOManager *ioManager;

ulong lastProgramTimestamp;
Config config;
Ticker myTicker;

void tick();

void setup()
{
    Serial.begin(115200);

    boardPowerManager = new BoardPowerManager();
    boardPowerManager->InitBoardPowerManager();

    configManager = new ConfigManager();
    config = configManager->readConfig();

    wifiManager = new WifiManager();
    wifiManager->WifiConnect(config);

    sensorManager = new SensorManager();
    sensorManager->SetupSensors();

    timeManager = new TimeManager();
    timeManager->initTime(config);

    positioningManager = new PositionManager(config, sensorManager);

    asyncWebServerManager = new AsyncWebServerManager();
    asyncWebServerManager->initWebServer();

    myTicker.attach(1.0, tick);
    // config.POT1_pin_MaxAngl = 35;
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
    auto positioningInterval = 
        positioningManager->GetPositioningMode() == PositionMode::Manual ||
        positioningManager->GetPositioningMode() == PositionMode::LowLight
        ? 5 * config.positioningUpdateIntervalMs
        : config.positioningUpdateIntervalMs;
    if (millis() - previousPositioningMillis >= positioningInterval)
    {
        previousPositioningMillis = millis();

        Serial.println();
        timeManager->printCurrentTime();
        positioningManager->UpdatePositioning();
    }

    if (millis() - previousButtonMillis >= 50)
    {
        previousButtonMillis = millis();
        positioningManager->MonitorBtnStates();
        positioningManager->UpdateLEDStates();
        asyncWebServerManager->loopWebServer();
    }
}
