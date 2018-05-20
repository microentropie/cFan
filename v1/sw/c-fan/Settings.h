#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <IPAddress.h>
#include <serializeEeprom.h>


// configuration, saved to / retrieved from EEPROM

struct baseConnectionConfig
{
  // station connection params:
  char hostName[30]; // and AP ssid
  char ssid[30];
  char password[30];
  bool bUseStaticIp;
  IPAddress ip;
  IPAddress gateway;
  IPAddress subnet;
  //
  // User access control
  char userName[16];
  char userPassword[16];
  unsigned char userGroup;
  //
  // AP parameters:
  char ApPassword[30];
  IPAddress ApIp;
};



//---
bool baseConnectionConfig_Save(baseConnectionConfig &cfg);
bool baseConnectionConfig_Load(baseConnectionConfig &cfg);

class baseConnectionConfigEEP : public baseConnectionConfig
{
public:
  bool Save();
  bool Load();
};



//---
#include <TimeZone.h>

bool Localization_Save(TimeAndDateInfo &cs);
bool Localization_Load(TimeAndDateInfo &cs);



//---
#include "src/time/ntp_model.h"
bool NtpConfig_Save(ntpConfig &ntpc);
bool NtpConfig_Load(ntpConfig &ntpc);

#endif //_SETTINGS_H_
