#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

// #include <Arduino.h>
#include <WiFi.h>
#include <Ticker.h>

#include "BoardPowerManager.h"
#include "SensorManager.h"
#include "ConfigManager.h"
#include "WifiManager.h"
#include "TimeManager.h"
#include "AsyncWebServerManager.h"
#include "PositionManager.h"
#include "Logger.h"
#include "BluetoothManager.h"

BoardPowerManager *boardPowerManager;
BluetoothManager *bluetoothManager;
WifiManager *wifiManager;
SensorManager *sensorManager;
ConfigManager *configManager;
TimeManager *timeManager;
PositionManager *positionManager;
AsyncWebServerManager *asyncWebServerManager;

TaskHandle_t Task1;

const char *TAG = "ST";
ulong lastProgramTimestamp;
Config config;
Ticker myTicker;

void tick();

void setup()
{
    Serial.begin(115200);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    boardPowerManager = new BoardPowerManager();
    boardPowerManager->InitBoardPowerManager();

    configManager = new ConfigManager();
    config = configManager->readConfig();

    timeManager = new TimeManager();
    // timeManager->initTime(); // moved to WifiManager after successful WiFi connection
   
    wifiManager = new WifiManager();
    wifiManager->WifiConnect();
    // wifiManager->WifiSetupCore_0();

    bluetoothManager = new BluetoothManager();
    bluetoothManager->SetupBT();

    sensorManager = new SensorManager();
    sensorManager->SetupSensors();

    positionManager = new PositionManager();

    asyncWebServerManager = new AsyncWebServerManager();
    asyncWebServerManager->initWebServer();

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
    auto positioningInterval =
        positionManager->GetPositioningMode() == PositionMode::Manual ||
                positionManager->GetPositioningMode() == PositionMode::LowLight
            ? config.positioningUpdateIntervalMsLowLight
            : config.positioningUpdateIntervalMs;
    if (millis() - previousPositioningMillis >= positioningInterval)
    {
        previousPositioningMillis = millis();

        Serial.println();
        // timeManager->printCurrentTime();
        positionManager->UpdatePositioning();
    }

    if (millis() - previousButtonMillis >= 250)
    {
        positionManager->RefreshPositioning ();
        previousButtonMillis = millis();
        positionManager->MonitorBtnStates();
        positionManager->UpdateLEDStates();
        asyncWebServerManager->loopWebServer();
        // bleManager->loopBLE();
        bluetoothManager->BT_doWork();
    }
    auto logs = Logger::getLogs();
    for (auto &entry : logs)
    {
        asyncWebServerManager->notifyClients("LOG_ENTRY:" + entry);
        bluetoothManager->BT_WriteLine(entry);
        // Serial.println(entry);
    }
    Logger::clearLogs();
}
