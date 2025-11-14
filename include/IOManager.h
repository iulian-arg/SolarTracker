
#ifndef IOManager_H
#define IOManager_H

// #include <Arduino.h>
// #include <time.h>
#include "ConfigManager.h"
#include "PositionManager.h"
// #include "BtnLEDManager.h"
// #include <vector>
// #include "PositionManager.h"

struct RelayState
{
    int stopTime = -1;
    int startTime = -1;
    bool triggerManual = false;
};
RelayState relayState;

ushort R1_pin_MoveLeft;
ushort R2_pin_MoveRight;
ushort R3_pin;
ushort R4_pin;
ushort R0_pin_Power;

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

class IOManager
{
private:
    Config config;
    PositionManager *posManager;
    uint8_t previousBtnPressed = 0;

public:
    IOManager() {}

    IOManager(Config _config)
    {
        config = _config;

        pinMode(config.B1_pin_Auto, INPUT);
        pinMode(config.B2_pin_MoveRight, INPUT);
        pinMode(config.B3_pin_MoveLeft, INPUT);
        // pinMode(config.LED1_pin_Auto, OUTPUT);
        // pinMode(config.LED2_pin_Manual, OUTPUT);
        // digitalWrite(config.LED1_pin_Auto, LOW);
        // digitalWrite(config.LED2_pin_Manual, LOW);

        R1_pin_MoveLeft = config.R1_pin_MoveLeft;
        R2_pin_MoveRight = config.R2_pin_MoveRight;
        R3_pin = config.R3_pin;
        R4_pin = config.R4_pin;
        R0_pin_Power = config.R0_pin_Power;

        pinMode(R1_pin_MoveLeft, OUTPUT);
        pinMode(R2_pin_MoveRight, OUTPUT);
        pinMode(R3_pin, OUTPUT);
        pinMode(R4_pin, OUTPUT);
        pinMode(R0_pin_Power, OUTPUT);

        SetRelayState(R0_pin_Power, false);
        SetRelayState(R1_pin_MoveLeft, false);
        SetRelayState(R2_pin_MoveRight, false);
        SetRelayState(R3_pin, false);
        SetRelayState(R4_pin, false);
    }

    void SetPositionManager(PositionManager *_positioningManager)
    {
        posManager = _positioningManager;
    }

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

    void SetRelayState(ushort relayPin, bool state)
    {
        digitalWrite(relayPin, state ? LOW : HIGH);
    }

    void TriggerMoveLeft()
    {
        if (IsMaxLeft())
        {
            Serial.println("At Max Left Position. Cannot Move Left.");
            return;
        }

        SetRelayState(R1_pin_MoveLeft, true);
    }
    void TriggerMoveRight()
    {
        if (IsMaxRight())
        {
            Serial.println("At Max Right Position. Cannot Move Right.");
            return;
        }

        SetRelayState(R2_pin_MoveRight, true);
    }

    void TriggerR3()
    {
        SetRelayState(R3_pin, true);
        // delay(1000);
        // SetRelayState(R0_pin_Power, true);
    }

    void TriggerR4()
    {
        SetRelayState(R4_pin, true);
        // delay(1000);
        // SetRelayState(R0_pin_Power, true);
    }

    void ResetRelays()
    {
        Serial.println("Resetting Relays");
        SetRelayState(R1_pin_MoveLeft, false);
        SetRelayState(R2_pin_MoveRight, false);
        SetRelayState(R3_pin, false);
        SetRelayState(R4_pin, false);
    }
};
#endif