
#ifndef AsyncWebServerManager_H
#define AsyncWebServerManager_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include "PositionManager.h"

extern PositionManager *positionManager;
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

String message = "";

// Json Variable to Hold Slider Values
JSONVar sliderValues;

// Get Slider Values
String getSliderValues()
{
    sliderValues["sliderValue1"] = String(0);
    // Serial.println(sliderValues.length());

    String jsonString = JSON.stringify(sliderValues);
    return jsonString;
}

// Initialize SPIFFS
void initFS()
{
    if (!SPIFFS.begin())
    {
        Serial.println("An error has occurred while mounting SPIFFS");
    }
    else
    {
        Serial.println("SPIFFS mounted successfully");
    }
}

void notifyClients(String sliderValues)
{
    ws.textAll(sliderValues);
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
        Serial.print("message: ");
        Serial.println(message.c_str());
        if (message.indexOf("MOVE_RIGHT_down") >= 0)
        {
            positionManager->SetPositioningMode(PositionMode::Manual);
            positionManager->TryMoveRight();
        }
        else if (message.indexOf("MOVE_RIGHT_up") >= 0)
        {
            positionManager->ResetMoving();
        }
        else if (message.indexOf("MOVE_LEFT_down") >= 0)
        {
            positionManager->SetPositioningMode(PositionMode::Manual);
            positionManager->TryMoveLeft();
        }
        else if (message.indexOf("MOVE_LEFT_up") >= 0)
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

        notifyClients(getSliderValues());

        if (strcmp((char *)data, "getValues") == 0)
        {
            notifyClients(getSliderValues());
        }
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
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
};

#endif