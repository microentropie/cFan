/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

(rice) Cooking Machine (http://www.microentropie.com)
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

#include "text\Ary_h_FaviconIco.cpp"
static const char hdrAttrib[] PROGMEM = "image/x-icon";

void handleFavicon()
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
   Serial.print("sending ");
   Serial.print(STRLEN_);
   Serial.print(" bytes - ");
   Serial.println(FPSTR(hdrAttrib));
#endif //VERBOSE_SERIAL
  webServer.setContentLength(SIZEOF_);
  webServer.send_P(200, hdrAttrib, "");
  webServer.client().write_P(ico, SIZEOF_); 
  //webServer.send_P(200, hdrAttrib, ico);
  //webServer.client().write_P(ico, SIZEOF_);
  //webServer.sendContent("");
  //webServer.client().stop();
}


