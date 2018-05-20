/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>
//#include <string.h>
#include "machine_io.h"
#include <serializeEeprom.h> 
#include <IRutils.h>
#include "String64.h"

#include "Ir_bl.h"
#include "IrReceive.h"
#include "debugUtl.h"
#include "commonString.h"
#include "bl.h"

void SetAjaxIr(String str);

static IrConfigModel irCfg;

void IrInit()
{
   if (!IrConfig_Load(irCfg))
   {
#ifdef VERBOSE_SERIAL
      Serial.println(F("Failed reading IR config, forcing defaults"));
#endif //VERBOSE_SERIAL

      // error reading from eeprom, set defaults
      irCfg.fan.SetOff = 0ULL;
      irCfg.fan.SetOn = 0ULL;
      irCfg.fan.ToggleOnOff = 0ULL;

      irCfg.light.SetOff = 0ULL;
      irCfg.light.SetOn = 0ULL;
      irCfg.light.ToggleOnOff = 0ULL;

      irCfg.WiFiForceOn = 0ULL;
      irCfg.irOn = false;
      irCfg.dummy = 0;

      IrConfig_Save(irCfg);
   }
   if (irCfg.irOn)
      IrRxStart();
   else
      IrRxStop();
}


void IrResume()
{
   IrRxStop();
}

#include "cFanMachine.h"
extern cFanMachine cFan;

void IrLoop()
{
   uint64_t irCode = IrRxLoop();
   if (irCode == 0ULL/*nothing received*/ || irCode == 0xFFFFFFFFFFFFFFFFULL/*REPEAT command*/) return;

   // decode command
   String rxCommand = String();
   StringAppendMillis(rxCommand);
   rxCommand += F("Received IR = 0x");
   rxCommand += String64(irCode, 16);
   rxCommand += F(" => ");
   if (irCode == irCfg.fan.SetOn)
   {
      unsigned long fanTimer = 3 * 60 * 60; // ON for 3h
      cFan.Update(fanTimer, NOCHANGE_TIMER);
      rxCommand += F("fan \"ON\"");
   }
   else if (irCode == irCfg.fan.SetOff)
   {
      cFan.Update(STOP_TIMER, NOCHANGE_TIMER);
      rxCommand += F("fan \"OFF\"");
   }
   else if (irCode == irCfg.fan.ToggleOnOff)
   {
      // to be implemented
      rxCommand += F("fan \"ToggleOnOff\"");
   }
   else if (irCode == irCfg.light.SetOn)
   {
      unsigned long lightTimer = 2 * 60 * 60; // ON for 2h
      cFan.Update(NOCHANGE_TIMER, lightTimer);
      rxCommand += F("light \"ON\"");
   }
   else if (irCode == irCfg.light.SetOff)
   {
      cFan.Update(NOCHANGE_TIMER, STOP_TIMER);
      rxCommand += F("light \"OFF\"");
   }
   else if (irCode == irCfg.light.ToggleOnOff)
   {
      // to be implemented
      rxCommand += F("light \"ToggleOnOff\"");
   }
   else if (irCode == irCfg.WiFiForceOn)
   {
      rxCommand += F("\"WiFi ON\"");
      // 1st command: force wifi on (change will be effective at next restart)
      // 2nd command: Restart NOW
      unsigned char wrs = machineIOs.WiFiReservationStatus();
      if (wrs > 1)
         Restart(); // 2nd (and all the following) command(s)
      else if (wrs == 1)
         rxCommand += F("command ignored, WiFi is always ON");
      else
         machineIOs.ForceWiFiOn(); // 1st command
   }
   else
   {
      rxCommand += F("unknown");
   }
   Serial.println(rxCommand);
   SetAjaxIr(rxCommand);
}

//----------------------------------------------------------------
// initialize the signature for this type
//                                                         I R R X
template<> uint32_t eepromIf<IrConfigModel>::Signature = 0x49525258;
template<> uint16_t eepromIf<IrConfigModel>::baseOffset = 900;

bool IrConfig_Save(IrConfigModel &cfg)
{
   return eepromIf<IrConfigModel>::Save(&cfg, EEPROM_log_RW);
}
bool IrConfig_Load(IrConfigModel &cfg)
{
   return eepromIf<IrConfigModel>::Load(&cfg, EEPROM_log_RW);
}
