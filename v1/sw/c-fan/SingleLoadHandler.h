#ifndef _SINGLE_LOAD_STATUS_H_
#define _SINGLE_LOAD_STATUS_H_

#include "cFanConfig.h"

#include "limits.h"
#define STOP_TIMER LONG_MAX
#define NOCHANGE_TIMER (LONG_MAX - 1)

struct SingleLoadHandler
{
private:
  // status variables:
  SingleLoadStatus currentStatus;

  SingleLoadConfigModel loadValues;

  char id;

public:
  SingleLoadHandler();
  // User methods:
  void Init(char id, SingleLoadConfigModel *pLoadValues, int temperature, int initiallyOffTimeMin);
  SingleLoadConfigModel *GetPreSets();
  bool GetStatus(cFanStatus *pStatus, unsigned long *pTimer);
  void Set(cFanStatus cStatus, unsigned long timer);
  void Update(signed long addTimer);

  bool IsInASafeStatus();
  bool IsLoadOn();
  bool IsActive();

  // Low level methods:
  void ZeroVariables();
  void AsyncUpdate();

  // utility functions:
  static char *PresetsToString(char *buf, int bufSize, SingleLoadConfigModel *pPresets);
  static char *TimerToString(char *buf, int bufSize, unsigned long timer);
  void SetStatusFromPresets(SingleLoadConfigModel *pLoadValues, int temperature);
  static const char *cFanStatusToString(cFanStatus cStatus);
private:
  //static char *cFanModeMainToString(char *buf, int bufSize, cFanModeMain mode);
  static const char *cFanModeTempToString(cFanModeTemp mode);
  int initiallyOffTimeSec;
};

#endif //_SINGLE_LOAD_STATUS_H_
