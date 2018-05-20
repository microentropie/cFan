#ifndef _AJAX_H_
#define _AJAX_H_

void AjaxInit();
//char *GetAjaxMachineConfig();
//char *GetAjaxMachineStatus();
//String GetAjaxIr();
void PrepareAjaxText_cFan(char *dtNow,
                          int fanStatus, String strFanStatus, String strFanTimer,
                          int lightStatus, String strLightStatus, String strLightTimer,
                          String temperat);

void PrepareAjaxText_MachineConfiguration(
       char *machineId, char *machineDescription,
       bool bFanEnabled, String strFanConfig,
       bool bLightEnabled, String strLightConfig,
       int year);

#endif //_AJAX_H_
