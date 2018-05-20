/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

http page handler, hardware test

Sources repository: https://github.com/microentropie/
*/

#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <DateTime.h>

//#include "temperature_processing.h"
#include "machine_bl.h"
#include "common.h"
#include "debugUtl.h"
#include "settings.h"
#include "machine_io.h"
#include "HttpUtils.h"
#include "enumsToString.h"

extern ESP8266WebServer webServer;
extern baseConnectionConfig cfg;
extern MachineIo machineIOs;

#include "WebServer.h"
#include "WebServerUtils.h"



void hwtestHandler()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    return forbiddenHandler(); // configuration modifications only possible if locally connected
  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();
  if (!IsMachineInASafeStatus())
    return handleConfigurationNotAllowed();

  Serial.println("hwtestHandler():");
  logDetails();
  OutSetAction_e fanRelayVal, lightRelayVal;

  fanRelayVal = OutSetAction_e::noChange;
  lightRelayVal = OutSetAction_e::noChange;

  bool bSet = false;
  if (webServer.args() > 0)
  {
    for (int i = 0; i < webServer.args(); i++)
    {
      if (webServer.argName(i) == String("exit"))
      {
        // exit now, but first switch off loads:
        machineIOs.SetRelays(OutSetAction_e::Off, OutSetAction_e::Off);
        // redirect to /config
        RedirectTo("config");
        Serial.println(F(" Safe exit hwtest"));//debug
        return;
      }
      else if (webServer.argName(i) == String("update"))
      {
        bSet = (webServer.arg(i) == String("SET"));
      }
      else if (webServer.argName(i) == String("fanRelay"))
      {
        fanRelayVal = (webServer.arg(i) == String("ON")) ? OutSetAction_e::On : OutSetAction_e::Off;
      }
      else if (webServer.argName(i) == String("lightRelay"))
      {
        lightRelayVal = (webServer.arg(i) == String("ON")) ? OutSetAction_e::On : OutSetAction_e::Off;
      }
    }
  }
  if (bSet)
  {
#ifdef VERBOSE_SERIAL
    Serial.print("SetRelays: Fan=>");
    Serial.print(OutSetActionToString(fanRelayVal));
    Serial.print(", Light=>");
    Serial.println(OutSetActionToString(lightRelayVal));
#endif //VERBOSE_SERIAL
    machineIOs.SetRelays(fanRelayVal, lightRelayVal);
  }

  // read values needed by form:
  machineIOs.GetRelays(fanRelayVal, lightRelayVal);
  String temperature = TemperatureToString(machineIOs.GetTemperature(), machineIOs.GetTemperatureUnits());

  // create http page:
  SendHeaderAndHead(webServer, -1, "HW TEST");
  char s[1500];
  snprintf_P(s, sizeof(s), PSTR(
    "<body bgcolor='#%02X%02X%02X'>"
    "<h2>WARNING this page gives direct control to hardware I/Os !<br>"
    "this may result in damages to the machine, persons and the environment</h2>"
    "<b><a href='/config'>Go back now</a> if not sure on how to proceed.</b><br>"
    "<h3>Actions</h3>"
    //"<br>"
    "<form method='post' action='/hwtest'>"
    "<table>"
    "<tr><td>Fan Relay</td><td><input type='radio' name='fanRelay' value='ON' %s>set ON&nbsp;<input type='radio' name='fanRelay' value='OFF' %s>set OFF</td></tr>"
    "<tr><td>Light Relay</td><td><input type='radio' name='lightRelay' value='ON' %s>set ON&nbsp;<input type='radio' name='lightRelay' value='OFF' %s>set OFF</td></tr>"
    //"<tr><td>Temperature</td><td>adc=%d (%s &deg;C)</td></tr>"
    "<tr><td>Temperature</td><td>%s</td></tr>"
    "</table>"
    "<br>"
    "<a href='/hwtest'>click to refresh</a>"
    "<br><br>"
    "<input type='submit' name='update' value='SET'>&nbsp;&nbsp;"
    "<input type='submit' name='exit' value='exit' title='set I/Os in a safe state then exit (up 1 level)'>"
    "</form>"
    "%s"
    "</body>"
    "</html>"
    ), 0xAD, 0xD8, 0xE6, // form background color
    (fanRelayVal == OutSetAction_e::On) ? "checked" : "", (fanRelayVal == OutSetAction_e::On) ? "" : "checked",
    (lightRelayVal == OutSetAction_e::On) ? "checked" : "", (lightRelayVal == OutSetAction_e::On) ? "" : "checked",
    temperature.c_str(),
    COPYRIGHT);
#ifdef VERBOSE_SERIAL
  Serial.println(s);
#endif //VERBOSE_SERIAL
  webServer.sendContent(s);
  webServer.sendContent("");
  webServer.client().stop(); // Stop is needed because we sent no content length
}

