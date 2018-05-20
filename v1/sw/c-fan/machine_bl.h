#ifndef _MACHINE_BL_H_
#define _MACHINE_BL_H_

void MachineInit();
void MachineStop();
void MachineUpdateStatus();
bool IsMachineInASafeStatus();
void MachineForceUpdateSensorsRead();
bool MachineCommand(String loadType, String cmd);

#include "cFan_model.h"
bool FanConfig_Save(MachineConfigModel &cfg);
bool FanConfig_Load(MachineConfigModel &cfg);

#endif //_MACHINE_BL_H_
