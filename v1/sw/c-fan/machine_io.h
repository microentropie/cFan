#ifndef _myMACHINE_IO_
#define _myMACHINE_IO_

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif //uint8_t

enum OutSetAction_e
{
  Off, On, noChange
};

#include "cFanConfig.h"

class OneWire;
class DallasTemperature;

class MachineIo
{
public:
  MachineIo();
  void Init(bool bLedsShow, temperatureUnits_e temperatureScale);
  void SetRelays(OutSetAction_e rFan, OutSetAction_e rLight);
  void GetRelays(OutSetAction_e &rFan, OutSetAction_e &rLight);
  void SetLeds(OutSetAction_e b);
  unsigned char WiFiReservationStatus();
  void ForceWiFiOn();
  bool AreLedsEnabled();
  int  GetTemperature();
  temperatureUnits_e GetTemperatureUnits();

private:
  OneWire *pOneWire;
  DallasTemperature *pSensors;
  temperatureUnits_e temperatureScale;
  unsigned long lastTemperatureReadin;
  int temperatureCachedValue;
  bool bLedsShow;
};
//#define outON 1
//#define outOFF (!outON)

extern MachineIo machineIOs;

#endif //_myMACHINE_IO_
