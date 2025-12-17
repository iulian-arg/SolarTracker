
#ifndef WebServerManager_H
#define WebServerManager_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

String message = "";
String sliderValue1 = "0";
String sliderValue2 = "0";
String sliderValue3 = "0";
String sliderValue4 = "0";
String sliderValue5 = "0";

// Json Variable to Hold Slider Values
JSONVar sliderValues;

// Get Slider Values
String getSliderValues()
{
    sliderValues["sliderValue1"] = String(sliderValue1);
    sliderValues["sliderValue2"] = String(sliderValue2);
    sliderValues["sliderValue3"] = String(sliderValue3);
    sliderValues["sliderValue4"] = String(sliderValue4);
    sliderValues["sliderValue5"] = String(sliderValue5);
    Serial.println(sliderValues.length());

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
        Serial.println(dutyCycle);
        if (message.indexOf("0s") >= 0)
        {
            sliderValue1 = sliderValue2 = sliderValue3 = sliderValue4 = sliderValue5 = sliderValue;
            dutyCycle1 = dutyCycle2 = dutyCycle3 = dutyCycle4 = dutyCycle5 = dutyCycle;
        }
        if (message.indexOf("1s") >= 0)
        {
            sliderValue1 = sliderValue;
            dutyCycle1 = dutyCycle;
        }
        if (message.indexOf("2s") >= 0)
        {
            sliderValue2 = sliderValue;
            dutyCycle2 = dutyCycle;
        }
        if (message.indexOf("3s") >= 0)
        {
            sliderValue3 = sliderValue;
            dutyCycle3 = dutyCycle;
        }
        if (message.indexOf("4s") >= 0)
        {
            sliderValue4 = sliderValue;
            dutyCycle4 = dutyCycle;
        }
        if (message.indexOf("5s") >= 0)
        {
            sliderValue5 = sliderValue;
            dutyCycle5 = dutyCycle;
        }
        Serial.print(getSliderValues());
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
class WebServerManager
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