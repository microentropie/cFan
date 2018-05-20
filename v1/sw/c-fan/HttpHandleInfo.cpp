/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

http page handler, system info

Sources repository: https://github.com/microentropie/
*/

#include <core_version.h>
#include <ESP8266WebServer.h>
//#include <ESP8266HTTPUpdateServer.h>
#include <DateTime.h>

#include "common.h"
#include "debugUtl.h"
#include "settings.h"
#include "machine_io.h"
#include "HttpUtils.h"
#include "enumsToString.h"
#include "SystemInfo.h"
#include "src/Macro_BuildDATE.h"

extern ESP8266WebServer webServer;
extern baseConnectionConfig cfg;

#include "WebServer.h"
#include "WebServerUtils.h"



static String PrepareRow(const __FlashStringHelper *title, String value)
{
  return String() + F("<tr><td>") + String(title) + F("</td><td>") + value + F("</td></tr>\n");
}

void infoHandler()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    forbiddenHandler(); // configuration modifications only possible if locally connected

  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();

  char h[80];

  Serial.print(FPSTR(__func__));
  Serial.println(FPSTR("()"));
  logDetails();

  String s = PrepareHead(-1, PublicName(NULL));

  snprintf(h, sizeof(h),
    "<body bgcolor='#%02X%02X%02X'>",
    0xB0, 0xFF, 0xB0);
  s += h;

  s += F("<h3>System Info</h3>"
    "<table>");
  s += PrepareRow(F("machine name"), String(PublicName(NULL)));
  s += PrepareRow(F("Source compiled"), String(BUILD_YEAR_CH0) + String(BUILD_YEAR_CH1) + String(BUILD_YEAR_CH2) + String(BUILD_YEAR_CH3) + String('-') + String(BUILD_MONTH_CH0) + String(BUILD_MONTH_CH1) + String('-') + String(BUILD_DAY_CH0) + String(BUILD_DAY_CH1) + " " + String(__TIME__));
#if defined(ARDUINO_ARCH_ESP8266)
  s += PrepareRow(F("Esp8266 Arduino Release"), String(F(ARDUINO_ESP8266_RELEASE)));

  uint32_t ui32;
  uint16_t ui16;
  uint8_t ui8;
  ui32 = ESP.getSketchSize();
  s += PrepareRow(F("getSketchSize()"), String(ui32));

  ui32 = ESP.getFreeSketchSpace();
  s += PrepareRow(F("getFreeSketchSpace()"), String(ui32));

  s += PrepareRow(F("System started"), String(millis()) + String(F(" ms ago (counter overflows every 71')")));

  DateTime::UtcIso8601DateTime(h, sizeof(h), time(NULL), true);
  s += PrepareRow(F("Now"), String(h));

  // TO get Vcc:
  // (0) RF must be enabled
  // (1) leave ADC pin floating
  // (2) out of any function, set:
  //     ADC_MODE(ADC_VCC);
  // (3) now analogRead(A0); cannot be used
  //     ESP.getVcc(); // will return the correct Vcc value (must be divided by 1024)
  //ui16 = ESP.getVcc();
  //s += PrepareRow(F("getVcc()"), String((float)ui16 / 1024.0f) + " V");
  ui16 = analogRead(A0);
  s += PrepareRow(F("analogRead(A0)"), String(ui16));

  ui8 = ESP.getBootMode();
  s += PrepareRow(F("getBootMode()"), String(ui8));

  ui8 = ESP.getBootVersion();
  s += PrepareRow(F("getBootVersion()"), String(ui8));

  ui32 = ESP.getChipId();
  s += PrepareRow(F("getChipId()"), String(ui32));

  ui8 = ESP.getCpuFreqMHz();
  s += PrepareRow(F("getCpuFreqMHz()"), String(ui8));

  ui32 = ESP.getCycleCount();
  s += PrepareRow(F("getCycleCount()"), String(ui32));

  ui32 = ESP.getFlashChipId();
  s += PrepareRow(F("getFlashChipId()"), String(ui32));

  FlashMode_t fm = ESP.getFlashChipMode();
  s += PrepareRow(F("getFlashChipMode()"), String(FlashModeToString(fm)));

  ui32 = ESP.getFlashChipRealSize();
  s += PrepareRow(F("getFlashChipRealSize()"), String(ui32));

  ui32 = ESP.getFlashChipSize();
  s += PrepareRow(F("getFlashChipSize()"), String(ui32));

  ui32 = ESP.getFlashChipSizeByChipId();
  s += PrepareRow(F("getFlashChipSizeByChipId()"), String(ui32));

  ui32 = ESP.getFlashChipSpeed();
  s += PrepareRow(F("getFlashChipSpeed()"), String(ui32));

  ui32 = ESP.getFreeHeap();
  s += PrepareRow(F("getFreeHeap()"), String(ui32));

  s += PrepareRow(F("getResetInfo()"), String(ESP.getResetInfo()));

  const char *p = ESP.getSdkVersion();
  s += PrepareRow(F("getSdkVersion()"), String(p));
#elif defined(ARDUINO_ARCH_ESP32) 
  s += PrepareRow(F("Esp32 Arduino Release"), F("?"));
#else
#error("undefined architecture")
#endif

  s += F("</table>"
    "<br /><a href='/config'>Cancel</a><br />");
  s += COPYRIGHT;
  s += F("</body></html>");

#ifdef VERBOSE_SERIAL
  Serial.println(s);
#endif //VERBOSE_SERIAL

  SendHtmlPage(webServer, -1, s);
}

