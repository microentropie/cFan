#ifndef _STRING64_H_
#define _STRING64_H_

#include <WString.h>
#include <stdint.h>

extern String String64(uint64_t number, int base = 10);

extern uint64_t HexStringToUint64(const char *str);
extern uint64_t atoull(const char *c);
extern int64_t atoll(const char *c);

extern uint64_t HexStringToUint64(const String &str);
extern uint64_t atoull(const String &str);
extern int64_t atoll(const String &str);
/*
#ifdef ESP8266
extern uint64_t atoull(const __FlashStringHelper *ifsh);
extern int64_t atoll(const __FlashStringHelper *ifsh);
#endif //ESP8266
*/
#endif //_STRING64_H_
