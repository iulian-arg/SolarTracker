
#ifndef RelayManager_H
#define RelayManager_H

// #include <Arduino.h>
// #include <time.h>
#include "ConfigManager.h"
// #include "BtnLEDManager.h"
// #include <vector>

struct RelayState
{
  int stopTime = -1;
  int startTime = -1;
  bool triggerManual = false;
};
RelayState relayState;

u8_t R1_pin_MoveLeft;
u8_t R2_pin_MoveRight;
u8_t R3_pin;
u8_t POT1_pin_MaxAngl;
u8_t R0_pin_Power;

class RelayManager
{
private:
  Config config;
  BtnLEDManager *btnLEDManager;

public:
  RelayManager() {}

  RelayManager(Config _config)
  {
    config = _config;

    R1_pin_MoveLeft = config.R1_pin_MoveLeft;
    R2_pin_MoveRight = config.R2_pin_MoveRight;
    R3_pin = config.R3_pin;
    POT1_pin_MaxAngl = config.POT1_pin_MaxAngl;
    R0_pin_Power = config.R0_pin_Power;

    pinMode(R1_pin_MoveLeft, OUTPUT);
    pinMode(R2_pin_MoveRight, OUTPUT);
    pinMode(R3_pin, OUTPUT);
    pinMode(POT1_pin_MaxAngl, OUTPUT);
    pinMode(R0_pin_Power, OUTPUT);

    SetRelayState(R0_pin_Power, false);
    SetRelayState(R1_pin_MoveLeft, false);
    SetRelayState(R2_pin_MoveRight, false);
    SetRelayState(R3_pin, false);
    SetRelayState(POT1_pin_MaxAngl, false);
  }
  void SetBtnLEDManager(BtnLEDManager *_btnLEDManager)
  {
    btnLEDManager = _btnLEDManager;
  }

  void SetRelayState(u8_t relayPin, bool state)
  {
    digitalWrite(relayPin, state ? LOW : HIGH);
  }

  void TriggerMoveLeft()
  {
    if (btnLEDManager->IsMaxLeft())
    {
      Serial.println("At Max Left Position. Cannot Move Left.");
      return;
    }

    SetRelayState(R1_pin_MoveLeft, true);
  }
  void TriggerMoveRight()
  {
    if (btnLEDManager->IsMaxRight())
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
    SetRelayState(POT1_pin_MaxAngl, true);
    // delay(1000);
    // SetRelayState(R0_pin_Power, true);
  }

  void ResetRelays()
  {
    Serial.println("Resetting Relays");
    SetRelayState(R1_pin_MoveLeft, false);
    SetRelayState(R2_pin_MoveRight, false);
    SetRelayState(R3_pin, false);
    SetRelayState(POT1_pin_MaxAngl, false);
  }
};
#endif