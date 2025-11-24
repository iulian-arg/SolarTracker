
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

extern Config config;
extern SensorManager *sensorManager;
class PositionManager
{
private:
    SensorInfo sensorInfo;
    uint8_t previousBtnPressed = 0;

    std::vector<float> LuxDiffQueue = std::vector<float>();
    std::vector<float> LuxAVGQueue = std::vector<float>();
    std::vector<PositioningModeChange> positioningModeChangeQueue = std::vector<PositioningModeChange>();
    std::vector<MoveEvent> MoveEventQueue = std::vector<MoveEvent>();

public:
    PositionManager()
    {
        SetPositioningMode(PositionMode::Automatic);
        positioningModeChangeQueue.reserve(config.lightTrackingQueueSize);
        positioningModeChangeQueue.push_back({PositionMode::Automatic, time(nullptr)});
        LuxAVGQueue.reserve(config.lightTrackingQueueSize);
        LuxAVGQueue.push_back(0.1);
        LuxDiffQueue.reserve(config.lightTrackingQueueSize);
        LuxDiffQueue.push_back(100);
        MoveEventQueue.reserve(config.lightTrackingQueueSize);

        pinMode(config.B1_pin_Auto, INPUT);
        pinMode(config.B2_pin_MoveRight, INPUT);
        pinMode(config.B3_pin_MoveLeft, INPUT);
        // pinMode(config.LED1_pin_Auto, OUTPUT);
        // digitalWrite(config.LED1_pin_Auto, LOW);

        pinMode(config.R0_pin_Power, OUTPUT);
        pinMode(config.R1_pin_MoveLeft, OUTPUT);
        pinMode(config.R2_pin_MoveRight, OUTPUT);
        // pinMode(config.R3_pin, OUTPUT);
        pinMode(config.POT1_pin_MaxAngl, INPUT);

        SetRelayState(config.R0_pin_Power, false);
        SetRelayState(config.R1_pin_MoveLeft, false);
        SetRelayState(config.R2_pin_MoveRight, false);
        // SetRelayState(config.R3_pin, false);
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
        // PrintQueues();
        CheckForLowLight();
        PrintPositioningMode();
        PositionMode currentMode = positioningModeChangeQueue.back().mode;
        if (currentMode == PositionMode::Automatic)
        {
            MoveDirection positionChange = CheckPositionChangeNeeded();
            if (positionChange == MoveDirection::MoveRight)
            {
                TryMoveRight();
            }
            else if (positionChange == MoveDirection::MoveLeft)
            {
                TryMoveLeft();
            }
            else
            {
                ResetMoving();
            }
        }
        else if (currentMode == PositionMode::LowLight)
        {
            return;
        }
        else if (currentMode == PositionMode::Manual)
        {
            // Manual positioning logic
        }
        else
        {
            Logger::info(TAG, "Unknown positioning mode.");
        }
    }

    void ResetMoving()
    {
        if (MoveEventQueue.size() > 0 &&
            MoveEventQueue.back().direction == MoveDirection::NoMove)
        {
            return; // No change in move event
        }
        Logger::info(TAG, "ResetMoving");
        ResetMovement();
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
        String msg = "Current Positioning Mode: ";
        switch (positioningModeChangeQueue.back().mode)
        {
        case PositionMode::Manual:
            msg += "MANUAL";
            break;
        case PositionMode::Automatic:
            msg += "AUTOMATIC";
            break;
        case PositionMode::LowLight:
            msg += "LOW LIGHT";
            break;
        default:
            msg += "UNKNOWN";
            break;
        }
        Logger::info(TAG, msg.c_str());
    }

    void SetPositioningMode(PositionMode mode)
    {
        if (positioningModeChangeQueue.size() > 0 &&
            positioningModeChangeQueue.back().mode == mode)
        {
            return; // No change in mode
        }
        String msg = "Changing Positioning Mode from ";
        if (positioningModeChangeQueue.size() > 0)
        {
            msg += GetPositioningModeString(positioningModeChangeQueue.back());
        }
        else
        {
            msg += "NONE";
        }
        msg += " to: ";
        positioningModeChangeQueue.push_back({mode, time(nullptr)});
        if (positioningModeChangeQueue.size() > config.lightTrackingQueueSize)
        {
            positioningModeChangeQueue.erase(positioningModeChangeQueue.begin());
        }
        msg += GetPositioningModeString(positioningModeChangeQueue.back());
        Logger::info(TAG, msg.c_str());
    }

    String GetPositioningModeString(PositioningModeChange modeChange)
    {
        switch (modeChange.mode)
        {
        case PositionMode::Manual:
            return "MANUAL ";
        case PositionMode::Automatic:
            return "AUTOMATIC ";
        case PositionMode::LowLight:
            return "LOW LIGHT ";
        default:
            return "UNKNOWN ";
        }
    }

    PositionMode GetPositioningMode()
    {
        return positioningModeChangeQueue.back().mode;
    }

    void PrintQueues()
    {
        String msg = "-- DIFF :";
        for (const float &entry : LuxDiffQueue)
        {
            msg += String(" %.2f  ", entry);
        }
        Logger::info(TAG, msg.c_str());
        msg = "-- AVG :";
        for (const float &entry : LuxAVGQueue)
        {
            msg += String(" %.2f  ", entry);
        }
        Logger::info(TAG, msg.c_str());
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
            SetPositioningMode(PositionMode::Manual);
            TryMoveRight();
        }
        else if (b3_pin_MoveLeft_state == _pressed &&
                 previousBtnPressed != config.B3_pin_MoveLeft)
        {
            previousBtnPressed = config.B3_pin_MoveLeft;
            SetPositioningMode(PositionMode::Manual);
            TryMoveLeft();
        }
        else if (b1_pin_Auto_state == _pressed &&
                 previousBtnPressed != config.B1_pin_Auto)
        {
            previousBtnPressed = config.B1_pin_Auto;
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
            Logger::info(TAG, "Btn Released, Resetting Movement");
            previousBtnPressed = 0;
            ResetMoving();
        }
    }

    void UpdateLEDStates()
    {
        if (millis() % config.ledBlinkIntervalMs < config.ledBlinkDurationMs)
        {
            if (GetPositioningMode() == PositionMode::Automatic)
            {
                digitalWrite(config.LED1_pin_Auto, HIGH);
            }
            else if (GetPositioningMode() == PositionMode::Manual)
            {
                digitalWrite(config.LED1_pin_Auto, LOW);
            }
        }
        else
        {
            digitalWrite(config.LED1_pin_Auto, LOW);
        }
    }

    void SetRelayState(ushort relayPin, bool state)
    {
        digitalWrite(relayPin, state ? LOW : HIGH);
    }

    void TryMoveLeft()
    {
        Logger::info(TAG, "Move Left Triggered");
        // Logger::info(TAG, "\n %d %d %d \n", config.POT1_pin_MaxAngl, config.POT_Max_Left_Val, analogRead(config.POT1_pin_MaxAngl));
        if (analogRead(config.POT1_pin_MaxAngl) >= config.POT_Max_Left_Val)
        {
            ResetMovement();
            Logger::info(TAG, "MAX LEFT. Reset movements.");
            return;
        }
        AddMoveEventQueue(MoveDirection::MoveLeft);
        SetRelayState(config.R2_pin_MoveRight, false);
        delay(100);
        SetRelayState(config.R0_pin_Power, true);
        delay(100);
        SetRelayState(config.R1_pin_MoveLeft, true);
    }
    void TryMoveRight()
    {
        Logger::info(TAG, "Move Right Triggered");
        // Logger::info(TAG, "\n %d %d %d \n", config.POT1_pin_MaxAngl, config.POT_Max_Right_Val, analogRead(config.POT1_pin_MaxAngl));

        if (analogRead(config.POT1_pin_MaxAngl) <= config.POT_Max_Right_Val)
        {
            ResetMovement();
            Logger::info(TAG, "MAX RIGHT. Reset movements.");
            return;
        }
        AddMoveEventQueue(MoveDirection::MoveRight);
        SetRelayState(config.R1_pin_MoveLeft, false);
        delay(100);
        SetRelayState(config.R0_pin_Power, true);
        delay(100);
        SetRelayState(config.R2_pin_MoveRight, true);
    }

    void ResetMovement()
    {
        Logger::info(TAG, "Resetting Movement");
        SetRelayState(config.R0_pin_Power, false);
        SetRelayState(config.R1_pin_MoveLeft, false);
        SetRelayState(config.R2_pin_MoveRight, false);
        delay(50);
    }
};
#endif
