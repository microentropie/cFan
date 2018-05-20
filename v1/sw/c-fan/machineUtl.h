#ifndef _MACHINE_UTL_H_
#define _MACHINE_UTL_H_

#include "cFanConfig.h"

void SingleLoadConfigModelFromCompact(SingleLoadConfigModel *pCfg, SingleLoadConfigModel_compact *pCfgCompact);
int cFanModeMainStrings(cFanModeMain mode, const char **ppDescriptionShort, const char **ppDescriptionLong);
int cFanModeTempStrings(cFanModeTemp mode, const char **ppDescriptionShort, const char **ppDescriptionLong);
int TemperatureUnitsStrings(temperatureUnits_e te, const char **ppDescriptionShort, const char **ppDescriptionLong);

#endif //_MACHINE_UTL_H_
