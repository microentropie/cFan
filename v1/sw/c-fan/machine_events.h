#ifndef _MACHINE_EVENTS__H_
#define _MACHINE_EVENTS__H_

//class MachineCycle;
//void MachineStatusUpdateCallback(MachineCycle *pMachine);
class cFanMachine;
extern void MachineStatusUpdateCallback(cFanMachine *pMachine);
extern void MachineInitConfig(cFanMachine *pMachine);

#endif //_MACHINE_EVENTS__H_
