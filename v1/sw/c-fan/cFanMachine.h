#ifndef _FAN_MACHINE_H_
#define _FAN_MACHINE_H_

//#include "cFanConfig.h"
#include "SingleLoadHandler.h"

#include "AjaxText.h"

// #define ON_LIGHT_MASK 2
// #define ON_FAN_MASK 1

#define MACHINE_STATUS_REFRESH_FREQ_SEC 1


class cFanMachine
{
protected:
  SingleLoadHandler fan;
  SingleLoadHandler light;

  char Description[DescriptionBufferSize];

private:
  void ZeroVariables();

public:
  cFanMachine();
  virtual ~cFanMachine();
  void Init(MachineConfigModel *pMachineConfig);
  bool GetStatusFan(cFanStatus *pFanStatus, unsigned long *pFanTimer);
  bool GetStatusLight(cFanStatus *pLightStatus, unsigned long *pLightTimer);
  //void Set(cFanStatus fanStatus, unsigned long fanTimer, cFanStatus lightStatus, unsigned long lightTimer);
  void Update(unsigned long fanTimer, signed long lightTimer);

  void GetPreSets(SingleLoadConfigModel *pFan, SingleLoadConfigModel *pLight);

  bool IsInASafeStatus();
  bool IsActiveFan();
  bool IsActiveLight();

  char *GetDescription();

  // timer interrupt functions:
protected:
  int StatusRefreshFreqSec = MACHINE_STATUS_REFRESH_FREQ_SEC;
private:
  void initTimer();
  void startTimer();
  void stopTimer();
  bool m_bUpdateStatus;

public:
  void UpdateStatus();
  void AsyncUpdateStatus();
};

#define HTTP_PAGE_UPDATE_FREQ_SEC 10

#endif //_FAN_MACHINE_H_
