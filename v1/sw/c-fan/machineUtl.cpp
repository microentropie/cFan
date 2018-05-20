/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>
#include "cFanConfig.h"
#include "enumsToString.h"

void SingleLoadConfigModelFromCompact(SingleLoadConfigModel *pCfg, SingleLoadConfigModel_compact *pCfgCompact)
{
  if (!pCfg || !pCfgCompact) panic();

  pCfg->EnMain = pCfgCompact->EnMain;
  pCfg->EnTempH = pCfgCompact->EnTempH;
  pCfg->EnTempL = pCfgCompact->EnTempL;
  pCfg->temperatureToggleValue = pCfgCompact->temperatureToggleValue;
  pCfg->timerMain = pCfgCompact->timerMain;
  pCfg->timerTempH = pCfgCompact->timerTempH;
  pCfg->timerTempL = pCfgCompact->timerTempL;
}

#ifndef NULL
#define NULL 0
#endif //NULL

int cFanModeMainStrings(cFanModeMain mode, const char **ppDescriptionShort, const char **ppDescriptionLong)
{
  if (ppDescriptionShort == NULL)
    return -2;
  switch (mode)
  {
  case cFanModeMain::Disabled:
    *ppDescriptionShort = "Disabled";
    *ppDescriptionLong = "Disabled";
    break;
  case cFanModeMain::Off:
    *ppDescriptionShort = "Off";
    *ppDescriptionLong = "Load always Off";
    break;
  case cFanModeMain::On:
    *ppDescriptionShort = "On";
    *ppDescriptionLong = "timer controlled load";
    break;
  case cFanModeMain::InitiallyOffThenOn:
    *ppDescriptionShort = "InitiallyOffThenOn";
    *ppDescriptionLong = "initially Off then On";
    break;
  case cFanModeMain::TempDependent:
    *ppDescriptionShort = "TempDependent";
    *ppDescriptionLong = "Temperature dependent";
    break;
  default:
    *ppDescriptionShort = NULL;
    *ppDescriptionLong = NULL;
    mode = (cFanModeMain)-1; // return an invalid value, existing enums are all >= 0)
    break;
  }
  return (int)mode;
}

int cFanModeTempStrings(cFanModeTemp mode, const char **ppDescriptionShort, const char **ppDescriptionLong)
{
  switch (mode)
  {
  case cFanModeTemp::Off:
  case cFanModeTemp::On:
  case cFanModeTemp::InitiallyOffThenOn:
    return cFanModeMainStrings((cFanModeMain)mode, ppDescriptionShort, ppDescriptionLong);
  default:
    *ppDescriptionShort = NULL;
    *ppDescriptionLong = NULL;
    mode = (cFanModeTemp)-1; // return an invalid value, existing enums are all >= 0)
    break;
  }
  return (int)mode;
}


int TemperatureUnitsStrings(temperatureUnits_e te, const char **ppDescriptionShort, const char **ppDescriptionLong)
{
  const char *resLong = TemperatureUnitsNameToString(te);
  if (*resLong == '?')
  {
    *ppDescriptionShort = NULL;
    *ppDescriptionLong = NULL;
    te = (temperatureUnits_e)-1; // return an invalid value, existing enums are all >= 0)
  }
  else
  {
    *ppDescriptionShort = TemperatureUnitsNotationToString(te);
    *ppDescriptionLong = resLong;
  }
  return (int)te;
}
