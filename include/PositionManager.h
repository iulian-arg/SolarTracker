
#ifndef PositionManager_H
#define PositionManager_H

// #include <Arduino.h>
// #include <time.h>
// #include "ConfigManager.h"
// #include "SensorManager.h"
// #include "RelayManager.h"
// #include "IOManager.h"

#include <vector>

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

enum PositionMode
{
    LowLight = 0,
    Automatic = 1,
    Manual = 2
};
enum MoveDirection
{
    MoveLeft = -1,
    NoMove = 0,
    MoveRight = 1
};
struct PositioningModeChange
{
    PositionMode mode;
    time_t timestamp;
};
struct MoveEvent
{
    MoveDirection direction;
    time_t timestamp;
};
// IOManager *ioManager;

// uint8_t queueSize = 30;

class PositionManager
{
private:
    Config config;
    SensorManager *sensorManager;
    SensorInfo sensorInfo;
    uint8_t previousBtnPressed = 0;

    std::vector<float> LuxDiffQueue = std::vector<float>();
    std::vector<float> LuxAVGQueue = std::vector<float>();
    std::vector<PositioningModeChange> positioningModeChangeQueue = std::vector<PositioningModeChange>();
    std::vector<MoveEvent> MoveEventQueue = std::vector<MoveEvent>();

public:
    PositionManager(
        Config _config,
        SensorManager *_sensorManager
        // ,
        // IOManager *_ioManager
    )
    {
        config = _config;               // Dynamically allocate and copy the config
        sensorManager = _sensorManager; // This will use the copy assignment operator
        // ioManager = _ioManager;   // This will use the copy assignment operator
        SetPositioningMode(PositionMode::Automatic);
        positioningModeChangeQueue.reserve(config.lightTrackingQueueSize);
        positioningModeChangeQueue.push_back({PositionMode::Automatic, time(nullptr)});
        LuxAVGQueue.reserve(config.lightTrackingQueueSize);
        LuxAVGQueue.push_back(0.1);
        LuxDiffQueue.reserve(config.lightTrackingQueueSize);
        LuxDiffQueue.push_back(100);
        MoveEventQueue.reserve(config.lightTrackingQueueSize);

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

    void ReadLightSensors()
    {
        sensorInfo = sensorManager->GetSensorInfo();
        LuxDiffQueue.push_back(sensorInfo.luxDiffPercent);
        if (LuxDiffQueue.size() > config.lightTrackingQueueSize)
        {
            LuxDiffQueue.erase(LuxDiffQueue.begin());
        }

        LuxAVGQueue.push_back(sensorInfo.luxAverage);
        if (LuxAVGQueue.size() > config.lightTrackingQueueSize)
        {
            LuxAVGQueue.erase(LuxAVGQueue.begin());
        }
        // Serial.printf("Lux Average: %.2f, Lux Difference: %.2f\n",
        //     luxAVGCurrent, luxDiffCurrent);
    }

    MoveDirection CheckPositionChangeNeeded()
    {
        if (positioningModeChangeQueue.back().mode == PositionMode::Automatic)
        {
            uint8_t positiveCount = 0;
            uint8_t negativeCount = 0;
            for (const float &entry : LuxDiffQueue)
            {
                if (entry >= 0 && entry <= config.lightDiffTreshold)
                {
                    positiveCount++;
                }
                else if (entry < 0 && entry >= -config.lightDiffTreshold)
                {
                    negativeCount++;
                }
            }
            if (positiveCount == LuxDiffQueue.size())
            {
                return MoveDirection::MoveRight;
            }
            else if (negativeCount == LuxDiffQueue.size())
            {
                return MoveDirection::MoveLeft;
            }
        }
        return MoveDirection::NoMove;
    }
 
    void UpdatePositioning()
    {
        ReadLightSensors();
        MonitorMaxPositions();
        // PrintQueues();
        CheckForLowLight();
        PrintPositioningMode();
        PositionMode currentMode = positioningModeChangeQueue.back().mode;
        if (currentMode == PositionMode::Automatic)
        {
            MoveDirection positionChange = CheckPositionChangeNeeded();
            if (positionChange == MoveDirection::MoveRight)
            {

                // Serial.println("UpdatePositioning: Move Right Triggered");
                TriggerMoveRight();
            }
            else if (positionChange == MoveDirection::MoveLeft)
            {
                // Serial.println("UpdatePositioning: Move Left Triggered");
                TriggerMoveLeft();
            }
            else
            {
                ResetMoving();
            }
        }
        else if (currentMode == PositionMode::LowLight)
        {
            delay(config.lowLightSleepTimeMs);
            return;
        }
        else if (currentMode == PositionMode::Manual)
        {
            // Manual positioning logic
        }
        else
        {
            Serial.println("Unknown positioning mode.");
        }
    }

    void ResetMoving()
    {
        if (MoveEventQueue.size() > 0 &&
            MoveEventQueue.back().direction == MoveDirection::NoMove)
        {
            return; // No change in move event
        }
        Serial.println("ResetMoving");
        // ioManager->ResetRelays();
        AddMoveEventQueue(MoveDirection::NoMove);
    }

    void AddMoveEventQueue(MoveDirection direction)
    {
        if (MoveEventQueue.size() > 0 &&
            MoveEventQueue.back().direction == direction)
        {
            return; // No change in move event
        }
        MoveEventQueue.push_back({direction, time(nullptr)});
        if (MoveEventQueue.size() > config.lightTrackingQueueSize)
        {
            MoveEventQueue.erase(MoveEventQueue.begin());
        }
    }

    void PrintPositioningMode()
    {
        Serial.print("Current Positioning Mode: ");
        switch (positioningModeChangeQueue.back().mode)
        {
        case PositionMode::Manual:
            Serial.println("MANUAL");
            break;
        case PositionMode::Automatic:
            Serial.println("AUTOMATIC");
            break;
        case PositionMode::LowLight:
            Serial.println("LOW LIGHT");
            break;
        default:
            Serial.println("UNKNOWN");
            break;
        }
    }

    void SetPositioningMode(PositionMode mode)
    {
        if (positioningModeChangeQueue.size() > 0 &&
            positioningModeChangeQueue.back().mode == mode)
        {
            // Serial.println("Positioning mode unchanged.");
            return; // No change in mode
        }
        Serial.print("Changing Positioning Mode from ");
        if (positioningModeChangeQueue.size() > 0)
        {
            Serial.print(GetPositioningModeString(positioningModeChangeQueue.back()));
        }
        else
        {
            Serial.print("NONE");
        }
        Serial.print("to: ");
        positioningModeChangeQueue.push_back({mode, time(nullptr)});
        if (positioningModeChangeQueue.size() > config.lightTrackingQueueSize)
        {
            positioningModeChangeQueue.erase(positioningModeChangeQueue.begin());
        }
        Serial.println(GetPositioningModeString(positioningModeChangeQueue.back()));
    }

    String GetPositioningModeString(PositioningModeChange modeChange)
    {
        switch (modeChange.mode)
        {
        case PositionMode::Manual:
            return "MANUAL " + String(modeChange.timestamp);
        case PositionMode::Automatic:
            return "AUTOMATIC " + String(modeChange.timestamp);
        case PositionMode::LowLight:
            return "LOW LIGHT " + String(modeChange.timestamp);
        default:
            return "UNKNOWN " + String(modeChange.timestamp);
        }
    }

    PositionMode GetPositioningMode()
    {
        return positioningModeChangeQueue.back().mode;
    }

    void PrintQueues()
    {
        Serial.print("-- DIFF :");
        for (const float &entry : LuxDiffQueue)
        {
            Serial.printf("%.2f  ", entry);
        }
        Serial.println();
        Serial.print("-- AVG :");
        for (const float &entry : LuxAVGQueue)
        {
            Serial.printf("%.2f ", entry);
        }
        Serial.println();
    }

    void CheckForLowLight()
    {
        if (GetPositioningMode() == PositionMode::Manual)
        {
            return; // Do not change mode if in Manual
        }
        short lowLightCount = 0;
        for (const float &entry : LuxAVGQueue)
        {
            if (entry < config.lowLightTreshold)
            {
                lowLightCount++;
            }
        }
        if (lowLightCount >= config.lightTrackingQueueSize)
        {
            SetPositioningMode(PositionMode::LowLight);
        }
        else
        {
            SetPositioningMode(PositionMode::Automatic);
        }
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
            // Serial.println("Move Right Btn _pressed");
            SetPositioningMode(PositionMode::Manual);
            TriggerMoveRight();
        }
        else if (b3_pin_MoveLeft_state == _pressed &&
                 previousBtnPressed != config.B3_pin_MoveLeft)
        {
            previousBtnPressed = config.B3_pin_MoveLeft;
            // Serial.println("Move Left Btn _pressed");
            SetPositioningMode(PositionMode::Manual);
            TriggerMoveLeft();
        }
        else if (b1_pin_Auto_state == _pressed &&
                 previousBtnPressed != config.B1_pin_Auto)
        {
            previousBtnPressed = config.B1_pin_Auto;
            // Serial.println("Automatic Mode Btn _pressed");
            if (GetPositioningMode() == PositionMode::Manual)
            {
                SetPositioningMode(PositionMode::Automatic);
            }
        }
        else if (previousBtnPressed != 0 &&
                 b1_pin_Auto_state == _notPressed &&
                 b2_pin_MoveRight_state == _notPressed &&
                 b3_pin_MoveLeft_state == _notPressed)
        {
            Serial.println("Btn Released, Resetting Movement");
            previousBtnPressed = 0;
            ResetMoving();
        }
    }

    void MonitorMaxPositions()
    {
        if (digitalRead(config.B4_pin_MaxLeft) == HIGH)
        {
            Serial.println("Max Left Position Reached");
            ResetMoving();
        }
        if (digitalRead(config.B5_pin_MaxRight) == HIGH)
        {
            Serial.println("Max Right Position Reached");
            ResetMoving();
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
            if (GetPositioningMode() == PositionMode::Automatic)
            {
                digitalWrite(config.LED1_pin_Auto, HIGH);
                digitalWrite(config.LED2_pin_Manual, LOW);
            }
            else if (GetPositioningMode() == PositionMode::Manual)
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

        Serial.println("Move Left Triggered");
        // ioManager->TriggerMoveLeft();
        AddMoveEventQueue(MoveDirection::MoveLeft);
        SetRelayState(R1_pin_MoveLeft, true);
    }
    void TriggerMoveRight()
    {
        if (IsMaxRight())
        {
            Serial.println("At Max Right Position. Cannot Move Right.");
            return;
        }

        Serial.println("Move Right Triggered");
        // ioManager->TriggerMoveRight();
        // ioManager->TriggerMoveRight();
        AddMoveEventQueue(MoveDirection::MoveRight);
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
