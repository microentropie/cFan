/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>
#include <ESP8266WebServer.h>

#include "debugUtl.h"
#include "WebServerUtils.h"
#include "settings.h"
#include "commonString.h"

extern ESP8266WebServer webServer;
extern baseConnectionConfig cfg;


#include "text\Ary_h_Javascript.cpp"

static const char hdrAttrib[] PROGMEM = "text/javascript";

void handleJavascript()
{
   if (!webServer.authenticate(cfg.userName, cfg.userPassword))
   {
      webServer.requestAuthentication();
      return;
   }
   Serial.print(FPSTR(__func__));
   Serial.println(FPSTR("()"));
   logDetails();
#ifdef VERBOSE_SERIAL
#if VERBOSE_SERIAL == SystemAndMyLog
   PrintStringCharByChar(Serial, FPSTR(js));
   Serial.println();
#else //VERBOSE_SERIAL == SystemAndMyLog
   Serial.print("sending ");
   Serial.print(STRLEN_);
   Serial.print(" bytes - ");
   //Serial.println(FPSTR(mimeTable[type.js].mimeType));
   Serial.println(FPSTR(hdrAttrib));
#endif //VERBOSE_SERIAL == SystemAndMyLog
#endif //VERBOSE_SERIAL
   webServer.setContentLength(STRLEN_);
   webServer.send_P(200, hdrAttrib, js);
   webServer.sendContent("");
   webServer.client().stop();
}

