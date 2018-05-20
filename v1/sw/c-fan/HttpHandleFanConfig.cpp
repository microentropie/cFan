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
#include "cFanMachine.h"
#include "machine_bl.h"
#include "machineUtl.h"
//#include "enumsToString.h"

//extern MachineCycle machine;
//extern MachineIo machineIOs;
extern WiFiMode wifiMode;

extern ESP8266WebServer webServer;

extern baseConnectionConfig cfg;

#include "WebServer.h"
#include "WebServerUtils.h"
#include "HttpUtils.h"

static String errMsg;


static void SingleLoadConfig(const char *loadName, SingleLoadConfigModel_compact *pLoadValues)
{
  String minutes = F("' (1h = 60', 1 day = 1440', 1 week = 10080')");
  String minutesMinMax = F("0 .. 10080");

  // cFanModeMain:
  const char *pBrief, *pLong, *pBriefDefault, *pLongDefault;
  if (cFanModeMainStrings(pLoadValues->EnMain, &pBriefDefault, &pLongDefault) < 0)
  {
    // invalid value, set a safe default
    pLoadValues->EnMain = cFanModeMain::Disabled;
    cFanModeMainStrings(pLoadValues->EnMain, &pBriefDefault, &pLongDefault);
  }
  webServer.sendContent(String() +
    F("<table style='background-color:#e0e0e0; padding:3px;'>"
      "<tr><th colspan='3' align='left'>") + loadName + F(":</th></tr>"
      "<tr style='background-color:cornsilk;'><td>Main Mode</td><td><select name='") + loadName + F("ModeMain'>"));
  int i;
  for (i = 0; i <= 127; ++i) // scan for all possible values
  {
    if (cFanModeMainStrings((cFanModeMain)i, &pBrief, &pLong) < 0)
      continue; // values are not contigue
    String r = String() + "<option value='" + String(i) + "' title='" + String(pLong) + "'";
    if (i == (int)pLoadValues->EnMain)
      r += " selected='selected'";
    r += ">" + String(pBrief) + "</option>";
    webServer.sendContent(r);
  }
  webServer.sendContent(String() + F("</select></td><td>") + String(pBriefDefault) + ": " + String(pLongDefault) + "</td></tr>");
  webServer.sendContent(String() + F("<tr style='background-color:cornsilk;'><td>Timer</td><td><input type='text' "/*placeholder='" + minutesMinMax + "' */"name='") + loadName + F("timer' value='") + String(pLoadValues->timerMain) + F("' title='") + minutesMinMax + F("'/></td><td>") + minutes + F("</td></tr>"));

  webServer.sendContent(String() + "<tr style='background-color:yellow;'><td>[*] Temperature set point</td><td><input type='text' "/*placeholder='toggle temperature' */"name='" + loadName + "temp' value='" + String(pLoadValues->temperatureToggleValue / 10) + "'/></td><td>in [1] units, decimals discarded</td></tr>");

  webServer.sendContent(String() + "<tr style='background-color:#ffbfa0;'><td>Temperature &gt;= [*], MODE</td><td><select name='" + loadName + "ModeTempH'>");
  if (cFanModeTempStrings(pLoadValues->EnTempH, &pBriefDefault, &pLongDefault) < 0)
  {
    // invalid value, set a safe default
    pLoadValues->EnTempH = cFanModeTemp::Off;
    cFanModeTempStrings(pLoadValues->EnTempH, &pBriefDefault, &pLongDefault);
  }
  for (i = 0; i <= 127; ++i) // scan for all possible values
  {
    if (cFanModeTempStrings((cFanModeTemp)i, &pBrief, &pLong) < 0)
      continue; // values are not contigue
    String r = String() + "<option value='" + String(i) + "' title='" + String(pLong) + "'";
    if (i == (int)pLoadValues->EnTempH)
      r += " selected='selected'";
    r += ">" + String(pBrief) + "</option>";
    webServer.sendContent(r);
  }
  webServer.sendContent(String() + F("</select></td><td>") + String(pBriefDefault) + ": " + String(pLongDefault) + "</td></tr>");
  webServer.sendContent(String() + "<tr style='background-color:#ffbfa0;'><td>Temperature &gt;= [*], TIMER</td><td><input type='text' "/*placeholder='" + minutesMinMax + "' */"name='" + loadName + "timerH' value='" + String(pLoadValues->timerTempH) + "' title='" + minutesMinMax + "'/></td><td>" + minutes + "</td></tr>");

  webServer.sendContent(String() + "<tr style='background-color:lightskyblue;'><td>Temperature &lt; [*], MODE</td><td><select name='" + loadName + "ModeTempL'>");
  if (cFanModeTempStrings(pLoadValues->EnTempL, &pBriefDefault, &pLongDefault) < 0)
  {
    // invalid value, set a safe default
    pLoadValues->EnTempL = cFanModeTemp::Off;
    cFanModeTempStrings(pLoadValues->EnTempL, &pBriefDefault, &pLongDefault);
  }
  for (i = 0; i <= 127; ++i) // scan for all possible values
  {
    if (cFanModeTempStrings((cFanModeTemp)i, &pBrief, &pLong) < 0)
      continue; // values are not contigue
    String r = String() + "<option value='" + String(i) + "' title='" + String(pLong) + "'";
    if (i == (int)pLoadValues->EnTempL)
      r += " selected='selected'";
    r += ">" + String(pBrief) + "</option>";
    webServer.sendContent(r);
  }
  webServer.sendContent(String() + F("</select></td><td>") + String(pBriefDefault) + ": " + String(pLongDefault) + "</td></tr>");
  webServer.sendContent(String() + "<tr style='background-color:lightskyblue;'><td>Temperature &lt; [*], TIMER</td><td><input type='text' "/*placeholder='" + minutesMinMax + "' */"name='" + loadName + "timerL' value='" + String(pLoadValues->timerTempL) + "' title='" + minutesMinMax + "'/></td><td>" + minutes + "</td></tr>" +
    "</table>");
}

void handleConfigFan()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    forbiddenHandler(); // configuration modifications only possible if locally connected
  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();
  if (!IsMachineInASafeStatus())
    return handleConfigurationNotAllowed();

  Serial.println("handleFanConfig():");
  logDetails();

  MachineConfigModel saveCfg;
  FanConfig_Load(saveCfg);

  SendHeaderAndHead(webServer, -1, PublicName(NULL));
  webServer.sendContent(
    "<body>"
    "<h2>Configuration</h2>"
  );

  if (errMsg != NULL && errMsg.length() > 0)
    webServer.sendContent(String("<p style='color:white; background-color:red;'>") + errMsg + String("</p>"));

  webServer.sendContent(String() + "\n<br /><form method='POST' action='configfansave'>");

  webServer.sendContent(String() + "<table style='background-color:#e0e0e0; padding:3px;'><tr><td>Description</td><td><input type='text' "/*placeholder='Room name or other description' */"name='descr' value='" + String(saveCfg.Description) + "'/></td><td>Room name or other description</td></tr>");

  webServer.sendContent(String() + "<tr style='background-color:yellow;'><td>[1] Temperature units</td><td><select name='TempScale'>");
  const char *pTempBriefDefault, *pTempLongDefault;
  if (TemperatureUnitsStrings(saveCfg.temperatureScale, &pTempBriefDefault, &pTempLongDefault) < 0)
  {
    // invalid value, set a safe default
    saveCfg.temperatureScale = temperatureUnits_e::Celsius;
    TemperatureUnitsStrings(saveCfg.temperatureScale, &pTempBriefDefault, &pTempLongDefault);
  }
  const char *pTempBrief, *pTempLong;
  int i;
  for (i = 0; i <= 15; ++i) // scan for all possible values
  {
    if (TemperatureUnitsStrings((temperatureUnits_e)i, &pTempBrief, &pTempLong) < 0)
      break;
    String r = String() + "<option value='" + String(i) + "' title='" + String(pTempBrief) + "'";
    if (i == (int)saveCfg.temperatureScale)
      r += " selected='selected'";
    r += ">" + String(pTempLong) + "</option>";
    webServer.sendContent(r);
  }
  webServer.sendContent(String() + F("</select></td><td>") + String(pTempLongDefault) + ": " + String(pTempBriefDefault) + "</td></tr>");

  webServer.sendContent(String() + F("<tr><td>Module blue LED enabled</td><td><input type='checkbox' name='LedOn' value='1' ") + String(saveCfg.bConnectionLedShow ? "checked" : "") + F("></td><td>Shows health status</td></tr>"));
  webServer.sendContent(String() + F("<tr><td>Initially Off Time</td><td><input type='text' "/*placeholder='" + 2 + "' */"name='InitiallyOff' value='") + String(saveCfg.InitiallyOffTime) + F("' title='number of minutes'/></td><td>minute(s) 1 .. 10</td></tr>") +
    "</table><br />");
  SingleLoadConfig("fan", &saveCfg.fan);
  webServer.sendContent("<br />");
  SingleLoadConfig("light", &saveCfg.light);
  webServer.sendContent(
    "<br />"
    "<p><FONT style='color:orange'><b>Will need to Save, then reboot for the changes to take effect !</b></FONT></p>"
    "<br />"
    "<a href='/config'>Cancel</a>&nbsp;&nbsp;&nbsp;"
    "<input type='submit' name='Save' value='Save'/>&nbsp;&nbsp;&nbsp;"
    //"<input type='submit' name='Default' value='Load Defaults'/>"
    "</form>\n"
  );
  webServer.sendContent(COPYRIGHT);
  webServer.sendContent("</body></html>");
  webServer.sendContent("");
  webServer.client().stop(); // Stop is needed because we sent no content length

  errMsg = String("");
}

void handleConfigFanSave()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    forbiddenHandler(); // configuration modifications only possible if locally connected

  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();

  Serial.println("handleConfigFanSave():");
  logDetails();
  if (webServer.args() < 18 || webServer.args() > 19)
  {
    Serial.print("Bad number of args; args=");
    Serial.println(webServer.args());
    badRequestHandler();
    return;
  }
  errMsg = String("");
  /*
   Number of query properties: 16-17
   - descr = ?
   - fanModeMain = 0
   - fantimer = 10
   - fantemp = 0
   - fanModeTempH = 0
   - fantimerH = 10
   - fanModeTempL = 0
   - fantimerL = 10
   - lightModeMain = 1
   - lighttimer = 10
   - lighttemp = 0
   - lightModeTempH = 1
   - lighttimerH = 10
   - lightModeTempL = 1
   - lighttimerL = 10
   - LedOn = 1 <=== checkbox
   - Save = Save
   */
   /*
     if (webServer.hasArg("Defaults"))
     {
       // reset parameters to the default ones
     }
     */
  if (webServer.hasArg("Save"))
  {
    MachineConfigModel saveCfg;
    //FanConfig_Load(saveCfg);

    webServer.arg("descr").toCharArray(saveCfg.Description, sizeof(saveCfg.Description) - 1);
    saveCfg.temperatureScale = temperatureUnits_e::Celsius; //TODO
    // fan:
    String prefix = String("fan");
    saveCfg.fan.EnMain = (cFanModeMain)webServer.arg(prefix + "ModeMain").toInt();
    saveCfg.fan.timerMain = webServer.arg(prefix + "timer").toInt();
    saveCfg.fan.temperatureToggleValue = webServer.arg(prefix + "temp").toInt() * 10;
    saveCfg.fan.EnTempH = (cFanModeTemp)webServer.arg(prefix + "ModeTempH").toInt();
    saveCfg.fan.timerTempH = webServer.arg(prefix + "timerH").toInt();
    saveCfg.fan.EnTempL = (cFanModeTemp)webServer.arg(prefix + "ModeTempL").toInt();
    saveCfg.fan.timerTempL = webServer.arg(prefix + "timerL").toInt();
    // light:
    prefix = String("light");
    saveCfg.light.EnMain = (cFanModeMain)webServer.arg(prefix + "ModeMain").toInt();
    saveCfg.light.timerMain = webServer.arg(prefix + "timer").toInt();
    saveCfg.light.temperatureToggleValue = webServer.arg(prefix + "temp").toInt() * 10;
    saveCfg.light.EnTempH = (cFanModeTemp)webServer.arg(prefix + "ModeTempH").toInt();
    saveCfg.light.timerTempH = webServer.arg(prefix + "timerH").toInt();
    saveCfg.light.EnTempL = (cFanModeTemp)webServer.arg(prefix + "ModeTempL").toInt();
    saveCfg.light.timerTempL = webServer.arg(prefix + "timerL").toInt();

    saveCfg.temperatureScale = (temperatureUnits_e)webServer.arg("TempScale").toInt();
    saveCfg.bConnectionLedShow = webServer.hasArg("LedOn");
    saveCfg.InitiallyOffTime = webServer.arg("InitiallyOff").toInt();
    saveCfg.dummy = 0;

    if (saveCfg.InitiallyOffTime < 1 || saveCfg.InitiallyOffTime > 10)
      errMsg = String("invalid InitiallyOffTime");
    if (errMsg.length() == 0)
      FanConfig_Save(saveCfg);

    // redirect to /configfan
    RedirectTo("configfan");
    //Serial.print(F("  debuginfo (saveCfg) wlan name: "));//debug
    //Serial.println(saveCfg.ssid);
  }
}

