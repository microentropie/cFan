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

#include "text\Ary_h_Style.cpp"
/*
@font-face {
  font-family: 'Roboto';
  font-style: normal;
  font-weight: 400;
  src: local('Roboto'), local('Roboto-Regular'), url(https://fonts.gstatic.com/s/roboto/v18/KFOmCnqEu92Fr1Mu4mxK.woff2) format('woff2');
}
@font-face {
  font-family: 'Roboto';
  font-style: normal;
  font-weight: 500;
  src: local('Roboto Medium'), local('Roboto-Medium'), url(https://fonts.gstatic.com/s/roboto/v18/KFOlCnqEu92Fr1MmEU9fBBc4.woff2) format('woff2');
}
*/
static const char hdrAttrib[] PROGMEM = "text/css";

void handleCss()
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
   PrintStringCharByChar(Serial, FPSTR(css));
   Serial.println();
#else //VERBOSE_SERIAL == SystemAndMyLog
   Serial.print("sending ");
   Serial.print(STRLEN_);
   Serial.print(" bytes - ");
   Serial.println(FPSTR(hdrAttrib));
#endif //VERBOSE_SERIAL == SystemAndMyLog
#endif //VERBOSE_SERIAL
  webServer.setContentLength(STRLEN_);
  webServer.send_P(200, hdrAttrib, css);
  //webServer.client().write_P(css, STRLEN_);
  //webServer.sendContent("");
  //webServer.client().stop();
}
