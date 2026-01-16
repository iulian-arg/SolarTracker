
#ifndef AsyncWebServerManager_H
#define AsyncWebServerManager_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LITTLEFS.h"
#include "FS.h"
#include "PositionManager.h"
#include "Logger.h"

extern const char *TAG;

extern ConfigManager *configManager;

extern PositionManager *positionManager;
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

String message = "";

// Initialize LITTLEFS
void initFS()
{
    if (!LITTLEFS.begin())
    {
        Logger::error(TAG, "An error has occurred while mounting LITTLEFS");
    }
}

void notifyClients(String notificationMessage)
{
    ws.textAll(notificationMessage);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0;
        message = (char *)data;
        auto sliderValue = message.substring(2);
        auto dutyCycle = map(sliderValue.toInt(), 0, 100, 0, 255);
        Logger::info(TAG, "WEB Socket message: %s", message.c_str());
        if (message.indexOf("MOVE_NORTH_down") >= 0 || message.indexOf("MOVE_SOUTH_down") >= 0)
        {
            positionManager->SetPositioningMode(PositionMode::Manual);
            if (message.indexOf("MOVE_NORTH_down") >= 0)
            {
                positionManager->AddMoveEventQueue(MoveDirection::MoveNorth,20);
            }
            if (message.indexOf("MOVE_SOUTH_down") >= 0)
            {
                positionManager->AddMoveEventQueue(MoveDirection::MoveSouth,21);
            }
        }
        else if (message.indexOf("MOVE_MAX_NORTH_down") >= 0 || message.indexOf("MOVE_MAX_SOUTH_down") >= 0)
        {
            positionManager->SetPositioningMode(PositionMode::Manual);
            if (message.indexOf("MOVE_MAX_NORTH_down") >= 0)
            {
                positionManager->AddMoveEventQueue(MoveDirection::MaxNorth,22);
            }
            if (message.indexOf("MOVE_MAX_SOUTH_down") >= 0)
            {
                positionManager->AddMoveEventQueue(MoveDirection::MaxSouth,23);
            }
        }
        else if (message.indexOf("MOVE_NORTH_up") >= 0 || message.indexOf("MOVE_SOUTH_up") >= 0)
        {
            positionManager->AddMoveEventQueue(MoveDirection::NoMove,24);
        }
        else if (message.indexOf("AUTO_MODE") >= 0 || message.indexOf("MANUAL_MODE") >= 0)
        {
            positionManager->AddMoveEventQueue(MoveDirection::NoMove,25);
            positionManager->SetPositioningMode(message.indexOf("AUTO_MODE") >= 0 ? PositionMode::Automatic : PositionMode::Manual);
        }
        else if (message.indexOf("RESET") >= 0)
        {
            positionManager->AddMoveEventQueue(MoveDirection::NoMove,26);
        }
        else if (message.indexOf("RESTART") >= 0)
        {
            Logger::info(TAG, "Restarting device as per Web request");
            notifyClients("RESTARTING");
            delay(1000);
            ESP.restart();
        }
        else if (message.indexOf("GET_config") >= 0)
        {
            Logger::info(TAG, "Sent config Config to clients");
            configManager->UpdateJSONFromLITTLEFS();
            notifyClients(String("config_CONFIG:" + String(configConfig)));
            return;
        }
        else if (message.indexOf("SET_config") >= 0)
        {
            Serial.println();
            String configString = message.substring(11); // Extract config part
            Logger::info(TAG, "Received config Config: %s", configString.c_str());
            configConfig = configString.c_str();
            configManager->WriteToLITTLEFS(configConfig);
            configManager->readConfig();
            Logger::info(TAG, "Updated config Config");
            notifyClients("config_UPDATED");
            return;
        }
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        Logger::info(TAG, "WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        Logger::info(TAG, "WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

void initWebSocket()
{
    ws.onEvent(onEvent);
    server.addHandler(&ws);

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LITTLEFS, "/index.html", "text/html"); });

    server.serveStatic("/", LITTLEFS, "/");

    // Start server
    server.begin();
}

extern PositionManager *posManager;
class AsyncWebServerManager
{
private:
public:
    void initWebServer()
    {
        initFS();
        initWebSocket();
    }

    void loopWebServer()
    {
        ws.cleanupClients();
    }
    void notifyClients(String notificationMessage)
    {
        ws.textAll(notificationMessage);
    }
};

#endif