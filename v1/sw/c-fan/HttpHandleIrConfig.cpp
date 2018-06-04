/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

Sources repository: https://github.com/microentropie/
*/

#include <ESP8266WebServer.h>

//#include "machine_io.h"
#include "common.h"
#include "settings.h"
//#include "cFanMachine.h"

#include "Ir_bl.h"
#include <IRutils.h>

#include "machine_bl.h"
//#include "machineUtl.h"
//#include "enumsToString.h"
#include "String64.h"

//extern MachineCycle machine;
//extern MachineIo machineIOs;
extern WiFiMode wifiMode;

extern ESP8266WebServer webServer;

extern baseConnectionConfig cfg;

#include "WebServer.h"
#include "WebServerUtils.h"
#include "HttpUtils.h"

static String errMsg;


static void SingleLoadConfig(const char *loadName, IrCommandsSingle *pLoadValues)
{
  webServer.sendContent(String() +
    "<table style='background-color:#e0e0e0; padding:3px;'>" +
    "<tr><th colspan='4' align='left'>" + loadName + ":</th></tr>" +
    "<tr><td>Switch On</td><td>&nbsp;0x</td><td><input type='text' "/*placeholder='uint64_t hex value' */"name='" + loadName + "On' value='" + String64(pLoadValues->SetOn, 16) + "'/></td><td>Remote command to switch ON, HEX value (0 if unused)</td></tr>" +
    "<tr><td>Switch Off</td><td>&nbsp;0x</td><td><input type='text' "/*placeholder='uint64_t hex value' */"name='" + loadName + "Off' value='" + String64(pLoadValues->SetOff, 16) + "'/></td><td>Remote command to switch OFF, HEX value (0 if unused)</td></tr>" +
    "<tr><td>Toggle</td><td>&nbsp;0x</td><td><input type='text' "/*placeholder='uint64_t hex value' */"name='" + loadName + "Toggle' value='" + String64(pLoadValues->ToggleOnOff, 16) + "'/></td><td>Remote command to toggle status ON/OFF, HEX value (0 if unused)</td></tr>" +
    "</table>");
}

void handleConfigIr()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    forbiddenHandler(); // configuration modifications only possible if locally connected
  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();
  if (!IsMachineInASafeStatus())
    return handleConfigurationNotAllowed();

   Serial.print(FPSTR(__func__));
   Serial.println(FPSTR("()"));
   logDetails();

  IrConfigModel saveCfg;
  IrConfig_Load(saveCfg);

  SendHeaderAndHead(webServer, -1, PublicName(NULL));
  webServer.sendContent(
    "<body onload=\"OnLoadIrDoc()\">"
    "<h2>InfraRed configuration</h2>"
  );

  if (errMsg != NULL && errMsg.length() > 0)
    webServer.sendContent(String("<p style='color:red'>") + errMsg + String("</p>"));

  webServer.sendContent(String() + "\n<br /><form method='POST' action='configirsave'>");

  webServer.sendContent(String() + "<table style='background-color:#e0e0e0; padding:3px;'>");
  webServer.sendContent(String() +
    "<tr><td>IR receiver</td><td><input type='checkbox' name='irOn' value='1' " + String(saveCfg.irOn ? "checked" : "") + "></td><td>enable</td><td>If set enables IR remote receiver</td></tr>" +
    "<tr><td>WiFi On</td><td>&nbsp;0x</td><td><input type='text' "/*placeholder='uint64_t hex value' */"name='WiFi' value='" + String64(saveCfg.WiFiForceOn, 16) + "'/></td><td>Remote command to force WiFi On, HEX value (0 if unused)</td></tr>" +
    "</table><br />");
  SingleLoadConfig("fan", &saveCfg.fan);
  webServer.sendContent("<br />");
  SingleLoadConfig("light", &saveCfg.light);

  if (!IsIrEnabled())
    webServer.sendContent(F(
      "<br>IR receiver is OFF, IR commands (codes) will not be received.<br>"
      "1) ENABLE IR receiver<br>"
      "2) Save<br>"
      "3) Restart"
    ));

  webServer.sendContent(F(
    "<br /><div>"
    "<textarea id='IrRxTxt' name='IrReceivedLog' cols='60' rows='5' readonly>received IR codes will go here</textarea>"
    "<button type='button' onclick='AjaxIrCodes()'>Check Now</button>"
    "</div><br />"
  ));
    
  webServer.sendContent(F(
    "<br />"
    "<p><FONT style='color:orange'><b>Will need to Save, then reboot for the changes to take effect !</b></FONT></p>"
    "<br />"
    "<a href='/config'>Cancel</a>&nbsp;&nbsp;&nbsp;"
    "<input type='submit' name='Save' value='Save'/>&nbsp;&nbsp;&nbsp;"
    //"<input type='submit' name='Default' value='Load Defaults'/>"
    "</form>\n"
    "<p>restart the machine <a href='/config-restart-now'>NOW</a></p>"
  ));
  webServer.sendContent(COPYRIGHT);
  webServer.sendContent(F("</body></html>"));
  webServer.sendContent(F("<noscript>Sorry, your browser does not support JavaScript!</noscript>"));
  webServer.sendContent("");
  webServer.client().stop(); // Stop is needed because we sent no content length
}

void handleConfigIrSave()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    forbiddenHandler(); // configuration modifications only possible if locally connected

  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();

  Serial.print(FPSTR(__func__));
  Serial.println(FPSTR("()"));
  logDetails();
  if (webServer.args() < 9 || webServer.args() > 10)
  {
    Serial.print("Bad number of args; args=");
    Serial.println(webServer.args());
    badRequestHandler();
    return;
  }
  errMsg = String("");
  /*
    if (webServer.hasArg("Defaults"))
    {
      // reset parameters to the default ones
    }
    */
  if (webServer.hasArg("Save"))
  {
    IrConfigModel saveCfg;
    //IrConfig_Load(saveCfg);

    // fan:
    String prefix = String("fan");
    saveCfg.fan.SetOff = HexStringToUint64(webServer.arg(prefix + "Off"));
    saveCfg.fan.SetOn = HexStringToUint64(webServer.arg(prefix + "On"));
    saveCfg.fan.ToggleOnOff = HexStringToUint64(webServer.arg(prefix + "Toggle"));
    // light:
    prefix = String("light");
    saveCfg.light.SetOff = HexStringToUint64(webServer.arg(prefix + "Off"));
    saveCfg.light.SetOn = HexStringToUint64(webServer.arg(prefix + "On"));
    saveCfg.light.ToggleOnOff = HexStringToUint64(webServer.arg(prefix + "Toggle"));

    saveCfg.irOn = webServer.hasArg("irOn");
    saveCfg.WiFiForceOn = HexStringToUint64(webServer.arg("WiFi"));
    saveCfg.dummy = 0;

    IrConfig_Save(saveCfg);

    // redirect to /configir
    RedirectTo("configir");
    //Serial.print(F("  debuginfo (saveCfg) wlan name: "));//debug
    //Serial.println(saveCfg.ssid);
  }
}

