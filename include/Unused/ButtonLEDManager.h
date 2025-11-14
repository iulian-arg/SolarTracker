#ifndef ButtonLEDManager_H
#define ButtonLEDManager_H


#include <Arduino.h>
#include "ConfigManager.h"
#include "PositioningManager.h"

// #include <vector>

enum ButtonState
{
    notPressed,
    Pressed,
};

enum ButtonCommand
{
    none,
    moveRight,
    moveLeft,
    manualMode,
    automaticMode,
};

class ButtonLEDManager
{
private:
    Config config;
    // PositioningManager *posManager;
    uint8_t previousButtonPressed = 0;

public:
    // Default constructor
    ButtonLEDManager();

    // Constructor to initialize relayManager
    ButtonLEDManager(Config _config)//, PositioningManager *_positioningManager)
    {
        config = _config;
        // posManager = _positioningManager;
        pinMode(config.B1_pin_Auto, INPUT);
        pinMode(config.B2_pin_MoveRight, INPUT);
        pinMode(config.B3_pin_MoveLeft, INPUT);
        // pinMode(config.LED1_pin_Auto, OUTPUT);
        // pinMode(config.LED2_pin_Manual, OUTPUT);
        // digitalWrite(config.LED1_pin_Auto, LOW);
        // digitalWrite(config.LED2_pin_Manual, LOW);
    }
/**
    void MonitorButtonStates()
    {
        ButtonState b1_pin_Auto_state = digitalRead(config.B1_pin_Auto) == HIGH ? Pressed : notPressed;
        ButtonState b2_pin_MoveRight_state = digitalRead(config.B2_pin_MoveRight) == HIGH ? Pressed : notPressed;
        ButtonState b3_pin_MoveLeft_state = digitalRead(config.B3_pin_MoveLeft) == HIGH ? Pressed : notPressed;

        if (b2_pin_MoveRight_state == Pressed &&
            previousButtonPressed != config.B2_pin_MoveRight)
        {
            previousButtonPressed = config.B2_pin_MoveRight;
            Serial.println("Move Right Button Pressed");
            // delay(50); // Debounce delay
            posManager->SetPositioningMode(PositioningMode::Manual);
            posManager->TriggerMoveRight();
        }
        else if (b3_pin_MoveLeft_state == Pressed &&
                 previousButtonPressed != config.B3_pin_MoveLeft)
        {
            previousButtonPressed = config.B3_pin_MoveLeft;
            Serial.println("Move Left Button Pressed");
            // delay(50); // Debounce delay
            posManager->SetPositioningMode(PositioningMode::Manual);
            posManager->TriggerMoveLeft();
        }
        else if (b1_pin_Auto_state == Pressed &&
                 previousButtonPressed != config.B1_pin_Auto)
        {
            previousButtonPressed = config.B1_pin_Auto;
            Serial.println("Automatic Mode Button Pressed");
            // delay(50); // Debounce delay
            if (posManager->GetPositioningMode() == PositioningMode::Manual)
            {
                posManager->SetPositioningMode(PositioningMode::Automatic);
            }
        }
        else if (previousButtonPressed != 0 &&
                 b1_pin_Auto_state == notPressed &&
                 b2_pin_MoveRight_state == notPressed &&
                 b3_pin_MoveLeft_state == notPressed)
        {
            Serial.println("Button Released, Resetting Movement");
            previousButtonPressed = 0;
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
            if (posManager->GetPositioningMode() == PositioningMode::Automatic)
            {
                digitalWrite(config.LED1_pin_Auto, HIGH);
                digitalWrite(config.LED2_pin_Manual, LOW);
            }
            else if (posManager->GetPositioningMode() == PositioningMode::Manual)
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