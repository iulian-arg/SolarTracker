
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
    MaxSouth = -2,
    MoveSouth = -1,
    NoMove = 0,
    MoveNorth = 1,
    MaxNorth = 2
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
    // Initialize MoveEventQueue with one default MoveEvent so back() is valid
    std::vector<MoveEvent> MoveEventQueue = std::vector<MoveEvent>(1, MoveEvent{MoveDirection::NoMove, time(nullptr)});

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
        if (MoveEventQueue.empty())
        {
            MoveEventQueue.push_back({MoveDirection::NoMove, time(nullptr)});
        }

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
            getLastMoveEvent().direction == MoveDirection::NoMove)
        {
            return; // No change in move event
        }
        ResetMovement();
        AddMoveEventQueue(MoveDirection::NoMove);
    }

    void AddMoveEventQueue(MoveDirection direction)
    {
        if (MoveEventQueue.size() > 0 &&
            getLastMoveEvent().direction == direction)
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

        Logger::info(TAG, "<0_%.1f, 1_%.1f, <>_%d, dif_%.1f, %.1fºC, %s>",
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

    bool OngoingMovement()
    {
        return getLastMoveEvent().direction != MoveDirection::NoMove;
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
            // start move north
            previousBtnPressed = config.B2_pin_MoveNorth;
            SetPositioningMode(PositionMode::Manual);
            TryMoveNorth();
        }
        else if (B2_pin_MoveNorth_state == _pressed &&
                 previousBtnPressed == config.B2_pin_MoveNorth && isMaxNorth())
        {
            // reached max north while holding button
            ResetMovement();
            Logger::warn(TAG, "MAX NORTH while holding button. Reset movements.");
        }
        else if (B3_pin_MoveSouth_state == _pressed &&
                 previousBtnPressed != config.B3_pin_MoveSouth)
        {
            previousBtnPressed = config.B3_pin_MoveSouth;
            SetPositioningMode(PositionMode::Manual);
            TryMoveSouth();
        }
        else if (B3_pin_MoveSouth_state == _pressed &&
                 previousBtnPressed == config.B3_pin_MoveSouth && isMaxSouth())
        {
            // reached max south while holding button
            ResetMovement();
            Logger::warn(TAG, "MAX SOUTH while holding button. Reset movements.");
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

    MoveEvent getLastMoveEvent()
    {
        return MoveEventQueue.back();
    }

    void ManageMaxCommands()
    {
        if (MoveEventQueue.size() == 0)
        {
            return;
        }
        MoveEvent lastEvent = getLastMoveEvent();
        if (lastEvent.direction == MoveDirection::MaxNorth)
        {
            TryMoveNorth(true);
        }
        else if (lastEvent.direction == MoveDirection::MaxSouth)
        {
            TryMoveSouth(true);
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

    bool isMaxNorth()
    {
        return analogRead(config.POT1_pin_MaxAngl) <= config.POT_Max_North_Val;
    }
    bool isMaxSouth()
    {
        return analogRead(config.POT1_pin_MaxAngl) >= config.POT_Max_South_Val;
    }

    void TryMoveSouth(bool isMaxCommand = false)
    {
        Logger::warn(TAG, "ongoingMovement: %d", getLastMoveEvent().direction);

        Logger::warn(TAG, "Move %s South Triggered", isMaxCommand ? "MAX" : "");
        // Logger::info(TAG, "\n %d %d %d \n", config.POT1_pin_MaxAngl, config.POT_Max_South_Val, analogRead(config.POT1_pin_MaxAngl));
        if (isMaxSouth())
        {
            ResetMovement();
            Logger::warn(TAG, "MAX SOUTH. Reset movements.");
            return;
        }
        auto newMoveDirrection = isMaxCommand ? MoveDirection::MaxSouth : MoveDirection::MoveSouth;

        if (getLastMoveEvent().direction == MoveDirection::MaxNorth ||
            getLastMoveEvent().direction == MoveDirection::MoveNorth)
        {
            ResetMovement();
        }
        if (newMoveDirrection != getLastMoveEvent().direction)
        {
            AddMoveEventQueue(newMoveDirrection);
        }
        SetRelayState(config.R2_pin_MoveNorth, false);
        delay(100);
        SetRelayState(config.R1_pin_MoveSouth, true);
        delay(100);
        SetRelayState(config.R0_pin_Power, true);
    }

    void TryMoveNorth(bool isMaxCommand = false)
    {
        Logger::warn(TAG, "ongoingMovement: %d", getLastMoveEvent().direction);
        Logger::warn(TAG, "Move %s North Triggered", isMaxCommand ? "MAX" : "");
        if (isMaxNorth())
        {
            ResetMovement();
            Logger::warn(TAG, "MAX NORTH. Reset movements.");
            return;
        }
        auto newMoveDirrection = isMaxCommand ? MoveDirection::MaxNorth : MoveDirection::MoveNorth;
        if (getLastMoveEvent().direction == MoveDirection::MaxSouth ||
            getLastMoveEvent().direction == MoveDirection::MoveSouth)
        {
            ResetMovement();
        }
        if (newMoveDirrection != getLastMoveEvent().direction)
        {
            AddMoveEventQueue(newMoveDirrection);
        }
        SetRelayState(config.R1_pin_MoveSouth, false);
        delay(100);
        SetRelayState(config.R2_pin_MoveNorth, true);
        delay(100);
        SetRelayState(config.R0_pin_Power, true);
    }

    void ResetMovement()
    {
        AddMoveEventQueue(MoveDirection::NoMove);
        Logger::warn(TAG, "Resetting Movement");
        SetRelayState(config.R0_pin_Power, false);
        delay(50);
        SetRelayState(config.R1_pin_MoveSouth, false);
        SetRelayState(config.R2_pin_MoveNorth, false);
        delay(50);
    }
};
#endif
