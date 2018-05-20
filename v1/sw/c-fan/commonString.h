#ifndef _COMMONSTRING_H_
#define _COMMONSTRING_H_

void SerialPrintMillis();
class Print;
size_t PrintMillis(Print &userClass);
class String;
void StringAppendMillis(String &userString);


size_t PrintStringCharByChar(Print &userClass, const char *str);
size_t PrintStringCharByChar(Print &userClass, String &str);
#if defined(ARDUINO_ARCH_ESP8266)
size_t PrintStringCharByChar(Print &userClass, const __FlashStringHelper *str);
#endif //defined(ARDUINO_ARCH_ESP8266)

#endif //_COMMONSTRING_H_
