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
#include <DateTime.h>

#include "cFanMachine.h"
#include "settings.h"
#include "common.h"
#include "debugUtl.h"
#include "machine_io.h"
#include "enumsToString.h"
/*
#include "ui.h"
*/

extern cFanMachine cFan;

extern ESP8266WebServer webServer;
extern baseConnectionConfig cfg;



#include "WebServer.h"
#include "WebServerUtils.h"
#include "HttpUtils.h"

#include "text\Ary_h_cFanHtml.cpp"
static const char hdrAttrib[] PROGMEM = "text/html";
void cFanHandler()
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
  PrintStringCharByChar(Serial, FPSTR(html));
  Serial.println();
#else //VERBOSE_SERIAL == SystemAndMyLog
   Serial.print("sending ");
   Serial.print(STRLEN_);
   Serial.print(" bytes - ");
   Serial.println(FPSTR(hdrAttrib));
#endif //VERBOSE_SERIAL == SystemAndMyLog
#endif //VERBOSE_SERIAL
  webServer.setContentLength(STRLEN_);
  // cache header control, should avoid Ajax caching:
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  //webServer.sendHeader("Expires", "-1");
  webServer.send_P(200, hdrAttrib, html);
  //webServer.sendContent("");
  //webServer.client().stop();
}

void cFanHandlerDo()
{
  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
  {
    webServer.requestAuthentication();
    return;
  }
  Serial.print(FPSTR(__func__));
  Serial.println(FPSTR("()"));
  logDetails();

  if (webServer.hasArg("Global"))
  {
    Serial.println(String() + F("Global=") + webServer.arg("Global"));
    // go to shutdown page, but first switch off loads:
    cFan.Update(STOP_TIMER, STOP_TIMER);
    RedirectTo("/ShutDown");
    return;
  }
  Serial.println("???");

  // redirect to /
  RedirectTo("/");
}

