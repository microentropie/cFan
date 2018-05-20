/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

EEPROM save interface

Sources repository: https://github.com/microentropie/
*/

#include <WString.h>
#include "settings.h"

bool baseConnectionConfigEEP::Save()
{
  return eepromIf<baseConnectionConfig>::Save((baseConnectionConfig *)this, EEPROM_log_RW);
}

bool baseConnectionConfigEEP::Load()
{
  return eepromIf<baseConnectionConfig>::Load((baseConnectionConfig *)this, EEPROM_log_RW);
}


//---------------------------------------------------------------- 
// initialize the signature for this type
template<> uint32_t eepromIf<baseConnectionConfig>::Signature = string2Int(FPSTR("CONN"));
template<> uint16_t eepromIf<baseConnectionConfig>::baseOffset = 0;

bool baseConnectionConfig_Save(baseConnectionConfig &cfg)
{
  return eepromIf<baseConnectionConfig>::Save(&cfg, EEPROM_log_RW);
}
bool baseConnectionConfig_Load(baseConnectionConfig &cfg)
{
  return eepromIf<baseConnectionConfig>::Load(&cfg, EEPROM_log_RW);
}


//----------------------------------------------------------------
// initialize the signature for this type
template<> uint32_t eepromIf<TimeAndDateInfo>::Signature = string2Int(FPSTR("LCLZ"));
template<> uint16_t eepromIf<TimeAndDateInfo>::baseOffset = 500;

bool Localization_Save(TimeAndDateInfo &lcl)
{
  return eepromIf<TimeAndDateInfo>::Save(&lcl, EEPROM_log_RW);
}
bool Localization_Load(TimeAndDateInfo &lcl)
{
  return eepromIf<TimeAndDateInfo>::Load(&lcl, EEPROM_log_RW);
}

//----------------------------------------------------------------
// initialize the signature for this type
template<> uint32_t eepromIf<ntpConfig>::Signature = string2Int(FPSTR("sNTP"));
template<> uint16_t eepromIf<ntpConfig>::baseOffset = 550;

bool NtpConfig_Save(ntpConfig &ntpc)
{
  return eepromIf<ntpConfig>::Save(&ntpc, EEPROM_log_RW);
}
bool NtpConfig_Load(ntpConfig &ntpc)
{
  return eepromIf<ntpConfig>::Load(&ntpc, EEPROM_log_RW);
}
