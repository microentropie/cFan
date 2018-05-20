#ifndef _ENUMS_TOSTRING_
#define _ENUMS_TOSTRING_

#include <WString.h>
#include "cFanConfig.h"
#include "machine_io.h"

extern const char *OutSetActionToString(OutSetAction_e osStatus);
extern String TemperatureToString(int temp, temperatureUnits_e units);
extern const char *TemperatureUnitsNameToString(temperatureUnits_e te);
extern const char *TemperatureUnitsNotationToString(temperatureUnits_e te);

#endif //_ENUMS_TOSTRING_
