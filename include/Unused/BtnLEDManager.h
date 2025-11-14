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
    _moveRight,
    _moveLeft,
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
        pinMode(config.B2_pin_MoveRight, INPUT);
        pinMode(config.B3_pin_MoveLeft, INPUT);
        // pinMode(config.LED1_pin_Auto, OUTPUT);
        // pinMode(config.LED2_pin_Manual, OUTPUT);
        // digitalWrite(config.LED1_pin_Auto, LOW);
        // digitalWrite(config.LED2_pin_Manual, LOW);
    }
    /**/
    void MonitorBtnStates()
    {
        BtnState b1_pin_Auto_state = digitalRead(config.B1_pin_Auto) == HIGH ? _pressed : _notPressed;
        BtnState b2_pin_MoveRight_state = digitalRead(config.B2_pin_MoveRight) == HIGH ? _pressed : _notPressed;
        BtnState b3_pin_MoveLeft_state = digitalRead(config.B3_pin_MoveLeft) == HIGH ? _pressed : _notPressed;

        if (b2_pin_MoveRight_state == _pressed &&
            previousBtnPressed != config.B2_pin_MoveRight)
        {
            previousBtnPressed = config.B2_pin_MoveRight;
            Serial.println("Move Right Btn _pressed");
            // delay(50); // Debounce delay
            posManager->SetPositioningMode(PositionMode::Manual);
            posManager->TriggerMoveRight();
        }
        else if (b3_pin_MoveLeft_state == _pressed &&
                 previousBtnPressed != config.B3_pin_MoveLeft)
        {
            previousBtnPressed = config.B3_pin_MoveLeft;
            Serial.println("Move Left Btn _pressed");
            // delay(50); // Debounce delay
            posManager->SetPositioningMode(PositionMode::Manual);
            posManager->TriggerMoveLeft();
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
                 b2_pin_MoveRight_state == _notPressed &&
                 b3_pin_MoveLeft_state == _notPressed)
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