/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

Common routines

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>

static const char msClose[] PROGMEM = " ms] ";

size_t PrintMillis(Print &userClass)
{
  size_t n = 0;
  n += userClass.print('[');
  n += userClass.print(millis());
  n += userClass.print(FPSTR(msClose));
  return n;
}

void SerialPrintMillis()
{
  PrintMillis(Serial);
}

void StringAppendMillis(String &userString)
{
  //PrintMillis(&String);
  userString += String('[');
  userString += String(millis());
  userString += String(FPSTR(msClose));
}


// NOTE [*]
// if the string is very long (some Kbyte) we need to print it char-by-char
// this slows down the process but guarantees a perfect execution
// The target of these functions is reducing the memory usage and avoid
// triggering the system WatchDog.
// Thus you can expect the execution time will not be low :-)

// see NOTE [*]
size_t PrintStringCharByChar(Print &userClass, const char *str)
{
  size_t n = 0;
  if(!str) return n;
  for(int i = 0; str[i]; ++i)
  {
    n += userClass.write(str[i]);
    yield(); // long strings may trigger watchdog
  }
  return n;
}

// see NOTE [*]
size_t PrintStringCharByChar(Print &userClass, const String &str)
{
  // the Arduino print(const String &str) method converts string to a char array then prints it
  // we need to keep heap memory usage low
  size_t n = 0;
  if(!str) return n;
  //int len = str.length(); the string length is buffered
  for(int i = 0; i < /*len*/str.length(); ++i)
  {
    n += userClass.write(str.charAt(i));
    yield(); // long strings may trigger watchdog
  }
  return n;
}

#if defined(ARDUINO_ARCH_ESP8266)
// see NOTE [*]
size_t PrintStringCharByChar(Print &userClass, const __FlashStringHelper *str)
{
  size_t n = 0;
  if(!str) return n;
  PGM_P p = reinterpret_cast<PGM_P>(str);
  for(;;)
  {
    uint8_t c = pgm_read_byte(p++);
    if (c == 0) break;
    n += userClass.write(c);
    yield(); // long strings may trigger watchdog
  }
  return n;
}
#endif //defined(ARDUINO_ARCH_ESP8266)

