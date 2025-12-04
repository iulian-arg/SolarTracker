
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
    _moveNorth,
    _moveSouth,
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
    MoveSouth = -1,
    NoMove = 0,
    MoveNorth = 1
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
        pinMode(config.B2_pin_MoveNorth, INPUT);
        pinMode(config.B3_pin_MoveSouth, INPUT);
        // pinMode(config.LED1_pin_Auto, OUTPUT);
        // digitalWrite(config.LED1_pin_Auto, LOW);

        pinMode(config.R0_pin_Power, OUTPUT);
        pinMode(config.R1_pin_MoveSouth, OUTPUT);
        pinMode(config.R2_pin_MoveNorth, OUTPUT);
        // pinMode(config.R3_pin, OUTPUT);
        pinMode(config.POT1_pin_MaxAngl, INPUT);

        SetRelayState(config.R0_pin_Power, false);
        SetRelayState(config.R1_pin_MoveSouth, false);
        SetRelayState(config.R2_pin_MoveNorth, false);
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
                return MoveDirection::MoveNorth;
            }
            else if (negativeCount == LuxDiffQueue.size())
            {
                return MoveDirection::MoveSouth;
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
            if (positionChange == MoveDirection::MoveNorth)
            {
                TryMoveNorth();
            }
            else if (positionChange == MoveDirection::MoveSouth)
            {
                TryMoveSouth();
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
            Logger::warn(TAG, "Unknown positioning mode.");
        }
    }

    void ResetMoving()
    {
        if (MoveEventQueue.size() > 0 &&
            MoveEventQueue.back().direction == MoveDirection::NoMove)
        {
            return; // No change in move event
        }
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
        String posMode;
        switch (positioningModeChangeQueue.back().mode)
        {
        case PositionMode::Manual:
            posMode = "MAN";
            break;
        case PositionMode::Automatic:
            posMode = "AUTO";
            break;
        case PositionMode::LowLight:
            posMode = "LOW";
            break;
        default:
            posMode = "UNKNOWN";
            break;
        }

        Logger::info(TAG, "<s0:%.2f, s1:%.2f, <>:%d, dif:%.2f, %.2fºC, %s>",
                     sensorInfo.lux_0,
                     sensorInfo.lux_1,
                     sensorInfo.angleSensorValue,
                     sensorInfo.luxDiffPercent,
                     sensorInfo.temperatureC,
                     posMode.c_str());
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
        Logger::warn(TAG, msg.c_str());
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
        BtnState B2_pin_MoveNorth_state = digitalRead(config.B2_pin_MoveNorth) == HIGH ? _pressed : _notPressed;
        BtnState B3_pin_MoveSouth_state = digitalRead(config.B3_pin_MoveSouth) == HIGH ? _pressed : _notPressed;

        if (B2_pin_MoveNorth_state == _pressed &&
            previousBtnPressed != config.B2_pin_MoveNorth)
        {
            previousBtnPressed = config.B2_pin_MoveNorth;
            SetPositioningMode(PositionMode::Manual);
            TryMoveNorth();
        }
        else if (B3_pin_MoveSouth_state == _pressed &&
                 previousBtnPressed != config.B3_pin_MoveSouth)
        {
            previousBtnPressed = config.B3_pin_MoveSouth;
            SetPositioningMode(PositionMode::Manual);
            TryMoveSouth();
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
                 B2_pin_MoveNorth_state == _notPressed &&
                 B3_pin_MoveSouth_state == _notPressed)
        {
            Logger::warn(TAG, "Btn Released, Resetting Movement");
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

    void TryMoveSouth()
    {
        Logger::warn(TAG, "Move South Triggered");
        // Logger::info(TAG, "\n %d %d %d \n", config.POT1_pin_MaxAngl, config.POT_Max_South_Val, analogRead(config.POT1_pin_MaxAngl));
        if (analogRead(config.POT1_pin_MaxAngl) >= config.POT_Max_South_Val)
        {
            ResetMovement();
            Logger::warn(TAG, "MAX SOUTH. Reset movements.");
            return;
        }
        AddMoveEventQueue(MoveDirection::MoveSouth);
        SetRelayState(config.R2_pin_MoveNorth, false);
        delay(100);
        SetRelayState(config.R1_pin_MoveSouth, true);
        delay(100);
        SetRelayState(config.R0_pin_Power, true);
    }
    void TryMoveNorth()
    {
        Logger::warn(TAG, "Move North Triggered");
        // Logger::info(TAG, "\n %d %d %d \n", config.POT1_pin_MaxAngl, config.POT_Max_North_Val, analogRead(config.POT1_pin_MaxAngl));

        if (analogRead(config.POT1_pin_MaxAngl) <= config.POT_Max_North_Val)
        {
            ResetMovement();
            Logger::warn(TAG, "MAX NORTH. Reset movements.");
            return;
        }
        AddMoveEventQueue(MoveDirection::MoveNorth);
        SetRelayState(config.R1_pin_MoveSouth, false);
        delay(100);
        SetRelayState(config.R2_pin_MoveNorth, true);
        delay(100);
        SetRelayState(config.R0_pin_Power, true);
    }

    void ResetMovement()
    {
        Logger::warn(TAG, "Resetting Movement");
        SetRelayState(config.R0_pin_Power, false);
        delay(50);
        SetRelayState(config.R1_pin_MoveSouth, false);
        SetRelayState(config.R2_pin_MoveNorth, false);
        delay(50);
    }
};
#endif
