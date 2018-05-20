/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

web server

Sources repository: https://github.com/microentropie/
*/

#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#include "common.h"
#include "debugUtl.h"
#include "settings.h"
//#include "ui.h"
#include "machine_bl.h"
#include "HttpUtils.h"
#include "HtmlStyle.h"
#include "HtmlJavascript.h"
#include "HtmlFavicon.h"
#include "WebServerUtils.h"
#include "AjaxText.h"

// server authentication credentials:
static char ota_relativeUrl[20];// /*PROGMEM*/ = "/esp8266httpupdate"; //"/update";

extern WiFiMode wifiMode;

ESP8266WebServer webServer(80);
ESP8266HTTPUpdateServer httpUpdater;
extern baseConnectionConfig cfg;

extern void handleConfigRoot();
extern void handleConfigWifi();
extern void handleConfigWifiSave();
//extern void handleConfigNtc();
//extern void handleConfigNtcSave();
extern void handleConfigLocalization();
extern void handleConfigLocalizationSave();
extern void handleConfigNtp();
extern void handleConfigNtpSave();
extern void handleConfigFan();
extern void handleConfigFanSave();
extern void handleConfigIr();
extern void handleConfigIrSave();

extern void handleConfigRestart();
extern void handleShutDown();

extern void handleAjaxCall();

//
extern void cFanHandler();
extern void cFanHandlerDo();
//
extern void hwtestHandler();
//
extern void infoHandler();

String PrepareHead(int refreshSec, char *title)
{
  String head = String() + F("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\r\n"
    "<html lang='en'><head>\r\n");
  if (refreshSec > 0)
    head += String() + F("<meta http-equiv='refresh' content='") + String(refreshSec) + F("'>\r\n");
  head += String() + F(
    "<meta name='viewport' content='width=device-width, initial-scale=1'>\r\n"
    "<link rel='stylesheet' type='text/css' href='style.css'>\r\n"
    "<link rel='shortcut icon' type='image/x-icon' href='/favicon.ico'>\r\n"
    "<script type='text/javascript' src='cFan.js'></script>\r\n"
    //"<style type='text/css'>h2, h3{ padding-bottom: 0px; margin-bottom: 1px; }</style>\r\n"
    "<meta http-equiv='Content-Type' content='text/html;charset=ISO-8859-1'>\r\n" //<===== in some cases smartPhone Os gets fooled by UTF-8
    //"<link href='https://fonts.googleapis.com/css?family=Roboto:400,300,500' rel='stylesheet' type='text/css'>\r\n"
    "<title>") + String(title) + F("</title>\r\n"
      "</head>\r\n");

  return head;
}

void SendHeader(ESP8266WebServer &webServer, int refreshSec)
{
  // documentazione molto interessante sulla cache:
  // https://www.mnot.net/cache_docs/
  //webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  //webServer.sendHeader("Pragma", "no-cache");
  //webServer.sendHeader("Expires", "-1");
  //webServer.sendHeader("Connection", "close");
  if (refreshSec > 0)
    webServer.sendHeader("max-age", String(refreshSec));
}

void SendHeaderAndHead(ESP8266WebServer &webServer, int refreshSec, char *title)
{
  SendHeader(webServer, refreshSec);
  webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  webServer.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  webServer.sendContent(PrepareHead(refreshSec, title));
}

void SendHtmlPage(ESP8266WebServer &webServer, int refreshSec, String content)
{
  SendHeader(webServer, refreshSec);
  webServer.setContentLength(content.length());
  webServer.send(200, "text/html", content);
  webServer.sendContent("");
}

void notFoundHandler()
{
  Serial.println(F("notFoundHandler():"));
  webServer.send(404, F("text/plain"), String() + F("404 Not found\r\n") + webServer.uri());
  logDetails();
}


void forbiddenHandler()
{
  Serial.println(F("forbiddenHandler():"));
  webServer.send(403, F("text/plain"), F("403 Forbidden"));
  logDetails();
}



void badRequestHandler()
{
  Serial.println(F("badRequestHandler():"));
  webServer.send(400, F("text/plain"), F("400 Bad Request"));
  logDetails();
}


void handleConfigurationNotAllowed()
{
  Serial.println(F("handleConfigurationNotAllowed():"));
  logDetails();

  SendHeaderAndHead(webServer, -1, PublicName(NULL));
  webServer.sendContent(F(
    "<body>"
    "<h2>Cooking Machine config</h2>"
    "Cannot enter maintenance mode:<br>"
    "machine is currently working !<br><br>"
    "<a href='/'>Home</a>"
  ));
  webServer.sendContent(COPYRIGHT);
  webServer.sendContent(F("</body></html>"));
  webServer.sendContent("");
  webServer.client().stop(); // Stop is needed because we sent no content length
}

void RedirectTo(char *url)
{
  Serial.print(F("RedirectTo(): "));
  Serial.println(url);

  webServer.sendHeader(F("Location"), url, true);
  webServer.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
  webServer.sendHeader(F("Pragma"), F("no-cache"));
  webServer.sendHeader(F("Expires"), "-1");
  webServer.send(302, F("text/plain"), "");  // Empty content inhibits Content-length header so we have to close the socket ourselves.
  webServer.client().stop(); // Stop is needed because we sent no content length
}

void handleUpdate()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    return forbiddenHandler(); // update only possible if locally connected
  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();
  if (!IsMachineInASafeStatus())
    return handleConfigurationNotAllowed(); // machine must be idle

  Serial.println(F("handleUpdate():"));
  logDetails();

  // redirect to /esp8266httpupdate
  RedirectTo(ota_relativeUrl);
  //webServer.sendHeader(F("Location"), ota_relativeUrl, true);
  //webServer.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
  //webServer.sendHeader(F("Pragma"), F("no-cache"));
  //webServer.sendHeader(F("Expires"), F("-1"));
  //webServer.send(302, F("text/plain"), F(""));  // Empty content inhibits Content-length header so we have to close the socket ourselves.
  //webServer.client().stop(); // Stop is needed because we sent no content length
}


void setupWebServer()
{
#if defined(ARDUINO_ARCH_ESP32) 
  unsigned long randNumber = esp_random();
#elif defined(ARDUINO_ARCH_ESP8266)
  unsigned long randNumber = RANDOM_REG32; //ESP8266_DREG(0x20E44)
#else
  unsigned long randNumber = random();
#endif
  snprintf(ota_relativeUrl, sizeof(ota_relativeUrl), "/U%u", randNumber);

  webServer.on("/", cFanHandler);
  webServer.on("/do", cFanHandlerDo);
  webServer.on("/config", handleConfigRoot);
  webServer.on("/configwifi", handleConfigWifi);
  webServer.on("/configwifisave", handleConfigWifiSave);
  //webServer.on("/configt", handleConfigNtc);
  //webServer.on("/configtsave", handleConfigNtcSave);
  webServer.on("/configlocaliz", handleConfigLocalization);
  webServer.on("/configlocalizsave", handleConfigLocalizationSave);
  webServer.on("/configntp", handleConfigNtp);
  webServer.on("/configntpsave", handleConfigNtpSave);
  webServer.on("/configfan", handleConfigFan);
  webServer.on("/configfansave", handleConfigFanSave);
  webServer.on("/configir", handleConfigIr);
  webServer.on("/configirsave", handleConfigIrSave);
  webServer.on("/info", infoHandler);
  webServer.on("/hwtest", hwtestHandler);
  webServer.on("/update", handleUpdate);
  webServer.on("/config-restart-now", handleConfigRestart);
  webServer.on("/ShutDown", handleShutDown);
  webServer.on("/style.css", handleCss);
  webServer.on("/cFan.js", handleJavascript);
  webServer.on("/ajaxCall", handleAjaxCall);
  webServer.on("/favicon.ico", handleFavicon);
  /*
  webServer.on("/favicon.ico", []()
  {
    webServer.send(404, "text/plain", "");
  });
  */
  webServer.onNotFound(notFoundHandler);

  httpUpdater.setup(&webServer, ota_relativeUrl, cfg.userName, cfg.userPassword);
  webServer.begin();
  Serial.println("Web Server is on");
  //if (wifiMode == WIFI_STA) // temp, remove:
  {
    Serial.print("OTA updater at: ");
    Serial.println(ota_relativeUrl);
  }
}

void loopWebServer()
{
  webServer.handleClient();
}
