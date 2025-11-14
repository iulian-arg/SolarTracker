
#ifndef PositionManager_H
#define PositionManager_H

// #include <Arduino.h>
// #include <time.h>
// #include "ConfigManager.h"
// #include "SensorManager.h"
// #include "RelayManager.h"
#include "IOManager.h"

#include <vector>

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
    //IOManager *ioManager;

// uint8_t queueSize = 30;

class PositionManager
{
private:
    Config config;
    SensorManager *sensorManager;
    SensorInfo sensorInfo;

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
        PrintQueues();
        CheckForLowLight();
        PrintPositioningMode();
        PositionMode currentMode = positioningModeChangeQueue.back().mode;
        if (currentMode == PositionMode::Automatic)
        {
            MoveDirection positionChange = CheckPositionChangeNeeded();
            if (positionChange == MoveDirection::MoveRight)
            {

                Serial.println("UpdatePositioning: Move Right Triggered");
                TriggerMoveRight();
            }
            else if (positionChange == MoveDirection::MoveLeft)
            {
                Serial.println("UpdatePositioning: Move Left Triggered");
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

    void TriggerMoveRight()
    {
        Serial.println("Move Right Triggered");
        // ioManager->TriggerMoveRight();
        // ioManager->TriggerMoveRight();
        AddMoveEventQueue(MoveDirection::MoveRight);
    }

    void TriggerMoveLeft()
    {
        Serial.println("Move Left Triggered");
        // ioManager->TriggerMoveLeft();
        AddMoveEventQueue(MoveDirection::MoveLeft);
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

    void printConfig()
    {
        Serial.println("PositionManager Config:");
        Serial.printf("lightDiffTreshold: %.2f\n", config.lightDiffTreshold);
        Serial.printf("lowLightTreshold: %d\n", config.lowLightTreshold);
        Serial.printf("lightTrackingQueueSize: %d\n", config.lightTrackingQueueSize);
    }

    void SetPreviousPositioningMode()
    {
        Serial.println("Reverting to previous positioning mode.");
        PositioningModeChange previousMode =
            positioningModeChangeQueue.at(positioningModeChangeQueue.size() - 2);
        SetPositioningMode(previousMode.mode);
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
};
#endif
