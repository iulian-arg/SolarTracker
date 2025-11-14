
#ifndef HttpServerManager_H
#define HttpServerManager_H

#include <ESP8266WiFi.h>

#include <ConfigManager.h>
#include <RelayManager.h>
#include <eagle_soc.h>

WiFiServer server(80);
ConfigManager *_configManager;
TimeManager *_timeManager;
RelayManager *_relayManager;

class HttpServerManager
{

public:
    HttpServerManager(TimeManager timeManager, RelayManager relayManager,ConfigManager configManager)
    {
        _timeManager = &timeManager;
        _relayManager = &relayManager;
        _configManager = &configManager;
        startServer();
    }
    void startServer()
    {
        
        // WiFi.hostname("fosa");
        if (!MDNS.begin("fosa"))
        {
            Serial.println("Error setting up MDNS responder!");
            while (1)
            {
                delay(1000);
            }
        }
        // Start the server
        server.begin();

        MDNS.addService("http", "tcp", 80);

        Serial.println("Server started");

        // Print the IP address
        Serial.print("Use this URL to connect: ");
        Serial.print(WiFi.localIP());
        Serial.println("/");
    }
    void handleHttpRequests()
    {

        // Check if a client has connected
        WiFiClient client = server.accept();
        if (!client)
        {
            return;
        }

        // Wait until the client sends some data
        Serial.println("new client");
        while (!client.available())
        {
            _timeManager->pauseAndUpdateTime();
        }

        // Read the first line of the request
        String request = client.readStringUntil('\r');
        Serial.println(request);
        client.flush();

        // Match the request
        if (request.indexOf("/OFF") != -1)
        {
            Serial.println("TURN OFF");

            _relayManager->resetRelayToDefault(_configManager->config);
        }
        if (request.indexOf("/PLUS5") != -1)
        {
            Serial.println("PLUS5");

            _configManager->ChangeRelayDefaultRun(5*60);
        }
        if (request.indexOf("/MINUS5") != -1)
        {
            Serial.println("MINUS5");
            Serial.printf("from %d secons", _configManager->config.relayDefaultRunTime);
            _configManager->ChangeRelayDefaultRun(-5*60);
            Serial.printf("to %d secons", _configManager->config.relayDefaultRunTime);
        }

        if (request.indexOf("/ON=") != -1)
        {
            String req = request;
            int index = request.indexOf("=");
            req.remove(0, index + 1);
            index = request.indexOf("HTTP");
            req.remove(index - 1);
            int seconds = req.toInt();
            Serial.printf("TURN ON for _%d_ seconds\n", seconds);

            _relayManager->triggerRelayManually(seconds);
        }

        // Return the response
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println(""); //  this is a must
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");
        client.println("<head><title>Fosa RELAY Control</title></head>");
        client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
        client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
        client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
        client.println(".txton {background-color: green;}");
        client.println(".txtoff {background-color: gray;}");
        client.println(".button2 {background-color: #77878A;}</style></head>");

        client.print("CURRENT STATUS ");

        if (digitalRead(RELAY) == HIGH)
        {
            client.print("<span class=\"txtoff\">Relay is now: OFF</span>");
        }
        else
        {
            client.print("<span class=\"txton\">Relay is now: ON</span>");
        }
        client.println("<br><br>");

        Time now = _timeManager->GetTimeFromSeconds(currentTime.currentTimeOfDay);
        Time startTime = _timeManager->GetTimeFromSeconds(relayState.startTime);
        Time stopTime = _timeManager->GetTimeFromSeconds(relayState.stopTime);
        client.printf("Current time: %d:%d:%d<br>", now.hours, now.minutes, now.seconds);

        client.printf("startTime %d:%d:%d; ", startTime.hours, startTime.minutes, startTime.seconds);
        client.printf("stopTime %d:%d:%d; ", stopTime.hours, stopTime.minutes, stopTime.seconds);
        client.printf("Current trigger mode: %s <br>", relayState.triggerManual ? "MANUAL" : "AUTO");

        client.printf("Current Settings: sleepSeconds=%d, relayDefaultRunTime=%d <br>", _configManager->config.sleepSeconds, _configManager->config.relayDefaultRunTime);

        client.println("<br><br>");
        client.println("<a href=\"/\"><button class=\"button\">REFRESH</button></a> <br>");

        client.println("<br><br>");

        client.println("<a href=\"/ON=60\"><button class=\"button\">ON 60s</button></a> <br>");
        client.println("<br>");
        client.println("<a href=\"/ON=300\"><button class=\"button\">ON 300s</button></a> <br>");
        client.println("<br>");
        client.println("<a href=\"/ON=600\"><button class=\"button\">ON 600s</button></a> <br>");
        client.println("<br>");
        client.println("<a href=\"/OFF\"><button class=\"button\">OFF</button></a> <br>");
        client.println("<br><br>");
        client.println("<a href=\"/PLUS5\"><button class=\"button\">+ 5 min</button></a>");
        client.println("<a href=\"/MINUS5\"><button class=\"button\">- 5 min</button></a> <br>");
        client.println("<br><br>");

        client.println("</html>");

        Serial.println("Client disonnected");
        Serial.println("");
    }
};
#endif