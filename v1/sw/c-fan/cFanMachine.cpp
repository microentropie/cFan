/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>
#include <Ticker.h>
#include "cFanMachine.h"
#include "machine_io.h"
#include "enumsToString.h"
#include "machineUtl.h"
#include "debugUtl.h"
#include "common.h"
#include "commonString.h"



//---------
cFanMachine::cFanMachine()
{
  ZeroVariables();
  initTimer();
}

cFanMachine::~cFanMachine()
{
  stopTimer();
}

void cFanMachine::ZeroVariables()
{
  fan.ZeroVariables();
  light.ZeroVariables();

  memset(Description, sizeof(Description), 0);
}

// By calling this method, warning timer does not start;
// for that, call:
// cFan.Update(0, 0);
void cFanMachine::Init(MachineConfigModel *pMachineConfig)
{
#ifdef VERBOSE_SERIAL
  Serial.println("cFanMachine::Init()");
#endif //VERBOSE_SERIAL
  stopTimer();
  ZeroVariables();
  machineIOs.Init(pMachineConfig->bConnectionLedShow, pMachineConfig->temperatureScale);
  int temperature = machineIOs.ForceTemperatureRead();
#ifdef VERBOSE_SERIAL
  Serial.print("temperature: ");
  Serial.println(TemperatureToString(temperature, machineIOs.GetTemperatureUnits()));
#endif //VERBOSE_SERIAL
  strncpy(Description, pMachineConfig->Description, sizeof(Description));
  Serial.print("Description=");
  Serial.println(Description);

  SingleLoadConfigModel fanCfg, lightCfg;

  SingleLoadConfigModelFromCompact(&lightCfg, &pMachineConfig->light);
#ifdef VERBOSE_SERIAL
  char buf[100];
  Serial.print("Load Light Config: ");
  Serial.println(SingleLoadHandler::PresetsToString(buf, sizeof(buf), &lightCfg));
#endif //VERBOSE_SERIAL
  light.Init('L', &lightCfg, temperature, pMachineConfig->InitiallyOffTime);

  SingleLoadConfigModelFromCompact(&fanCfg, &pMachineConfig->fan);
#ifdef VERBOSE_SERIAL
  Serial.print("Load Fan Config: ");
  Serial.println(SingleLoadHandler::PresetsToString(buf, sizeof(buf), &fanCfg));
#endif //VERBOSE_SERIAL
  fan.Init('F', &fanCfg, temperature, pMachineConfig->InitiallyOffTime);

#ifdef VERBOSE_SERIAL
  SerialPrintMillis();
  Serial.println(F("Init() before UpdateStatus()"));
#endif //VERBOSE_SERIAL

  // set relays now:
  m_bUpdateStatus = true;
  UpdateStatus();

  // timer not started here, user will have to call:
  // cFan.Update(0, 0);
  //
  // start counting
  //if (fan.IsActive() || light.IsActive())
  //  startTimer();
}

char *cFanMachine::GetDescription()
{
  return this->Description;
}

bool cFanMachine::GetStatusFan(cFanStatus *pFanStatus, unsigned long *pFanTimer)
{
  return fan.GetStatus(pFanStatus, pFanTimer);
}
bool cFanMachine::GetStatusLight(cFanStatus *pLightStatus, unsigned long *pLightTimer)
{
  return light.GetStatus(pLightStatus, pLightTimer);
}

/*
void cFanMachine::Set(cFanStatus fanStatus, unsigned long fanTimer,
  cFanStatus lightStatus, unsigned long lightTimer)
{
#ifdef VERBOSE_SERIAL
  Serial.print("cFanMachine::Set() => Fan ");
  Serial.print(cFanStatusToString(fanStatus));
  Serial.print(" (timer");
  Serial.print(fanTimer);
  Serial.println("')");
  Serial.print("cFanMachine::Set() => Light ");
  Serial.print(cFanStatusToString(fanStatus));
  Serial.print(" (timer");
  Serial.print(lightTimer);
  Serial.println("')");
#endif //VERBOSE_SERIAL
  fan.Set(fanStatus, fanTimer);
  light.Set(lightStatus, lightTimer);

  if (fan.IsActive() || light.IsActive())
  {
    startTimer();
    // output status will be updated within MACHINE_STATUS_REFRESH_FREQ_SEC seconds
  }
  else
  {
    // both Fan and Light OFF,
    // forcing a quick OFF:
    stopTimer();
    m_bUpdateStatus = true;
    UpdateStatus();
    // a different approach is to do nothing here and
    // let timer be stopped at the next Ticker call
  }
}
*/

void cFanMachine::Update(unsigned long fanTimer, signed long lightTimer)
{
#ifdef VERBOSE_SERIAL
  char buf[15];

  SerialPrintMillis();
  Serial.print(F("cFanMachine::Update() => Light "));
  //Serial.print(cFanStatusToString(fanStatus));
  Serial.print(F(" (timer "));
  Serial.print(SingleLoadHandler::TimerToString(buf, sizeof(buf), lightTimer));
  Serial.print(F("), Fan "));
  //Serial.print(cFanStatusToString(fanStatus));
  Serial.print(F(" (timer "));
  Serial.print(SingleLoadHandler::TimerToString(buf, sizeof(buf), fanTimer));
  Serial.println(")");
#endif //VERBOSE_SERIAL
  //light.Set(cFanStatus::NoChange, lightTimer);
  if (lightTimer != NOCHANGE_TIMER)
    light.Update(lightTimer);
  //fan.Set(cFanStatus::NoChange, fanTimer);
  if (fanTimer != NOCHANGE_TIMER)
    fan.Update(fanTimer);

  if (fan.IsActive() || light.IsActive())
  {
    startTimer();
    // output status will be updated every MACHINE_STATUS_REFRESH_FREQ_SEC seconds
  }
  else
  {
    // both Fan and Light OFF,
    // forcing a quick OFF:
    stopTimer();
    m_bUpdateStatus = true;
    UpdateStatus();
    // a different approach is to do nothing here and
    // let timer be stopped at the next Ticker call
  }
}

void cFanMachine::GetPreSets(SingleLoadConfigModel *pFan, SingleLoadConfigModel *pLight)
{
  if (pFan)
    *pFan = *fan.GetPreSets();

  if (pLight)
    *pLight = *light.GetPreSets();
}

void cFanMachine::AsyncUpdateStatus()
{
  // this routine is async-called:
  // better not calling library functions
  // from here, may lead to unexpected results
  // if not carefully pounded.
  // Hence NO log output here:
  //Serial.println("AsyncUpdateStatus()");
  if (!fan.IsActive() && !light.IsActive())
  {
    stopTimer(); // On cycle completed, disable timer
    return;
  }

  fan.AsyncUpdate();
  light.AsyncUpdate();
  m_bUpdateStatus = true;
}

void cFanMachine::UpdateStatus()
{
  if (!m_bUpdateStatus) return;
  m_bUpdateStatus = false;
  // relays must be set ON/OFF
  OutSetAction_e rFanCurrent, rLightCurrent;
  machineIOs.GetRelays(rFanCurrent, rLightCurrent);
#ifdef VERBOSE_SERIAL
  SerialPrintMillis();
  Serial.println("cFanMachine::UpdateStatus() Relays current status:");
  Serial.println("- Fan  =" + String(OutSetActionToString(rFanCurrent)));
  Serial.println("- Light=" + String(OutSetActionToString(rLightCurrent)));
#endif //VERBOSE_SERIAL

  OutSetAction_e rFanNext, rLightNext;
  rFanNext = (fan.IsLoadOn()) ? OutSetAction_e::On : OutSetAction_e::Off;
  rLightNext = (light.IsLoadOn()) ? OutSetAction_e::On : OutSetAction_e::Off;
#ifdef VERBOSE_SERIAL
  Serial.println("cFanMachine::UpdateStatus() Relays next status:");
  Serial.println("- Fan  =" + String(OutSetActionToString(rFanNext)));
  Serial.println("- Light=" + String(OutSetActionToString(rLightNext)));
#endif //VERBOSE_SERIAL

  if (rFanNext == rFanCurrent)
    rFanNext = OutSetAction_e::noChange;
  else
    Serial.println("Relays: Fan " + String(OutSetActionToString(rFanCurrent)) + " => " + String(OutSetActionToString(rFanNext)));

  if (rLightNext == rLightCurrent)
    rLightNext = OutSetAction_e::noChange;
  else
    Serial.println("Relays: Light " + String(OutSetActionToString(rLightCurrent)) + " => " + String(OutSetActionToString(rLightNext)));

  machineIOs.SetRelays(rFanNext, rLightNext);
}

bool cFanMachine::IsInASafeStatus()
{
  return fan.IsInASafeStatus() && light.IsInASafeStatus();
}

bool cFanMachine::IsActiveFan()
{
  return fan.IsActive();
}

bool cFanMachine::IsActiveLight()
{
  return light.IsActive();
}



// -- time functions - begin
Ticker ticker;

// create a pure function, required by Ticker class.
// This implementation prevents from creating more than one instance
// of MachineCycle class. An error is issued via Serial if any.
// Think this is not a problem and could eventually be extended
// creating an array of class-instance-pointer & function.
cFanMachine *pClassInstancePointer = NULL;
void pureFnTimerCallback()
{
  // warning this is an async call and may interfere with other libraries,
  // so NO libraries call if possible and as quick as possible
  if (!pClassInstancePointer) return;
  pClassInstancePointer->AsyncUpdateStatus();
  // NO yield():
  //yield();
}

void cFanMachine::initTimer()
{
  pClassInstancePointer = NULL;
  //#ifdef VERBOSE_SERIAL
  //  // better not running Serial...: this method is called before setup()
  //  Serial.println("initTimer()");
  //#endif //VERBOSE_SERIAL
}

void cFanMachine::startTimer()
{
#ifdef VERBOSE_SERIAL
  Serial.println("startTimer()");
  Serial.flush();
#endif //VERBOSE_SERIAL
  if (pClassInstancePointer)
  {
    // already running
    //Serial.println("Error cannot start a new timer, too many instances of cFanStatus class running");
    //Serial.flush();
    return;
  }
  pClassInstancePointer = this;
  //os_timer_setfn(&myTimer, timerCallback, NULL);
  //os_timer_arm(&myTimer, 1000, 1); // fire event every second

 //pTimerCallback = &MachineCycle::InternalUpdateStatus;
 //pTimerCallbackF = (callback_t)&MachineCycle::InternalUpdateStatus;
 //(*this.*pTimerCallback)();
 //(this->*pTimerCallback)();
 //pTimerCallbackF();
  ticker.attach_ms(StatusRefreshFreqSec * 1000, pureFnTimerCallback); // fire event every <StatusRefreshFreqSec> seconds
}

void cFanMachine::stopTimer()
{
  //os_timer_disarm(&myTimer);
  ticker.detach();
  pClassInstancePointer = NULL;
#ifdef VERBOSE_SERIAL
  Serial.println("stopTimer()");
#endif //VERBOSE_SERIAL
}
// -- time functions - end
