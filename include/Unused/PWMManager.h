
#ifndef PWMManager_H
#define PWMManager_H

#include <Arduino.h>
// #include <AnalogWrite.h>

// const int _PIN_PeltierModule = 15;
// const int _PIN_LPGPump = 12;
// const int _PIN_WaterPump = 14;
// const int _PIN_WaterPump2 = 27;

// Set LED GPIO
const uint8_t _R0_LPGPump0_Relay = 13;
const uint8_t _R1_Tank_Valve = 12;
const uint8_t _R2_Enginebay_Valve = 14;
const uint8_t _R3_Return_Valve = 27;
const uint8_t _R4_RELAY = 26;

uint8_t dutyCycle1;
uint8_t dutyCycle2;
uint8_t dutyCycle3;
uint8_t dutyCycle4;
uint8_t dutyCycle5;

// setting PWM properties
const uint8_t freq = 5000;
const uint8_t ledChannel1 = 0;
const uint8_t ledChannel2 = 1;
const uint8_t ledChannel3 = 2;
const uint8_t ledChannel4 = 3;
const uint8_t ledChannel5 = 4;

const uint8_t resolution = 8;

class PWMManager
{
private:
public:
    void initPWM()
    {
        pinMode(_R0_LPGPump0_Relay, OUTPUT);
        pinMode(_R1_Tank_Valve, OUTPUT);
        pinMode(_R2_Enginebay_Valve, OUTPUT);
        pinMode(_R3_Return_Valve, OUTPUT);
        pinMode(_R4_RELAY, OUTPUT);

        // configure LED PWM functionalitites
        ledcSetup(ledChannel1, freq, resolution);
        ledcSetup(ledChannel2, freq, resolution);
        ledcSetup(ledChannel3, freq, resolution);
        ledcSetup(ledChannel4, freq, resolution);
        ledcSetup(ledChannel5, freq, resolution);

        // attach the channel to the GPIO to be controlled
        ledcAttachPin(_R0_LPGPump0_Relay, ledChannel1);
        ledcAttachPin(_R1_Tank_Valve, ledChannel2);
        ledcAttachPin(_R3_Return_Valve, ledChannel3);
        ledcAttachPin(_R2_Enginebay_Valve, ledChannel4);
        ledcAttachPin(_R4_RELAY, ledChannel5);
    }

    void loopPWM()
    {

        ledcWrite(ledChannel1, dutyCycle1);
        ledcWrite(ledChannel2, dutyCycle2);
        ledcWrite(ledChannel3, dutyCycle3);
        ledcWrite(ledChannel4, dutyCycle4);
        ledcWrite(ledChannel5, dutyCycle5);
        // if (millis() % 200 == 0)
        // {
        //     Serial.print("dutyCycle4= ");
        //     Serial.print(dutyCycle4);
        // }
    }

    ulong prevMilis = 0;
    uint8_t dutyCycle = 0;
    bool ascending = true;
    u_short dclimit = 255;
    void testPWM()
    {
        ulong currentMs = millis();
        // Serial.println(currentMs);

        if (currentMs > prevMilis + 100)
        {
            prevMilis = currentMs;
            // auto dutyCycle = (prevMilis / 100) % 100;
            // dutyCycle %= 256;

            ledcWrite(ledChannel1, dutyCycle);
            ledcWrite(ledChannel2, dutyCycle);
            ledcWrite(ledChannel3, dutyCycle);
            ledcWrite(ledChannel4, dutyCycle);
            ledcWrite(ledChannel5, dutyCycle);
            // analogWrite(ledChannel1, dutyCycle);
            // analogWrite(ledChannel2, dutyCycle);
            // analogWrite(ledChannel3, dutyCycle);
            // analogWrite(ledChannel4, dutyCycle);
            Serial.print("dutyCycle= ");
            Serial.print(dutyCycle);
            Serial.print(" currentMs= ");
            Serial.println(currentMs);

            if (dutyCycle == dclimit)
            {
                delay(2000);
                ascending = false;
            }
            if (dutyCycle == 0)
            {
                ascending = true;
            }

            if (ascending)
                dutyCycle++;
            else
                dutyCycle--;
        }
    }
};

#endif