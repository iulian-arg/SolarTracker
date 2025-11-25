#ifndef BtnLEDManager_H
#define BtnLEDManager_H

// #include <Arduino.h>
// #include "ConfigManager.h"
#include "PositionManager.h"

// #include <vector>

enum BtnState
{
    _notPressed,
    _pressed,
};

enum BtnCommand
{
    _none,
    _moveNorth,
    _moveSouth,
    _manualMode,
    _automaticMode,
};

class BtnLEDManager
{
private:
    Config config;
    PositionManager *posManager;
    uint8_t previousBtnPressed = 0;

public:
    // Default constructor
    BtnLEDManager();

    // Constructor to initialize relayManager
    BtnLEDManager(Config _config, PositionManager *_positioningManager)
    {
        config = _config;
        posManager = _positioningManager;
        pinMode(config.B1_pin_Auto, INPUT);
        pinMode(config.B2_pin_MoveNorth, INPUT);
        pinMode(config.B3_pin_MoveSouth, INPUT);
        // pinMode(config.LED1_pin_Auto, OUTPUT);
        // pinMode(config.LED2_pin_Manual, OUTPUT);
        // digitalWrite(config.LED1_pin_Auto, LOW);
        // digitalWrite(config.LED2_pin_Manual, LOW);
    }
    /**/
    void MonitorBtnStates()
    {
        BtnState b1_pin_Auto_state = digitalRead(config.B1_pin_Auto) == HIGH ? _pressed : _notPressed;
        BtnState B2_pin_MoveNorth_state = digitalRead(config.B2_pin_MoveNorth) == HIGH ? _pressed : _notPressed;
        BtnState B3_pin_MoveSouth_state = digitalRead(config.B3_pin_MoveSouth) == HIGH ? _pressed : _notPressed;

        if (B2_pin_MoveNorth_state == _pressed &&
            previousBtnPressed != config.B2_pin_MoveNorth)
        {
            previousBtnPressed = config.B2_pin_MoveNorth;
            Serial.println("Move North Btn _pressed");
            // delay(50); // Debounce delay
            posManager->SetPositioningMode(PositionMode::Manual);
            posManager->TryMoveNorth();
        }
        else if (B3_pin_MoveSouth_state == _pressed &&
                 previousBtnPressed != config.B3_pin_MoveSouth)
        {
            previousBtnPressed = config.B3_pin_MoveSouth;
            Serial.println("Move South Btn _pressed");
            // delay(50); // Debounce delay
            posManager->SetPositioningMode(PositionMode::Manual);
            posManager->TryMoveSouth();
        }
        else if (b1_pin_Auto_state == _pressed &&
                 previousBtnPressed != config.B1_pin_Auto)
        {
            previousBtnPressed = config.B1_pin_Auto;
            Serial.println("Automatic Mode Btn _pressed");
            // delay(50); // Debounce delay
            if (posManager->GetPositioningMode() == PositionMode::Manual)
            {
                posManager->SetPositioningMode(PositionMode::Automatic);
            }
        }
        else if (previousBtnPressed != 0 &&
                 b1_pin_Auto_state == _notPressed &&
                 B2_pin_MoveNorth_state == _notPressed &&
                 B3_pin_MoveSouth_state == _notPressed)
        {
            Serial.println("Btn Released, Resetting Movement");
            previousBtnPressed = 0;
            posManager->ResetMoving();
        }
    }

    void MonitorMaxPositions()
    {
        if (digitalRead(config.B4_pin_MaxLeft) == HIGH)
        {
            Serial.println("Max Left Position Reached");
            posManager->ResetMoving();
        }
        if (digitalRead(config.B5_pin_MaxRight) == HIGH)
        {
            Serial.println("Max Right Position Reached");
            posManager->ResetMoving();
        }
    }

    bool IsMaxLeft()
    {
        return digitalRead(config.B4_pin_MaxLeft) == HIGH;
    }
    bool IsMaxRight()
    {
        return digitalRead(config.B5_pin_MaxRight) == HIGH;
    }

    void UpdateLEDStates()
    {
        if (millis() % config.ledBlinkIntervalMs < config.ledBlinkDurationMs)
        {
            if (posManager->GetPositioningMode() == PositionMode::Automatic)
            {
                digitalWrite(config.LED1_pin_Auto, HIGH);
                digitalWrite(config.LED2_pin_Manual, LOW);
            }
            else if (posManager->GetPositioningMode() == PositionMode::Manual)
            {
                digitalWrite(config.LED1_pin_Auto, LOW);
                digitalWrite(config.LED2_pin_Manual, HIGH);
            }
        }
        else
        {
            digitalWrite(config.LED1_pin_Auto, LOW);
            digitalWrite(config.LED2_pin_Manual, LOW);
        }
    }
    /* */
};

#endif