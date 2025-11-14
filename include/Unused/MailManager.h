
#ifndef MailManager_H
#define MailManager_H

#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#endif

#include <ESP_Mail_Client.h>

class MailManager
{

public:
};
#endif