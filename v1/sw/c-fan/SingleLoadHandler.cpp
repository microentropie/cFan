/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>
//#include <Ticker.h>
#include "SingleLoadHandler.h"
//#include "machine_io.h"
//#include "enumsToString.h"
#include "debugUtl.h"

// one day  is 86400 = 24 * 60 * 60 seconds
// one week is 604800 = 86400 * 7 seconds
#define ONE_DAY 86400
#define ONE_WEEK 604800


SingleLoadHandler::SingleLoadHandler()
{
  ZeroVariables();
}

void SingleLoadHandler::ZeroVariables()
{
  id = '?';
  currentStatus.mode = cFanStatus::Disabled;
  currentStatus.currentTimer = 0L;
  currentStatus.loadTimer = 0L;

  loadValues.EnMain = cFanModeMain::Disabled;
  loadValues.EnTempH = cFanModeTemp::Off;
  loadValues.EnTempL = cFanModeTemp::Off;
  loadValues.temperatureToggleValue = -1;
  loadValues.timerMain = 0;
  loadValues.timerTempH = 0;
  loadValues.timerTempL = 0;

  initiallyOffTimeSec = 1 * 60; // 1'
}

void SingleLoadHandler::Init(char id, SingleLoadConfigModel *pLoadValues, int temperature, int initiallyOffTimeMin)
{
#ifdef VERBOSE_SERIAL
  this->id = toupper(id);
  Serial.print(F("SingleLoadHandler-"));
  Serial.print(this->id);
  Serial.println(F("::Init()"));
#endif //VERBOSE_SERIAL
  if (pLoadValues == NULL)
    panic();
  this->loadValues = *pLoadValues;
  this->initiallyOffTimeSec = initiallyOffTimeMin * 60;
  SetStatusFromPresets(pLoadValues, temperature);
}


void SingleLoadHandler::Set(cFanStatus newStatus, unsigned long newTimer)
{
#ifdef VERBOSE_SERIAL
  Serial.print(F("SingleLoadHandler-"));
  Serial.print(this->id);
  Serial.print(F("::Set(): cFanStatus input: "));
  Serial.println((int)newStatus);
#endif //VERBOSE_SERIAL
  if (newStatus != cFanStatus::NoChange)
    currentStatus.mode = newStatus;
#ifdef VERBOSE_SERIAL
  Serial.print("Set(): cFanStatus set: ");
  Serial.println((int)currentStatus.mode);
#endif //VERBOSE_SERIAL
  if (newTimer == STOP_TIMER)
    currentStatus.currentTimer = 0;
  else
  {
    if (newTimer > ONE_WEEK)
      newTimer = ONE_WEEK;
    currentStatus.loadTimer = newTimer;
    currentStatus.currentTimer = currentStatus.loadTimer;
  }
#ifdef VERBOSE_SERIAL
  Serial.print("SingleLoadHandler-");
  Serial.print(this->id);
  Serial.print("::Set() exit => mode = ");
  Serial.print(cFanStatusToString(currentStatus.mode));
  Serial.print(", timer = ");
  Serial.println(currentStatus.loadTimer);
#endif //VERBOSE_SERIAL
}

// add the specified time (seconds)
void SingleLoadHandler::Update(signed long addTimer)
{
#ifdef VERBOSE_SERIAL
  Serial.print("SingleLoadHandler-");
  Serial.print(this->id);
  Serial.print("::Update(");
  Serial.print(addTimer);
  Serial.print("\") => load status = ");
  Serial.print((int)currentStatus.mode);
  Serial.print(", currentTimer = ");
  Serial.println(currentStatus.currentTimer);
#endif //VERBOSE_SERIAL
  if (addTimer == STOP_TIMER)
    currentStatus.currentTimer = 0;
  else
  {
    // set some limits to ON time
    if (addTimer > ONE_DAY)
      addTimer = ONE_DAY;
    else if (addTimer < -ONE_DAY)
      addTimer = -ONE_DAY;
    if (currentStatus.currentTimer + addTimer < 0)
      currentStatus.currentTimer = 0; // cannot subtract below 0
    else
      currentStatus.currentTimer += addTimer; // can be set > loadTimer
    if (currentStatus.currentTimer > ONE_WEEK)
      currentStatus.currentTimer = ONE_WEEK;

    if (currentStatus.currentTimer < 0)
      currentStatus.currentTimer = 0; // should never occur
  }

  if (addTimer > 0 && currentStatus.mode == cFanStatus::InitiallyOffThenOn)
    currentStatus.mode = cFanStatus::On;

  if (currentStatus.currentTimer <= 0 && currentStatus.mode == cFanStatus::On)
    currentStatus.mode = cFanStatus::Off;
  else if (currentStatus.currentTimer > 0 && currentStatus.mode == cFanStatus::Off)
    currentStatus.mode = cFanStatus::On; // user has forced on
#ifdef VERBOSE_SERIAL
  Serial.print("SingleLoadHandler-");
  Serial.print(this->id);
  Serial.print("::Update() exit => load status = ");
  Serial.print((int)currentStatus.mode);
  Serial.print(", currentTimer = ");
  Serial.print(currentStatus.currentTimer);
  Serial.print(", load timer = ");
  Serial.println(currentStatus.loadTimer);
#endif //VERBOSE_SERIAL
}

SingleLoadConfigModel *SingleLoadHandler::GetPreSets()
{
  return &loadValues;
}

bool SingleLoadHandler::GetStatus(cFanStatus *pStatus, unsigned long *pTimer)
{
  if (pStatus)
    *pStatus = currentStatus.mode;
  if (pTimer)
    *pTimer = (unsigned long)currentStatus.currentTimer;
  return IsLoadOn();
}


bool SingleLoadHandler::IsInASafeStatus()
{
  return (currentStatus.mode == cFanStatus::Disabled || currentStatus.mode == cFanStatus::Off);
}

bool SingleLoadHandler::IsActive()
{
  return currentStatus.mode == cFanStatus::On ||
    currentStatus.mode == cFanStatus::InitiallyOffThenOn;
}

bool SingleLoadHandler::IsLoadOn()
{
  return currentStatus.mode == cFanStatus::On;
}

void SingleLoadHandler::AsyncUpdate()
{
  if (currentStatus.currentTimer > 0)
    --currentStatus.currentTimer;
  if (currentStatus.currentTimer <= 0 && currentStatus.mode != cFanStatus::Disabled)
  {
    currentStatus.mode = cFanStatus::Off;
    return;
  }
  if (currentStatus.mode == cFanStatus::InitiallyOffThenOn && currentStatus.currentTimer <= currentStatus.loadTimer - initiallyOffTimeSec)
  {
    currentStatus.mode = cFanStatus::On;
    return;
  }
  // no status change
}


void SingleLoadHandler::SetStatusFromPresets(SingleLoadConfigModel *pLoadValues, int temperature)
{
#ifdef VERBOSE_SERIAL
  Serial.print("SingleLoadHandler-");
  Serial.print(this->id);
  Serial.println("::SetStatusFromPresets()");
#endif //VERBOSE_SERIAL
  if (pLoadValues == NULL)
    return;

  switch (pLoadValues->EnMain)
  {
  case cFanModeMain::Disabled:
    Set(cFanStatus::Disabled, 0);
    return;
  case cFanModeMain::Off:
    Set(cFanStatus::Off, 0);
    return;
  case cFanModeMain::On:
  case cFanModeMain::InitiallyOffThenOn:
#ifdef VERBOSE_SERIAL
    Serial.print("cFanModeMain: ");
    Serial.println((int)pLoadValues->EnMain);
    Serial.print("cFanStatus: ");
    Serial.println((int)((cFanStatus)pLoadValues->EnMain));
#endif //VERBOSE_SERIAL
    Set((cFanStatus)pLoadValues->EnMain, pLoadValues->timerMain * 60);
    return;
  case cFanModeMain::TempDependent:
    if (temperature >= pLoadValues->temperatureToggleValue)
    {
      unsigned long tmr = pLoadValues->timerTempH * 60;
      if ((cFanStatus)pLoadValues->EnTempH == cFanStatus::Off)
        tmr = 0UL;
      Set((cFanStatus)pLoadValues->EnTempH, tmr);
    }
    else
    {
      unsigned long tmr = pLoadValues->timerTempL * 60;
      if ((cFanStatus)pLoadValues->EnTempL == cFanStatus::Off)
        tmr = 0UL;
      Set((cFanStatus)pLoadValues->EnTempL, tmr);
    }
    return;
  }

  //Serial.Println("Invalid Status");
  panic();
}

char *SingleLoadHandler::PresetsToString(char *buf, int bufSize, SingleLoadConfigModel *pLoadValues)
{
  if (!buf)
    return NULL;

  if (!pLoadValues)
  {
    snprintf(buf, bufSize, "Err%d", 1);
    return buf;
  }

  switch (pLoadValues->EnMain)
  {
  case cFanModeMain::Disabled:
    snprintf(buf, bufSize, "Disabled.");
    break;
  case cFanModeMain::Off:
    snprintf(buf, bufSize, "Off, timer: %d'.", pLoadValues->timerMain);
    break;
  case cFanModeMain::On:
    snprintf(buf, bufSize, "On, timer: %d'.", pLoadValues->timerMain);
    break;
  case cFanModeMain::InitiallyOffThenOn:
    snprintf(buf, bufSize, "OffThenOn, timer: %d'.", pLoadValues->timerMain);
    break;
  case cFanModeMain::TempDependent:
    snprintf(buf, bufSize, "Temp+ => %s, timer: %d' / Temp- => %s, timer: %d'.",
      SingleLoadHandler::cFanModeTempToString(pLoadValues->EnTempH),
      pLoadValues->timerTempH,
      SingleLoadHandler::cFanModeTempToString(pLoadValues->EnTempL),
      pLoadValues->timerTempL);
    break;
  default:
    snprintf(buf, bufSize, "Err%d=%d.", 2, (int)pLoadValues->EnMain);
  }

  return buf;
}

const char *SingleLoadHandler::cFanModeTempToString(cFanModeTemp mode)
{
  switch (mode)
  {
  case cFanModeTemp::Off:
  case cFanModeTemp::On:
  case cFanModeTemp::InitiallyOffThenOn:
    return SingleLoadHandler::cFanStatusToString((cFanStatus)mode);
  }
  return "Err-cFanModeTempToString";
}

const char *SingleLoadHandler::cFanStatusToString(cFanStatus cStatus)
{
  switch (cStatus)
  {
  case cFanStatus::NoChange:
    return "NoChange";
  case cFanStatus::Disabled:
    return "Disabled";
  case cFanStatus::Off:
    return "Off";
  case cFanStatus::On:
    return "On";
  case cFanStatus::InitiallyOffThenOn:
    return "OffThenOn";
  }
  return "Err-cFanStatusToString";
}

char *SingleLoadHandler::TimerToString(char *buf, int bufSize, unsigned long timer)
{
  if (!buf)
    return NULL;

  if (timer == STOP_TIMER)
    strncpy(buf, "STOP_TIMER", bufSize);
  else
    snprintf(buf, bufSize, "%d\"", timer);
  return buf;
}

