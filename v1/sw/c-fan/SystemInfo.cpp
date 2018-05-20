/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

system info

Sources repository: https://github.com/microentropie/
*/

#include "SystemInfo.h"

const __FlashStringHelper *FlashModeToString(FlashMode_t m)
{
  switch (m)
  {
  case FM_QIO:
    return F("QIO");
  case FM_QOUT:
    return F("QOUT");
  case FM_DIO:
    return F("DIO");
  case FM_DOUT:
    return F("DOUT");
  case FM_UNKNOWN:
    return F("UNKNOWN");
  }
  return F("?");
}
