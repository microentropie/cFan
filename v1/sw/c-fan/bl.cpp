/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "machine_bl.h"
#include "machine_io.h"

extern WiFiMode wifiMode;
extern void disconnectWiFi();

void ShutDown()
{
  if (wifiMode != WIFI_OFF)
    disconnectWiFi();
  MachineStop();
  machineIOs.SetLeds(OutSetAction_e::Off);

}

void Restart()
{
  machineIOs.ForceWiFiOn(); // must be sure that WiFi will stay on after restart

  ShutDown();
  ESP.restart();
}
