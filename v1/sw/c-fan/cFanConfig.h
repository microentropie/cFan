#ifndef _CFAN_CONFIG_H_
#define _CFAN_CONFIG_H_

// used for normal operation:
enum class cFanStatus // values range 0 .. 127 (7 bit). DO NOT USE negative values.
{
  Off = 0,
  On = 1,
  InitiallyOffThenOn = 2, // first minute(s) load is off, then on
  // ...
  NoChange = 125,
  // TempDependent = 126, is unusable here
  Disabled = 127, // this load (fan or light) is not present (i/o pin is left as input)
};

struct SingleLoadStatus
{
  cFanStatus mode;
  signed long currentTimer;
  unsigned long loadTimer;
};

// Used to store default value (temperature dependent):
enum class cFanModeTemp // values range 0 .. 15 (4 bit). DO NOT USE negative values.
{
  Off = 0,
  On = 1,
  InitiallyOffThenOn = 2,
  // ...
};

// Used to store default value:
enum class cFanModeMain // values range 0 .. 127 (7 bit). DO NOT USE negative values.
{
  Off = 0,
  On = 1,
  InitiallyOffThenOn = 2, // first minute load is off, then on
  // ...
  // NoChange = 125, is unusable here
  TempDependent = 126, // behaviour depends on temperature
  Disabled = 127, // this load (fan or light) is not present
};

// the default configuration:
struct SingleLoadConfigModel
{
  cFanModeMain EnMain;
  cFanModeTemp EnTempH;
  cFanModeTemp EnTempL;

  unsigned short temperatureToggleValue;

  unsigned short timerMain;
  unsigned short timerTempH;
  unsigned short timerTempL;

  //SetDefaults();
};

enum temperatureUnits_e
{
  Celsius,
  Kelvin,
  Fahrenheit,
  Reaumur,
  Newton,
  Rankine,
};

// the default configuration in a compact form (stored in EEPROM):
struct SingleLoadConfigModel_compact
{
  cFanModeMain EnMain : 8;
  cFanModeTemp EnTempH : 4;
  cFanModeTemp EnTempL : 4;
  unsigned short temperatureToggleValue; // °C or °F, x 10

  unsigned short timerMain;
  unsigned short timerTempH;
  unsigned short timerTempL;
};

#define DescriptionBufferSize 20
// saved in EEPROM:
struct MachineConfigModel
{
  char Description[DescriptionBufferSize];
  SingleLoadConfigModel_compact fan;
  SingleLoadConfigModel_compact light;
  bool bConnectionLedShow : 1;
  temperatureUnits_e temperatureScale : 3;
  int dummy : 4;// align to 8 bit
  unsigned char InitiallyOffTime; // minutes
};
#endif //_CFAN_CONFIG_H_
