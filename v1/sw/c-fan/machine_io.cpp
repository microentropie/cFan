/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

Hardware interface layer

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>
#include "machine_io.h"
#include "common_io.h"
#include "common.h"
#include "debugUtl.h"


#if !_ADC_TEMPERATURE_
// following 2 includes for OneWire DS18B20 temperature sensor:
#include <DallasTemperature.h> 
#endif //_ADC_TEMPERATURE_

MachineIo machineIOs;

MachineIo::MachineIo()
{
  // cannot call libraries methods from constructor
  // only variables initialization with constant values
  pOneWire = NULL;
  pSensors = NULL;

  bLedsShow = true;
  temperatureScale = temperatureUnits_e::Celsius;
  lastTemperatureReadin = 0UL;
  temperatureCachedValue = 0;

}

void MachineIo::Init(bool bLedsShow, temperatureUnits_e temperatureScale)
{
#ifdef VERBOSE_SERIAL
  Serial.println("MachineIo::Init()");
#endif //VERBOSE_SERIAL
  // GPIO init:
  pinMode(RY_FAN_PIN, OUTPUT);
  digitalWrite(RY_FAN_PIN, 0);
  pinMode(RY_FAN_PIN2, OUTPUT);
  digitalWrite(RY_FAN_PIN2, 1);

  pinMode(RY_LIGHT_PIN, OUTPUT);
  digitalWrite(RY_LIGHT_PIN, 0);
  pinMode(RY_LIGHT_PIN2, OUTPUT);
  digitalWrite(RY_LIGHT_PIN2, 1);

  //
  // User interface (LEDs and button):
  // prepare GPIOs:
  pinMode(MODULE_BLUELED_PIN, OUTPUT);
  digitalWrite(MODULE_BLUELED_PIN, 1); // init module blue LED off

#if !_ADC_TEMPERATURE_
  // temperature sensor:
  pOneWire = new OneWire(ONE_WIRE_BUS);
  pSensors = new DallasTemperature(pOneWire);
  pSensors->begin();
  this->temperatureScale = temperatureScale;
  lastTemperatureReadin = 0UL;
#endif //_ADC_TEMPERATURE_
  this->bLedsShow = bLedsShow;

  // phisical jumper for WiFi on/off
#if WIFI_ENABLE_PIN == 16
  pinMode(WIFI_ENABLE_PIN, INPUT_PULLDOWN_16);
#else //WIFI_ENABLE_PIN
  pinMode(WIFI_ENABLE_PIN, INPUT_PULLUP);
#endif
}

#define WiFiMagicNumber 0x123D3210

// return:
// 0: WiFi must be set OFF
// 1: WiFi must be set ON
// 2: WiFi must temporarily be set ON (until power off)
unsigned char MachineIo::WiFiReservationStatus()
{
#if WIFI_ENABLE_PIN == 16
  if (digitalRead(WIFI_ENABLE_PIN) == 0) return 1;
#else //WIFI_ENABLE_PIN
  if (digitalRead(WIFI_ENABLE_PIN) == 1) return 1;
#endif
  bool en = false;
  uint32_t rtcWiFiForceOn = 0;
  ESP.rtcUserMemoryRead(0, &rtcWiFiForceOn, sizeof(rtcWiFiForceOn));
  if (rtcWiFiForceOn == WiFiMagicNumber)
  {
#ifdef VERBOSE_SERIAL
    Serial.println("WiFi jumper Off, but forced by RTC");
#endif //VERBOSE_SERIAL
    // If the following code is left commented out, this unit will restart with WiFi On.
    // Only when the power will be switched on, thsi information will be lost by RTC
    /*
    rtcWiFiForceOn = 0;
    ESP.rtcUserMemoryWrite(0, &rtcWiFiForceOn, sizeof(rtcWiFiForceOn))
    */
    return 2;
  }
  return 0;
}
void MachineIo::ForceWiFiOn()
{
  uint32_t rtcWiFiForceOn = WiFiMagicNumber;
  ESP.rtcUserMemoryWrite(0, &rtcWiFiForceOn, sizeof(rtcWiFiForceOn));
#ifdef VERBOSE_SERIAL
  Serial.println("WiFi On forced by writing to RTC (will be on after reboot)");
#endif //VERBOSE_SERIAL
}
#undef WiFiMagicNumber

bool MachineIo::AreLedsEnabled()
{
  return bLedsShow;
}

void MachineIo::SetRelays(OutSetAction_e rFan, OutSetAction_e rLight)
{
#ifdef VERBOSE_SERIAL
  Serial.println("MachineIo::SetRelays()");
#endif //VERBOSE_SERIAL
  // GPIO set
  if (rFan != OutSetAction_e::noChange)
  {
    digitalWrite(RY_FAN_PIN, (rFan == OutSetAction_e::On) ? 1 : 0);
    digitalWrite(RY_FAN_PIN2, (rFan == OutSetAction_e::On) ? 0 : 1);
#ifdef VERBOSE_SERIAL
    SerialPrintMillis();
    Serial.print(F("FanRelay is now "));
    Serial.print((rFan == OutSetAction_e::On) ? "ON" : "OFF");
    Serial.print(F(" (GPIO "));
    Serial.print(RY_FAN_PIN);
    Serial.print(" & ");
    Serial.print(RY_FAN_PIN2);
    //Serial.print(" = ");
    //Serial.print((rFan == OutSetAction_e::On) ? 1 : outOFF);
    Serial.println(")");
#endif //VERBOSE_SERIAL
  }

  if (rLight != OutSetAction_e::noChange)
  {
    digitalWrite(RY_LIGHT_PIN, (rLight == OutSetAction_e::On) ? 1 : 0);
    digitalWrite(RY_LIGHT_PIN2, (rLight == OutSetAction_e::On) ? 0 : 1);
#ifdef VERBOSE_SERIAL
    SerialPrintMillis();
    Serial.print(F("LightRelay is now "));
    Serial.print((rLight == OutSetAction_e::On) ? "ON" : "OFF");
    Serial.print(F(" (GPIO "));
    Serial.print(RY_LIGHT_PIN);
    Serial.print(" & ");
    Serial.print(RY_LIGHT_PIN2);
    //Serial.print(" = ");
    //Serial.print((rLight == OutSetAction_e::On) ? outON : outOFF);
    Serial.println(")");
#endif //VERBOSE_SERIAL
  }
}

void MachineIo::GetRelays(OutSetAction_e &rFan, OutSetAction_e &rLight)
{
  // GPIO get
  uint8_t v;

  v = digitalRead(RY_FAN_PIN); // not sure digitalRead() returns 1 for pin high
  rFan = (!(v == 0)) ? OutSetAction_e::On : OutSetAction_e::Off;
#ifdef VERBOSE_SERIAL
  Serial.print("Get FanRelay : ");
  Serial.print((rFan == OutSetAction_e::On) ? "ON" : "OFF");
  Serial.print(" (GPIO");
  Serial.print(RY_FAN_PIN);
  Serial.print(") digitalRead=");
  Serial.println(v);
#endif //VERBOSE_SERIAL

  v = digitalRead(RY_LIGHT_PIN); // not sure digitalRead() returns 1 for pin high
  rLight = (!(v == 0)) ? OutSetAction_e::On : OutSetAction_e::Off;
#ifdef VERBOSE_SERIAL
  Serial.print("Get LightRelay : ");
  Serial.print((rLight == OutSetAction_e::On) ? "ON" : "OFF");
  Serial.print(" (GPIO");
  Serial.print(RY_LIGHT_PIN);
  Serial.print(") digitalRead=");
  Serial.println(v);
#endif //VERBOSE_SERIAL
}

void MachineIo::SetLeds(OutSetAction_e b)
{
  /*
  if (r != noChange)
    digitalWrite(PANEL_REDLED_PIN, r == Off);
  if (g != noChange)
    digitalWrite(PANEL_GREENLED_PIN, g == Off);
*/  if (b != noChange)
digitalWrite(MODULE_BLUELED_PIN, b == Off);
}

int MachineIo::GetTemperature()
{
#if _ADC_TEMPERATURE_
  // GPIO read
  //return TEMP_NO_READING;
  int adc = 0;
  for (int i = 0; i < 32; ++i)
    adc += analogRead(TEMP_PIN);
  adc = adc >> 5;

  return adc;
#else
  // as tempereature readin is slow and ad temperature changes slowly
  // just cache the value
  if(millis() - lastTemperatureReadin < 31000) return temperatureCachedValue;

  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  pSensors->requestTemperatures(); // Send the command to get temperatures 
                                 // After we got the temperatures, we can print them here.
                                 // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  int t = 0;
  switch (temperatureScale)
  {
  case Celsius:
    t = pSensors->getTempCByIndex(0) * 10.0 + 0.5;
    break;
  case Kelvin:
    t = (pSensors->getTempCByIndex(0) + 273.15) * 10.0 + 0.5;
    break;
  case Fahrenheit:
    t = pSensors->getTempFByIndex(0) * 10.0 + 0.5;
    break;
  case Reaumur:
    t = pSensors->getTempCByIndex(0) * 12.5 + 0.5;
    break;
  case Newton:
    t = pSensors->getTempCByIndex(0) * 3.3 + 0.5;
    break;
  case Rankine:
    t = (pSensors->getTempFByIndex(0) + 459.67) * 10.0 + 0.5;
    break;
  }
  temperatureCachedValue = t;
  lastTemperatureReadin = millis();

  return t;
#endif
}



temperatureUnits_e MachineIo::GetTemperatureUnits()
{
  return temperatureScale;
}
