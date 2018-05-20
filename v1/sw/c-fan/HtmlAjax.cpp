/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

http page handler, cooking

Sources repository: https://github.com/microentropie/
*/

//#include <Arduino.h>
#include <ESP8266WebServer.h>
#include "debugUtl.h"
//#include "AjaxText.h"
extern char *GetAjaxMachineConfig();
extern char *GetAjaxMachineStatus();
extern String GetAjaxIr();
#include "WebServerUtils.h"
#include "WebServer.h"
#include "settings.h"
#include "machine_bl.h"

extern ESP8266WebServer webServer;
extern baseConnectionConfig cfg;

static const char hdrAttrib[] PROGMEM = "text/plain";

void handleAjaxCall()
{
   if (!webServer.authenticate(cfg.userName, cfg.userPassword))
   {
      webServer.requestAuthentication();
      return;
   }
   Serial.print(FPSTR(__func__));
   Serial.println(FPSTR("()"));
   logDetails();
   String q = String(F("q"));
   if (!webServer.hasArg(q))
   {
      badRequestHandler();
      return;
   }
   String question = webServer.arg(q);

   // get response data
   String txt;

   if (question == "ir")
      txt = GetAjaxIr();
   else if (question == "ms")
      txt = String(GetAjaxMachineStatus());
   else if (question == "mc")
      txt = String(GetAjaxMachineConfig());
   else if (question == "ut")
   {
      if(MachineCommand(webServer.arg("lt"), webServer.arg("cmd")))
        txt = String(GetAjaxMachineStatus());
      else
        txt = String() + F("  invalid parameter(s) lt='") + webServer.arg("lt") + F("', cmd='") + webServer.arg("cmd") + F("'");
   }
   else
   {
      txt = String() + F("  ?unknown request q='") + question + F("'");
      Serial.println(txt);
   }

   Serial.print("q:");
   Serial.print(question);
#ifdef VERBOSE_SERIAL
   Serial.print(" => '");
   Serial.print(txt);
   Serial.print("'");
#else //VERBOSE_SERIAL
   Serial.print(" ");
   Serial.print(txt.length());
   Serial.print(" bytes");
#endif //VERBOSE_SERIAL
   Serial.println();

   // send response data
   webServer.setContentLength(txt.length());
   // cache header control, should avoid Ajax caching:
   webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
   webServer.sendHeader("Pragma", "no-cache");
   webServer.sendHeader("Expires", "-1");
   webServer.send(200, FPSTR(hdrAttrib), txt);
   //webServer.sendContent("");
   //webServer.client().stop();
}
