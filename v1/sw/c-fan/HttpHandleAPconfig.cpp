/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

http page handler, configuration

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <TimeZone.h>

#include "machine_bl.h"
#include "common.h"
#include "debugUtl.h"
#include "settings.h"
#include "bl.h"
#include "WebServerUtils.h"

extern WiFiMode wifiMode;

extern ESP8266WebServer webServer;
extern baseConnectionConfig cfg;


#include "WebServer.h"
#include "HttpUtils.h"
#include "WiFi.h"
//extern void disconnectWiFi();

static String httpErrMsg;


void handleConfigRoot()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    return forbiddenHandler(); // configuration modifications only possible if locally connected
  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();
  if (!IsMachineInASafeStatus())
    return handleConfigurationNotAllowed();

  Serial.print(FPSTR(__func__));
  Serial.println(FPSTR("()"));
  logDetails();

  String s = PrepareHead(-1, PublicName(NULL));
  //SendHeaderAndHead(webServer, -1, PublicName(NULL));
  s += String() +
    F("<body>"
      "<h2>") +
    String(PublicName(NULL)) +
    F(" config</h2>");
  if (webServer.client().localIP() == cfg.ApIp)
    s += String() + F("<p>Connected through the soft AP: ") + String(cfg.hostName) + F("</p>");
  else
    s += String() + F("<p>Connected through the wifi network: ") + String(cfg.ssid) + F("</p>");
  s += F("<a href='/'>Home</a><br><br>");
  s += F("<a href='/info'>System info</a><br>");
  s += F("<a href='/configwifi'>config the wifi connection</a><br>");
  //s += F("<a href='/configt'>edit temperature setPoints</a><br>");
  s += F("<a href='/configlocaliz'>config localization</a><br>");
  s += F("<a href='/configntp'>config NTP</a><br>");
  s += F("<a href='/configfan'>config Fan</a><br>");
  s += F("<a href='/configir'>config I.R.</a><br>");
  s += F("<br>");
  s += F("<i><a href='/hwtest'>hardware test</a><br><br>");
  s += F("<a href='/update'>firmware update</a><br><br>");
  s += F("<a href='/ShutDown' title='useful for serial/usb update (will reconnect WiFi to same address)'>shutdown</a></i><br>");
  s += COPYRIGHT;
  s += F("</body></html>");

#ifdef VERBOSE_SERIAL
  Serial.println(s);
#endif //VERBOSE_SERIAL

  SendHtmlPage(webServer, -1, s);
}

void handleConfigWifi()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    return forbiddenHandler(); // configuration modifications only possible if locally connected
  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();
  if (!IsMachineInASafeStatus())
    return handleConfigurationNotAllowed();

  Serial.print(FPSTR(__func__));
  Serial.println(FPSTR("():"));
  logDetails();

  SendHeaderAndHead(webServer, -1, PublicName(NULL));
  webServer.sendContent(F(
    "<body>"
    "<h2>Cooking Machine config</h2>"
  ));
  webServer.sendContent(String() + F("\r\n<br /><form method='POST' action='configwifisave'>"));
  webServer.sendContent(String() + F("<table><tr><th colspan='2' align='left'>Access Point configuration</th></tr>"));
  webServer.sendContent(String() + F("<tr><td>SSID</td><td>") + String(cfg.hostName) + F("</td></tr>"));
  webServer.sendContent(String() + F("<tr><td>Password</td><td><input type='password' "/*placeholder='tool tip' */"name='app' value='") + String(cfg.ApPassword) + F("'/></td></tr>"));
  webServer.sendContent(String() + F("<tr><td>IP</td><td>"));
  for (int i = 0; i < 4; ++i)
  {
    if (i > 0)
      webServer.sendContent(String("."));
    webServer.sendContent(String() + F("<input type='text' "/*placeholder='tool tip' */"name='apip") + String(i) + "' size='3' value='" + String(cfg.ApIp[i]) + "'/>");
  }
  webServer.sendContent(F(
    "</td></tr>"
    "</table>"
    "\r\n<br />"
    "<table><tr><th colspan='2' align='left'>User Access Control</th></tr>"
  ));
  webServer.sendContent(String() + F("<tr><td>User</td><td><input type='text' "/*placeholder='tool tip' */"name='un' value='") + String(cfg.userName) + "'/></td></tr>");
  webServer.sendContent(String() + F("<tr><td>Password</td><td><input type='password' "/*placeholder='tool tip' */"name='up' value='") + String(cfg.userPassword) + "'/></td></tr>");
  webServer.sendContent(String() + F("<tr><td>Group</td><td><input type='text' "/*placeholder='tool tip' */"name='ug' value='") + String("Administrator") + "'/></td></tr>");

  webServer.sendContent(F(
    "</table>"
    "\r\n<br />"
    "<table><tr><th colspan='2' align='left'>Wireless LAN connect to</th></tr>"
  ));
  webServer.sendContent(String() + F("<tr><td>SSID</td><td><input type='text' "/*placeholder='tool tip' */"name='wn' value='") + String(cfg.ssid) + "'/></td></tr>");
  webServer.sendContent(String() + F("<tr><td>Password</td><td><input type='password' "/*placeholder='tool tip' */"name='wp' value='") + String(cfg.password) + "'/></td></tr>");
  // static ip ?
  webServer.sendContent(String() + F("<tr><td colspan='2'>Get dynamic IP from DHCP server <input type='checkbox' name='IpDhcp' value='fromap' ") + String(cfg.bUseStaticIp ? "" : "checked") + "></td></tr>");
  webServer.sendContent(String() + F("<tr><td>IP address</td><td>"));
  for (int i = 0; i < 4; ++i)
  {
    if (i > 0)
      webServer.sendContent(String("."));
    webServer.sendContent(String() + F("<input type='text' "/*placeholder='tool tip' */"name='wip") + String(i) + F("' size='2' value='") + String(cfg.ip[i]) + "'/>");
  }
  // gateway
  webServer.sendContent(String() + F("</td></tr><tr><td>Gateway</td><td>"));
  for (int i = 0; i < 4; ++i)
  {
    if (i > 0)
      webServer.sendContent(String("."));
    webServer.sendContent(String() + F("<input type='text' "/*placeholder='tool tip' */"name='wg") + String(i) + F("' size='2' value='") + String(cfg.gateway[i]) + "'/>");
  }
  // mask
  webServer.sendContent(String() + F("</td></tr><tr><td>Subnet Mask</td><td>"));
  for (int i = 0; i < 4; ++i)
  {
    if (i > 0)
      webServer.sendContent(String("."));
    webServer.sendContent(String() + F("<input type='text' "/*placeholder='tool tip' */"name='wm") + String(i) + F("' size='2' value='") + String(cfg.subnet[i]) + "'/>");
  }
  webServer.sendContent(F(
    "</td></tr>"
    "</table>"
    "\r\n<br />"
    "<table><tr><th colspan='6' align='left'>WLAN list (<a href='javascript:history.go(0)'>refresh</a>)</th></tr>"
  ));
  webServer.sendContent(String() + F("\r\n<tr><td>BSSID</td><td><b>SSID</b></td><td>hidden</td><td>encripted</td><td>ch.</td><td><i>signal strength</i></td></tr>"));
  Serial.println(F("scan start"));
  int n = WiFi.scanNetworks();
  // per migliorare la lista: https://github.com/esp8266/Arduino/issues/1843
  Serial.println(F("scan done"));
  if (n > 0 && n < 20)
  {
    for (int i = 0; i < n; i++)
      webServer.sendContent(String() + "\r\n<tr><td>" + WiFi.BSSIDstr(i) + "</td><td>" + WiFi.SSID(i) + "</td><td>" + String(WiFi.isHidden(i) ? "yes" : "no") + "</td><td>" + String((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "no" : "yes") + "</td><td>" + WiFi.channel(i) + "</td><td>" + WiFi.RSSI(i) + "</td></tr>");
  }
  else
  {
    webServer.sendContent(String() + F("<tr colspan='6'><td>No WLAN found</td></tr>"));
  }

  webServer.sendContent(F(
    "</table>"
    "<p>Will need to reboot for the changes to take effect !</p>"
  ));
  Serial.println(F("scan sent to http"));

  if (httpErrMsg.length() > 0)
  {
    webServer.sendContent(String() +
      F("<p style='color:white; background-color:red; font-size:110%;'>") + httpErrMsg + "</p>"
    );
  }
  webServer.sendContent(F(
    "<br /><a href='/config'>Cancel</a>&nbsp;&nbsp;&nbsp;<input type='submit' value='Save'/></form>"
    "<p>restart the machine <a href='/config-restart-now'>NOW</a></p>"
    "</body></html>"
  ));
  webServer.sendContent("");
  webServer.client().stop(); // Stop is needed because we sent no content length
}

/* Handle the WLAN save form and redirect to WLAN config page again */
void handleConfigWifiSave()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    return forbiddenHandler(); // configuration modifications only possible if locally connected
  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();
  if (!IsMachineInASafeStatus())
    return handleConfigurationNotAllowed();

  Serial.print(FPSTR(__func__));
  Serial.println(FPSTR("():"));
  logDetails();
  if (webServer.args() != 22 && webServer.args() != 23)
  {
    Serial.print(F("Bad number of args; args="));
    Serial.println(webServer.args());
    badRequestHandler();
    return;
  }

  baseConnectionConfig saveCfg;
  // Access Point configuration:
  strncpy(saveCfg.hostName, PublicName(NULL), sizeof(saveCfg.hostName)); // buffer padded with \0s
  webServer.arg("app").toCharArray(saveCfg.ApPassword, sizeof(saveCfg.ApPassword) - 1);
  for (int i = 0; i < 4; ++i)
    saveCfg.ApIp[i] = webServer.arg("apip" + String(i)).toInt();
  // User Access Control:
  webServer.arg("un").toCharArray(saveCfg.userName, sizeof(saveCfg.userName) - 1);
  webServer.arg("up").toCharArray(saveCfg.userPassword, sizeof(saveCfg.userPassword) - 1);
  // Wireless Lan configuration:
  webServer.arg("wn").toCharArray(saveCfg.ssid, sizeof(saveCfg.ssid) - 1);
  webServer.arg("wp").toCharArray(saveCfg.password, sizeof(saveCfg.password) - 1);
  saveCfg.bUseStaticIp = !webServer.hasArg("IpDhcp");
  for (int i = 0; i < 4; ++i)
    saveCfg.ip[i] = webServer.arg("wip" + String(i)).toInt();
  for (int i = 0; i < 4; ++i)
    saveCfg.gateway[i] = webServer.arg("wg" + String(i)).toInt();
  for (int i = 0; i < 4; ++i)
    saveCfg.subnet[i] = webServer.arg("wm" + String(i)).toInt();
  Serial.print("WiFi IP address: ");
  for (int i = 0; i < 4; ++i)
  {
    if (i > 0)
      Serial.print(".");
    Serial.print(String(saveCfg.ip[i]));
  }
  Serial.println();

  if (saveCfg.ApIp[3] < 1 || saveCfg.ip[3] < 1 || saveCfg.gateway[3] < 1)
  {
    httpErrMsg = String(F("!! Invalid IP address specified, cannot save. Changes discarded !!"));
    Serial.println(httpErrMsg);
  }
  else
  {
    httpErrMsg = String("");
    baseConnectionConfig_Save(saveCfg);
    cfg = saveCfg;
  }

  // redirect to /configwifi
  RedirectTo("configwifi");
  Serial.print(F("  debuginfo (saveCfg) wlan name: "));//debug
  Serial.println(saveCfg.ssid);
}

void handleConfigRestart()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    return forbiddenHandler(); // configuration modifications only possible if locally connected
  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();
  if (!IsMachineInASafeStatus())
    return handleConfigurationNotAllowed();

  Serial.print(FPSTR(__func__));
  Serial.println(FPSTR("():"));
  logDetails();

  RedirectTo("config");

  Serial.println(F("restarting ..."));
  Serial.flush();

  Restart();
}

void handleShutDown()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    return forbiddenHandler(); // configuration modifications only possible if locally connected
  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();
  if (!IsMachineInASafeStatus())
    return handleConfigurationNotAllowed();

  Serial.print(FPSTR(__func__));
  Serial.println(FPSTR("():"));
  logDetails();
  SendHeaderAndHead(webServer, -1, PublicName(NULL));

  webServer.sendContent(
    F(
    "<body>"
    "WiFi disconnection in progress<br>"
    "Update firmware via serial/usb<br>"
    "<br>"
    "Web server will not respond !<br>"
    "Machine was shutdown<br>"
    "</body>"
    "</html>")
  );
  webServer.sendContent("");
  webServer.client().stop(); // Stop is needed because we sent no content length
  for (int i = 0; i < 10; ++i)
  {
    loopWebServer();
    loopWiFi();
    delay(100);
  }
  Serial.println("ShutDown completed.");
  Serial.flush();

  ShutDown();

  for (;;)
    delay(10); // now die
}

void handleConfigLocalization()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    return forbiddenHandler(); // configuration modifications only possible if locally connected
  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();
  if (!IsMachineInASafeStatus())
    return handleConfigurationNotAllowed();

  Serial.print(FPSTR(__func__));
  Serial.println(FPSTR("():"));
  logDetails();

  struct TimeAndDateInfo lcl = GetLocalization();

  String ttl = String(PublicName(NULL)) + " Locales";
  SendHeaderAndHead(webServer, -1, (char *)ttl.c_str());
  webServer.sendContent(String() +
    "<body>" +
    "<h2>" + String(ttl) + "</h2>"
  );
  webServer.sendContent(F(
    "\r\n<br /><form method='POST' action='configlocalizsave'>"
    "<table>"
  ));
  webServer.sendContent(String() + F("<tr><td>Time difference</td><td><input type='text' title='local time - UTC' name='TimeZone' value='") + String(lcl.timeZone) + F("' size='4'/></td><td>hours from UTC (decimals ok, f.i. 3.5 = +3h30')</td></tr>"));

  // DST:
  const char *pBrief, *pLong, *pBriefDefault, *pLongDefault;
  if (DstCriteriaStrings(lcl.dst, &pBriefDefault, &pLongDefault) < 0)
  {
    lcl.dst = noDST;
    DstCriteriaStrings(lcl.dst, &pBriefDefault, &pLongDefault);
  }
  webServer.sendContent(String() + F("<tr><td>Daylight Savings Time</td><td><select name='DstCriteria'>"));
  int i;
  for (i = 0; i < 50; ++i) // scan for all possible DST
  {
    if (DstCriteriaStrings((dstCriteria)i, &pBrief, &pLong) < 0)
      break;
    String r = String() + "<option value='" + String(i) + "' title='" + String(pLong) + "'";
    if (i == lcl.dst)
      r += " selected='selected'";
    r += ">" + String(pBrief) + "</option>";
    webServer.sendContent(r);
  }
  webServer.sendContent(String() + F("</select></td><td>") + String(pBriefDefault) + ": " + String(pLongDefault) + "</td></tr>");

  webServer.sendContent(String() + F("<tr><td>Date Separator character</td><td><input type='text' title='Date Separator character (f.i. /)' name='dateSeparator' value='") + String(lcl.dateSeparator) + F("' size='1'/></td><td></td></tr>"));

  // Date format:
  if (DateFormatStrings(lcl.dateFormat, &pBriefDefault, &pLongDefault) < 0)
  {
    lcl.dateFormat = 32;
    DateFormatStrings(lcl.dateFormat, &pBriefDefault, &pLongDefault);
  }
  webServer.sendContent(String() + F("<tr><td>Date print format</td><td><select name='DateFormat'>"));
  for (i = 0; i < 255; ++i) // scan for all possible Date formats
  {
    if (DateFormatStrings(i, &pBrief, &pLong) < 0)
      continue;
    String r = String() + F("<option value='") + String(i) + F("' title='") + String(pLong) + "'";
    if (i == lcl.dateFormat)
      r += " selected='selected'";
    r += ">" + String(pBrief) + "</option>";
    webServer.sendContent(r);
  }
  webServer.sendContent(String() + F("</select></td><td>") + String(pBriefDefault) + F(": ") + String(pLongDefault) + F("</td></tr>"));

  webServer.sendContent(String() + F("<tr><td>Time Separator character</td><td><input type='text' title='Time Separator character (f.i. :)' name='timeSeparator' value='") + String(lcl.timeSeparator) + F("' size='1'/></td><td></td></tr>"));

  // Time format:
  if (TimeFormatStrings(lcl.timeFormat, &pBriefDefault, &pLongDefault) < 0)
  {
    lcl.timeFormat = 0;
    TimeFormatStrings(lcl.timeFormat, &pBriefDefault, &pLongDefault);
  }
  webServer.sendContent(String() + F("<tr><td>Time print format</td><td><select name='TimeFormat'>"));
  for (i = 0; i < 255; ++i) // scan for all possible Time formats
  {
    if (TimeFormatStrings(i, &pBrief, &pLong) < 0)
      continue;
    String r = String() + F("<option value='") + String(i) + F("' title='") + String(pLong) + "'";
    if (i == lcl.timeFormat)
      r += " selected='selected'";
    r += ">" + String(pBrief) + "</option>";
    webServer.sendContent(r);
  }
  webServer.sendContent(String() + F("</select></td><td>") + String(pBriefDefault) + ": " + String(pLongDefault) + F("</td></tr>"));
  webServer.sendContent(F(
    "</table>\r\n"
    "<br /><a href='/config'>Cancel</a>&nbsp;&nbsp;&nbsp;<input type='submit' value='Save'/></form>"
  ));
  webServer.sendContent(COPYRIGHT);
  webServer.sendContent(F("</body></html>"));
  webServer.sendContent("");
  webServer.client().stop(); // Stop is needed because we sent no content length
}

void handleConfigLocalizationSave()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    return forbiddenHandler(); // configuration modifications only possible if locally connected
  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();
  if (!IsMachineInASafeStatus())
    return handleConfigurationNotAllowed();

  Serial.print(FPSTR(__func__));
  Serial.println(FPSTR("():"));
  logDetails();
  if (webServer.args() != 6)
  {
    Serial.print(F("Bad number of args; args="));
    Serial.println(webServer.args());
    badRequestHandler();
    return;
  }

  struct TimeAndDateInfo lcl;
  Localization_Load(lcl);
  //struct TimeAndDateInfo lcl = GetLocalization();

  lcl.timeZone = webServer.arg("TimeZone").toFloat();
  lcl.dst = (dstCriteria)webServer.arg("DstCriteria").toInt();
  lcl.dateSeparator = webServer.arg("dateSeparator")[0];
  lcl.dateFormat = webServer.arg("DateFormat").toInt();
  lcl.timeSeparator = webServer.arg("timeSeparator")[0];
  lcl.timeFormat = webServer.arg("TimeFormat").toInt();

  // redirect to /configlocaliz
  RedirectTo("configlocaliz");
  Serial.println(F("  debuginfo (saveLocalization)"));//debug

#ifdef VERBOSE_SERIAL
  Serial.print(F("lcl.dateFormat = ")); Serial.println(lcl.dateFormat);
  Serial.print(F("lcl.timeFormat = ")); Serial.println(lcl.timeFormat);
#endif //VERBOSE_SERIAL

  Localization_Save(lcl);
  SetLocalization(lcl);
}



void handleConfigNtp()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    return forbiddenHandler(); // configuration modifications only possible if locally connected
  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();
  if (!IsMachineInASafeStatus())
    return handleConfigurationNotAllowed();

  Serial.print(FPSTR(__func__));
  Serial.println(FPSTR("():"));
  logDetails();

  struct ntpConfig ntpc;
  NtpConfig_Load(ntpc);

  String ttl = String(PublicName(NULL)) + " NTP";
  SendHeaderAndHead(webServer, -1, (char *)ttl.c_str());
  webServer.sendContent(String() +
    F("<body>"
    "<h2>") + String(ttl) + F("</h2>")
  );
  webServer.sendContent(F(
    "\r\n<br /><form method='POST' action='configntpsave'>"
  ));
  // Server:
  webServer.sendContent(String() + F("<table>"));
  webServer.sendContent(String() + F("<tr><td>Server1&nbsp;</td><td><input type='text' name='Server1' value='") + String(ntpc.Server1) + "'/></td></tr>");
  webServer.sendContent(String() + F("<tr><td>Server2&nbsp;</td><td><input type='text' name='Server2' value='") + String(ntpc.Server2) + "'/></td></tr>");
  webServer.sendContent(String() + F("<tr><td>Server3&nbsp;</td><td><input type='text' name='Server3' value='") + String(ntpc.Server3) + "'/></td></tr>");
  webServer.sendContent(String() + F("</table>"));
  webServer.sendContent(F(
    "<br /><a href='/config'>Cancel</a>&nbsp;&nbsp;&nbsp;<input type='submit' value='Save'/></form>"
  ));

  webServer.sendContent(F("<br>Check <a href='http://www.pool.ntp.org/zone/@' target='_blank' title='pool.ntp.org'>this site</a> for a detailed and updated list,<br>"));
  webServer.sendContent(F("there are time servers for near every country.<br>"));
  webServer.sendContent(F("Then click on the link of your area and get your nearest server.<br>"));

  webServer.sendContent(F("<br>Here is a small list of generic good time servers:<br>"));
  // trick to hold strings in FLASH
  String b = String(F("*&nbsp;"));
  String e = String(F("<br>"));
  webServer.sendContent(b + F("pool.ntp.org") + e);
  webServer.sendContent(b + F("africa.pool.ntp.org") + e);
  webServer.sendContent(b + F("asia.pool.ntp.org") + e);
  webServer.sendContent(b + F("europe.pool.ntp.org") + e);
  webServer.sendContent(b + F("north-america.pool.ntp.org") + e);
  webServer.sendContent(b + F("oceania.pool.ntp.org") + e);
  webServer.sendContent(b + F("south-america.pool.ntp.org") + e);
  webServer.sendContent(b + F("us.pool.ntp.org") + e);
  webServer.sendContent(b + F("time.nist.gov") + e);
  webServer.sendContent(COPYRIGHT);
  webServer.sendContent(F("</body></html>"));
  webServer.sendContent("");
  webServer.client().stop(); // Stop is needed because we sent no content length
}

void handleConfigNtpSave()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    return forbiddenHandler(); // configuration modifications only possible if locally connected
  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();
  if (!IsMachineInASafeStatus())
    return handleConfigurationNotAllowed();

  Serial.print(FPSTR(__func__));
  Serial.println(FPSTR("():"));
  logDetails();
  if (webServer.args() != 3)
  {
    Serial.print(F("Bad number of args; args="));
    Serial.println(webServer.args());
    badRequestHandler();
    return;
  }

  struct ntpConfig ntpc;
  NtpConfig_Load(ntpc);

  strncpy(ntpc.Server1, webServer.arg("Server1").c_str(), sizeof(ntpc.Server1)); // extra space filled with 0s
  strncpy(ntpc.Server2, webServer.arg("Server2").c_str(), sizeof(ntpc.Server2)); // extra space filled with 0s
  strncpy(ntpc.Server3, webServer.arg("Server3").c_str(), sizeof(ntpc.Server3)); // extra space filled with 0s
  ntpc.updatePeriodSeconds = 0;

  // redirect to /configntp
  RedirectTo("configntp");
  Serial.println(F("  debuginfo (saveNtpConfig)"));//debug

  NtpConfig_Save(ntpc);
}
