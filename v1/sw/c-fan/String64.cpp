/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>

#include "String64.h"

// this function from Arduino forum
String String64(uint64_t number, int base)
{
  unsigned char buf[64];
  uint8_t i = 0;

  if (number == 0)
    return String('0');

  if (base < 2) base = 10;
  else if (base > 16) base = 16;

  while (number > 0)
  {
    uint64_t q = number / base;
    buf[i++] = number - q*base;
    number = q;
  }

  String n64;
  for (; i > 0; i--)
    n64 += (char)(buf[i - 1] < 10 ?
      '0' + buf[i - 1] :
      'A' + buf[i - 1] - 10);

  return n64;
}



//---
static int asciihex_to_int(uint8_t c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  else if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  else if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  return 0xFF; // invalid char !
}

uint64_t HexStringToUint64(const char *c)
{
  uint64_t result = 0;

  while (*c == ' ' || *c == '\t') ++c;

  for (; *c != 0; ++c)
  {
    int nibble = asciihex_to_int(*c);
    if (nibble == 0xFF) break;
    result = (result <<= 4) | nibble;
  }
  return result;
}


uint64_t atoull(const char *c)
{
  uint64_t result = 0;

  while (*c == ' ' || *c == '\t') ++c;

  for (; *c != 0; ++c)
  {
    if (*c >= '0' && *c <= '9')
      result = (result * 10) + (uint64_t)(*c - '0');
    else
      break;
  }

  return result;
}

int64_t atoll(const char *c)
{
  while (*c == ' ' || *c == '\t') ++c;

  if (*c == '-')
    return -1LL * (long long int)atoull(++c);
  return (long long int)atoull(c);
}



uint64_t HexStringToUint64(const String &str)
{
  return HexStringToUint64(str.c_str());
}

uint64_t atoull(const String &str)
{
  return atoull(str.c_str());
}

int64_t atoll(const String &str)
{
  return atoll(str.c_str());
}

/*
Why should anyone save a 64 bit number as string in PROGMEM ?,
better saving it as 64 bit integer directly and the convert to string in required.

//#define ESP8266
#if defined(ESP8266)
uint64_t HexStringToUint64(const __FlashStringHelper *ifsh)
{
  PGM_P p = reinterpret_cast<PGM_P>(ifsh);

  uint8_t c;
  for (;;)
  {
    c = pgm_read_byte(p);
    if (c != ' ' && c != '\t')
      break;
    ++p;
  }

  uint64_t result = 0;
  for (;;)
  {
    c = pgm_read_byte(p++);
    if (c == '\0') break;
    int nibble = asciihex_to_int(c);
    if (nibble == 0xFF) break;
    result = (result <<= 4) | nibble;
  }
  return result;
}

uint64_t atoull(const __FlashStringHelper *ifsh)
{
  PGM_P p = reinterpret_cast<PGM_P>(ifsh);

  uint8_t c;
  for (;;)
  {
    c = pgm_read_byte(p);
    if (c != ' ' && c != '\t')
      break;
    ++p;
  }

  uint64_t result = 0;
  for (;;)
  {
    c = pgm_read_byte(p++);
    if (c == 0) break;
    if (c >= '0' && c <= '9')
      result = (result * 10) + (uint64_t)(c - '0');
    else
      break;
  }
  return result;
}

int64_t atoll(const __FlashStringHelper *ifsh)
{
  PGM_P p = reinterpret_cast<PGM_P>(ifsh);

  uint8_t c;
  for (;;)
  {
    c = pgm_read_byte(p);
    if (c != ' ' && c != '\t')
      break;
    ++p;
  }

  int64_t sign = 1LL;
  if (pgm_read_byte(p) == '-')
  {
    sign = -1LL;
    p++;
  }

  int64_t result = 0;
  for (;;)
  {
    c = pgm_read_byte(p++);
    if (c == 0) break;
    if (c >= '0' && c <= '9')
      result = (result * 10) + (int64_t)(c - '0');
    else
      break;
  }
  return sign * result;
}
#endif //ESP8266
*/