/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

html page generator for cooking

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>
#include <limits.h>
#include <ESP8266WebServer.h>
#include <DateTime.h>

#include "debugUtl.h"
#include "cFanMachine.h"
#include "AjaxText.h"
#include "common.h"
#include "commonString.h"
#include "enumsToString.h"



void MachineStatusUpdateCallback(cFanMachine *pMachine)
{
  if (pMachine == NULL)
  {
    // initialization
    //AjaxInit();
    return;
  }

#ifdef VERBOSE_SERIAL
  SerialPrintMillis();
  Serial.println(F("MachineStatusUpdateCallback() begin"));
#endif //VERBOSE_SERIAL

  char s[40];

  cFanStatus fanStatus, lightStatus;
  unsigned long fanTimer, lightTimer;
  bool ok = pMachine->GetStatusFan(&fanStatus, &fanTimer);
  ok = pMachine->GetStatusLight(&lightStatus, &lightTimer);
  String strFanStatus = SingleLoadHandler::cFanStatusToString(fanStatus);
  String strFanTimer = String(strSecToHhMmSs(s, sizeof(s), fanTimer));
  String strLightStatus = SingleLoadHandler::cFanStatusToString(lightStatus);
  String strLightTimer = String(strSecToHhMmSs(s, sizeof(s), lightTimer));
  String temperat = TemperatureToString(machineIOs.GetTemperature(), machineIOs.GetTemperatureUnits());
  
#ifdef VERBOSE_SERIAL
  SerialPrintMillis();
  Serial.println(F("MachineStatusUpdateCallback() before PrepareAjaxText_cFan()"));
#endif //VERBOSE_SERIAL

  DateTime::LocalTimeToString(s, sizeof(s), TimePrintOption_e::SecondsYes);
  PrepareAjaxText_cFan(s, // now
                       (int)fanStatus, strFanStatus, strFanTimer,
                       (int)lightStatus, strLightStatus, strLightTimer,
                       temperat);
  
#ifdef VERBOSE_SERIAL
  SerialPrintMillis();
  Serial.println(F("MachineStatusUpdateCallback() end"));
#endif //VERBOSE_SERIAL
}



void MachineInitConfig(cFanMachine *pMachine)
{
#ifdef VERBOSE_SERIAL
  SerialPrintMillis();
  Serial.println(F("MachineInitConfig() begin"));
#endif //VERBOSE_SERIAL

  char s[100];

  cFanStatus fanStatus, lightStatus;
  unsigned long fanTimer, lightTimer;
  bool ok = pMachine->GetStatusFan(&fanStatus, &fanTimer);
  ok = pMachine->GetStatusLight(&lightStatus, &lightTimer);

  SingleLoadConfigModel fanPresets, lightPresets;
  pMachine->GetPreSets(&fanPresets, &lightPresets);
  String strFanPresets = String(SingleLoadHandler::PresetsToString(s, sizeof(s), &fanPresets));
  String strLightPresets = String(SingleLoadHandler::PresetsToString(s, sizeof(s), &lightPresets));

  DateTime *pDt = new DateTime(DateTime::Now());
  PrepareAjaxText_MachineConfiguration(
         PublicName(NULL), pMachine->GetDescription(),
         fanStatus != cFanStatus::Disabled, strFanPresets,
         lightStatus != cFanStatus::Disabled, strLightPresets,
         pDt->Year());
}

