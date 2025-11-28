
#ifndef AsyncWebServerManager_H
#define AsyncWebServerManager_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
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

// Json Variable to Hold Slider Values
JSONVar sliderValues;


// Initialize SPIFFS
void initFS()
{
    if (!SPIFFS.begin())
    {
        Logger::error(TAG, "An error has occurred while mounting SPIFFS");
    }
    else
    {
        Logger::info(TAG, "SPIFFS mounted successfully");
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
        if (message.indexOf("MOVE_NORTH_down") >= 0)
        {
            positionManager->SetPositioningMode(PositionMode::Manual);
            positionManager->TryMoveNorth();
        }
        else if (message.indexOf("MOVE_NORTH_up") >= 0)
        {
            positionManager->ResetMoving();
        }
        else if (message.indexOf("MOVE_SOUTH_down") >= 0)
        {
            positionManager->SetPositioningMode(PositionMode::Manual);
            positionManager->TryMoveSouth();
        }
        else if (message.indexOf("MOVE_SOUTH_up") >= 0)
        {
            positionManager->ResetMoving();
        }
        else if (message.indexOf("AUTO_MODE") >= 0)
        {
            positionManager->ResetMoving();
            positionManager->SetPositioningMode(PositionMode::Automatic);
        }
        else if (message.indexOf("MANUAL_MODE") >= 0)
        {
            positionManager->ResetMoving();
            positionManager->SetPositioningMode(PositionMode::Manual);
        }
        else if (message.indexOf("RESET") >= 0)
        {
            positionManager->ResetMoving();
        }
        else if (message.indexOf("RESTART") >= 0)
        {
            Logger::info(TAG, "Restarting device as per Web request");
            notifyClients("RESTARTING");
            delay(1000);
            ESP.restart();
        }
        else if (message.indexOf("GET_JSON") >= 0)
        {
            Logger::info(TAG, "Sent JSON Config to clients");
            Serial.printf("\njsonConfig=%s\n", jsonConfig);
            notifyClients(String("JSON_CONFIG:" + String(jsonConfig)));
            return;
        }
        else if (message.indexOf("SET_JSON") >= 0)
        {
            Serial.println();
            Serial.println();
            Serial.println();
            String jsonString = message.substring(9); // Extract JSON part
            Logger::info(TAG, "Received JSON Config: %s", jsonString.c_str());
            jsonConfig = jsonString.c_str();
            configManager->WriteToSPIFFS(jsonConfig);
            // configManager->UpdateJSONFromSPIFFS();
            configManager->readConfig();
            Logger::info(TAG, "Updated JSON Config");
            notifyClients("JSON_UPDATED");
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
              { request->send(SPIFFS, "/index.html", "text/html"); });

    server.serveStatic("/", SPIFFS, "/");

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