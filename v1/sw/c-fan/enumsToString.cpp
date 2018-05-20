/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

enum -> string

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>
#include "machine_io.h"

const char *OutSetActionToString(OutSetAction_e osStatus)
{
  switch (osStatus)
  {
  case OutSetAction_e::Off:
    return "Off";
  case OutSetAction_e::On:
    return "On";
  case OutSetAction_e::noChange:
    return "noChange";
  }
  return "?";
}

const char *TemperatureUnitsNameToString(temperatureUnits_e te)
{
  switch (te)
  {
  case temperatureUnits_e::Celsius:
    return "Celsius";
  case temperatureUnits_e::Kelvin:
    return "Kelvin";
  case temperatureUnits_e::Fahrenheit:
    return "Fahrenheit";
  case temperatureUnits_e::Reaumur:
    return "Reaumur";
  case temperatureUnits_e::Newton:
    return "Newton";
  case temperatureUnits_e::Rankine:
    return "Rankine";
  }
  return "?";
}

const char *TemperatureUnitsNotationToString(temperatureUnits_e te)
{
  switch (te)
  {
  case temperatureUnits_e::Celsius:
    return "&deg;C";
  case temperatureUnits_e::Kelvin:
    return "&deg;K";
  case temperatureUnits_e::Fahrenheit:
    return "&deg;F";
  case temperatureUnits_e::Reaumur:
    return "&deg;r";
  case temperatureUnits_e::Newton:
    return "&deg;N";
  case temperatureUnits_e::Rankine:
    return "&deg;R";
  }
  return "?";
}

String TemperatureToString(int temp, temperatureUnits_e units)
{
  int integerPart = temp / 10;
  int decimalPart = abs(temp) % 10;
  return String(integerPart) + String(".") + String(decimalPart) + TemperatureUnitsNotationToString(units);
}
