/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>
#include "cFanMachine.h"
#include "machine_bl.h"
#include "common.h"



// semaphore remains locked for the time of copying from nextAjaxText to currentAjaxText
#define MAX_AJAX_TEXT_LEN 60
static bool lockSemaphore;
static unsigned long latestAjaxUpdate;
static char CurrentMachineStatusAjaxText[MAX_AJAX_TEXT_LEN];
static char NextMachineStatusAjaxText[MAX_AJAX_TEXT_LEN];
static char MachineConfiguration[200];

#define MAX_IR_ROWS 4
static String *IrAjaxText[MAX_IR_ROWS];
static String NextIrAjaxText;

static bool AjaxLock()
{
   noInterrupts();
   if (lockSemaphore)
   {
      interrupts();
      return false;
   }
   lockSemaphore = true;
   interrupts();
   return true;
}
static void AjaxUnLock()
{
   noInterrupts();
   lockSemaphore = false;
   interrupts();
}

void AjaxInit()
{
   // init semaphore
   lockSemaphore = false;

   // init machine configuration
   MachineConfiguration[0] = '\0';

   // init machine status
   latestAjaxUpdate = 0;
   CurrentMachineStatusAjaxText[0] = '\0';
   NextMachineStatusAjaxText[0] = '\0';

   // init Ir
   for (int i = 0; i < MAX_IR_ROWS; ++i)
      IrAjaxText[i] = new String();
   NextIrAjaxText = "";
}

void SetAjaxIr(String str)
{
   if(str.length() <= 0) return; // nothing to do
   bool locked = false;
   for (int milliSec = millis(); !locked && millis() - milliSec < 600;)
   {
      locked = AjaxLock();
      yield();
   }
   if (!locked)
      return; // unable to reserve, this text will be lost
   int i;
   for (i = 0; i < MAX_IR_ROWS; ++i)
   {
      if ((*IrAjaxText[i]).length() <= 0)
         break;
   }
   if (i >= MAX_IR_ROWS)
   {
       // rotate pointers
      String *p = IrAjaxText[0];
      for (i = 1; i < MAX_IR_ROWS; ++i)
         IrAjaxText[i - 1] = IrAjaxText[i];
      IrAjaxText[MAX_IR_ROWS - 1] = p;
      i = MAX_IR_ROWS - 1;
   }
   *(IrAjaxText[i]) = str;
   AjaxUnLock();
}

String GetAjaxIr()
{
   if (AjaxLock())
   {
      NextIrAjaxText = "";
      // check if there is something to send
      int len = 0;
      for (int i = 0; i < MAX_IR_ROWS; ++i)
      {
        len += (*IrAjaxText[i]).length();
        if(len > 0) break;
      }
      if(len > 0)
      {
        // yes, some code to send
        for (int i = 0; i < MAX_IR_ROWS; ++i)
           NextIrAjaxText += *IrAjaxText[i] + "\r\n";
      }
      AjaxUnLock();
   }
   return NextIrAjaxText;
}


//---
char *GetAjaxMachineStatus()
{
   if (millis() - latestAjaxUpdate > /*HTTP_PAGE_UPDATE_FREQ_SEC **/ 1000)
   {
      // the status has not been internally updated for a long time,
      // force update now
      MachineForceUpdateSensorsRead();
   }

   if (AjaxLock())
      memcpy(CurrentMachineStatusAjaxText, NextMachineStatusAjaxText, MAX_AJAX_TEXT_LEN);
   AjaxUnLock();
   return CurrentMachineStatusAjaxText;
}


void PrepareAjaxText_cFan(char *dtNow,
                          int fanStatus, String strFanStatus, String strFanTimer,
                          int lightStatus, String strLightStatus, String strLightTimer,
                          String temperat)
{
   bool locked = AjaxLock();
   /*
   for(int i = 0; !locked && i < 10; ++i)
   {
     yield();
     locked = AjaxLock();
   }
   */
   if (!locked)
      return;
#define FieldSeparatorChar ','
#define QtParameters 8
   snprintf_P(NextMachineStatusAjaxText, MAX_AJAX_TEXT_LEN,
              PSTR("%c%d%c%s%c%d%c%s%c%s%c%d%c%s%c%s%c%s"), FieldSeparatorChar, QtParameters, FieldSeparatorChar,
              dtNow, FieldSeparatorChar,
              fanStatus, FieldSeparatorChar,
              strFanStatus.c_str(), FieldSeparatorChar,
              strFanTimer.c_str(), FieldSeparatorChar,
              lightStatus, FieldSeparatorChar,
              strLightStatus.c_str(), FieldSeparatorChar,
              strLightTimer.c_str(), FieldSeparatorChar,
              temperat.c_str());
#undef QtParameters
#undef FieldSeparatorChar

   latestAjaxUpdate = millis();
   AjaxUnLock();

#ifdef VERBOSE_SERIAL
   SerialPrintMillis();
   Serial.print(F("new Ajax text: '"));
   Serial.print(NextAjaxText);
   Serial.println(F("'"));
#endif //VERBOSE_SERIAL
}


//---
char *GetAjaxMachineConfig()
{
  // this variable is written before the webService is active
  return MachineConfiguration;
}

void PrepareAjaxText_MachineConfiguration(
       char *machineId, char *machineDescription,
       bool bFanEnabled, String strFanConfig,
       bool bLightEnabled, String strLightConfig,
       int year)
{
#define FieldSeparatorChar '~'
#define QtParameters 7
  snprintf_P(MachineConfiguration, sizeof(MachineConfiguration),
             PSTR("%c%d%c%s%c%s%c%d%c%s%c%d%c%s%c%d"), FieldSeparatorChar, QtParameters, FieldSeparatorChar,
             machineId, FieldSeparatorChar,
             machineDescription, FieldSeparatorChar,
             bFanEnabled? 1 : 0, FieldSeparatorChar,
             strFanConfig.c_str(), FieldSeparatorChar,
             bLightEnabled? 1 : 0, FieldSeparatorChar,
             strLightConfig.c_str(), FieldSeparatorChar,
             year);
#undef QtParameters
#undef FieldSeparatorChar
}
