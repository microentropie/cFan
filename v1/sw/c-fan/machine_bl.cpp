/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>
#include <WString.h>
#include <string.h>
#include "debugUtl.h"
//#include "settings.h"
#include "cFanMachine.h"
#include "machine_io.h"
#include <serializeEeprom.h> 

#include "machine_bl.h"
#include "machine_events.h"
#include "common.h"

cFanMachine cFan;

void MachineInit()
{
  struct MachineConfigModel eepCfg;
  if (!FanConfig_Load(eepCfg))
  {
#ifdef VERBOSE_SERIAL
    Serial.println(F("Failed reading machine config, setting defaults"));
#endif //VERBOSE_SERIAL

    // error reading from eeprom, set defaults
    strncpy(eepCfg.Description, "?", sizeof(eepCfg.Description)); // buffer padded with \0s
    eepCfg.fan.EnMain = cFanModeMain::Off;
    eepCfg.fan.EnTempH = cFanModeTemp::Off;
    eepCfg.fan.EnTempL = cFanModeTemp::Off;
    eepCfg.fan.temperatureToggleValue = 25;
    eepCfg.fan.timerMain = 10;
    eepCfg.fan.timerTempH = 10;
    eepCfg.fan.timerTempL = 10;

    eepCfg.light.EnMain = cFanModeMain::On;
    eepCfg.light.EnTempH = cFanModeTemp::Off;
    eepCfg.light.EnTempL = cFanModeTemp::Off;
    eepCfg.light.temperatureToggleValue = 0;
    eepCfg.light.timerMain = 10;
    eepCfg.light.timerTempH = 10;
    eepCfg.light.timerTempL = 10;

    eepCfg.bConnectionLedShow = true;
    eepCfg.temperatureScale = temperatureUnits_e::Celsius;
    eepCfg.InitiallyOffTime = 1;
    eepCfg.dummy = 0;

    FanConfig_Save(eepCfg);
  }
#ifdef VERBOSE_SERIAL
    SerialPrintMillis();
    Serial.println(F("before cFan Init"));
#endif //VERBOSE_SERIAL

  cFan.Init(&eepCfg);
  cFan.Update(0, 0); // start timer
 
  AjaxInit();
  MachineInitConfig(&cFan);
}

void MachineUpdateStatus()
{
  cFan.UpdateStatus();
}

void MachineStop()
{
  cFan.Update(STOP_TIMER, STOP_TIMER);
}

bool MachineCommand(String loadType, String cmd)
{
  bool bOk = true;
  unsigned long timer = 0;

  Serial.print("'" + loadType + "'='" + cmd + "'");

  if (cmd == "OFF")
    timer = STOP_TIMER;
  else
  {
    timer = cmd.toInt() * 60UL;
    if(timer == 0UL)
    {
      Serial.print(F(" <== bad"));
      bOk = false;
    }
  }

  if(bOk)
  {
    if (loadType == "Fan")
      cFan.Update(timer, NOCHANGE_TIMER);
    else if (loadType == "Light")
      cFan.Update(NOCHANGE_TIMER, timer);
    else
    {
      Serial.print(F(" <= invalid load type"));
      bOk = false;
    }
  }
  Serial.println();

  return bOk;
}

void MachineForceUpdateSensorsRead()
{
  MachineStatusUpdateCallback(&cFan);
}

bool IsMachineInASafeStatus()
{
  cFan.IsInASafeStatus();
}

//----------------------------------------------------------------
// initialize the signature for this type
template<> uint32_t eepromIf<MachineConfigModel>::Signature = string2Int(FPSTR("cFAN"));
template<> uint16_t eepromIf<MachineConfigModel>::baseOffset = 300;

bool FanConfig_Save(MachineConfigModel &cfg)
{
  return eepromIf<MachineConfigModel>::Save(&cfg, EEPROM_log_RW);
}
bool FanConfig_Load(MachineConfigModel &cfg)
{
  return eepromIf<MachineConfigModel>::Load(&cfg, EEPROM_log_RW);
}
