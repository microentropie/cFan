/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

Main project entry.
The following libraries are required:
* Sort (https://github.com/microentropie/Sort)
* serializeEeprom (https://github.com/microentropie/serializeEeprom)
* DateTimeManipulation (https://github.com/microentropie/DateTimeManipulation)
* OneWire from Paul Stoffregen (https://github.com/PaulStoffregen/OneWire)
* Arduino-Temperature-Control-Library (https://github.com/milesburton/Arduino-Temperature-Control-Library)
* IRremoteESP8266 (menu Sketch -> Include Library -> Manage Libraries -> IRremoteESP8266) (https://github.com/markszabo/IRremoteESP8266)

Sources repository: https://github.com/microentropie/
*/

#include <core_version.h>
#include <TimeZone.h>

#include "common.h"
#include "debugUtl.h"
#include "common_io.h"
#include "WebServer.h"
#include "WiFi.h"
#include "src/Macro_BuildDATE.h"
#include "SystemInfo.h"

//#include "FanCycle.h"
#include "settings.h"
#include "src/time/TimeNTP.h"
#include "machine_bl.h"
#include "machine_io.h"
#include "Ir_bl.h"

extern bool loadWiFiConfig();
extern void Localization_init();
extern void ntp_init();

extern WiFiMode wifiMode;
void setup()
{
  ZeroWiFi();

  Serial.begin(UART_BAUDRATE, SERIAL_8N1, SERIAL_TX_ONLY);
#if defined(VERBOSE_SERIAL) && VERBOSE_SERIAL == SystemAndMyLog
  Serial.setDebugOutput(true); // enable system log to UART0
#endif //VERBOSE_SERIAL

  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println("------------ FRESH FROM RESET :-) ------------");
  Serial.print(F("time (ms):            ")); Serial.println(millis());
  Serial.println(F(__FILE__));
  Serial.println(COPYRIGHTtxt);
  Serial.print(F("This is:              ")); Serial.println(PublicName("cFan"));
  Serial.print(F("Compiled:             ")); Arduino_DATE_print(Serial);
                                             Serial.println(" " + String(F(__TIME__)));
  //                                         Serial.println(F(__TIMESTAMP__));
  Serial.print(F("Esp8266 Arduino Rel.: ")); Serial.println(F(ARDUINO_ESP8266_RELEASE));
  Serial.print(F("CpuFreq (MHz):        ")); Serial.println(ESP.getCpuFreqMHz());
  Serial.print(F("Flash Chip Mode:      ")); Serial.println(FlashModeToString(ESP.getFlashChipMode()));
  Serial.print(F("FlashChipRealSize:    ")); Serial.println(ESP.getFlashChipRealSize());
  Serial.print(F("FreeHeap:             ")); Serial.println(ESP.getFreeHeap());
  Serial.print(F("FreeSketchSpace:      ")); Serial.println(ESP.getFreeSketchSpace());
  Serial.print(F("SketchSize:           ")); Serial.println(ESP.getSketchSize());
  Serial.println(F("Reset info:"));
  Serial.println(ESP.getResetInfo());
  Serial.println("----------------------------------------------");
#if defined(VERBOSE_SERIAL) && VERBOSE_SERIAL == SystemAndMyLog
  Serial.println(F("System log enabled"));
#endif //VERBOSE_SERIAL
  Serial.println();
  Serial.flush();

  MachineInit();
  yield();

  unsigned char wrs = machineIOs.WiFiReservationStatus();
  if (wrs > 0)
  {
    if (wrs > 1)
      Serial.println(F("WiFi OFF (jumper closed), but forced ON by IR Command"));

    bool ok = loadWiFiConfig();
    if (!ok) // config error
      wifiMode = WIFI_AP;
    else
      wifiMode = WIFI_STA;

    Serial.print(F("WiFiMode will be set to "));
    Serial.println(WiFiModeToString(wifiMode));

    // Connect to WiFi network or create an AP
    wifiMode = setupWiFi(wifiMode); // if unable to connect to an AP, becomes an AP itself

    Localization_init();
    yield();

    ntp_init();
    yield();

    setupWebServer();
    yield();
  }
  else
  {
    wifiMode = WIFI_OFF;
    Serial.println("Wifi is OFF");
  }

  IrInit();

  machineIOs.SetLeds(Off);
}

void loop()
{
  if (wifiMode == WIFI_OFF)
    MachineUpdateStatus();
  else
  {
    loopWiFi();
    loopWebServer();
    MachineUpdateStatus();
    NtpTime_loop();
  }
  IrLoop();

  if (machineIOs.AreLedsEnabled())
  {
    // ESP module blue LED will flash as a good health indicator
    if (wifiMode == WIFI_OFF)
      machineIOs.SetLeds(((millis() / 500) & 1) ? On : Off); // 1 Hz blink with 50% duty cycle
    else if (wifiMode == WIFI_STA)
      machineIOs.SetLeds((((millis() / 125) & 7) == 0) ? On : Off); // 1 Hz blink with 12.5% duty cycle
    else
      machineIOs.SetLeds((((millis() / 125) & 7) != 0) ? On : Off); // 1 Hz blink with 87.5% duty cycle
  }
  yield();
}
