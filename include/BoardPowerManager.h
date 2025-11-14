#ifndef BoardPowerManager_H
#define BoardPowerManager_H

// #include <Arduino.h>

class BoardPowerManager
{
private:
    void DisplayFrequency()
    {
        pinMode(5, OUTPUT);
        uint32_t Freq = getCpuFrequencyMhz();
        Serial.println("---FREQ---");
        Serial.print("CPU Freq = ");
        Serial.print(Freq);
        Serial.println(" MHz");
        Freq = getXtalFrequencyMhz();
        Serial.print("XTAL Freq = ");
        Serial.print(Freq);
        Serial.println(" MHz");
        Freq = getApbFrequency();
        Serial.print("APB Freq = ");
        Serial.print(Freq);
        Serial.println(" Hz");
        Serial.println("---FREQ---");
    }

public:
    BoardPowerManager()
    {
        // Constructor code here
    }

    void InitBoardPowerManager()
    {
        // DisplayFrequency();
        setCpuFrequencyMhz(80);
        //DisplayFrequency();
    }

};

#endif
