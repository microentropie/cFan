/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

web server utilities

Sources repository: https://github.com/microentropie/
*/

#include "debugUtl.h"

#ifdef VERBOSE_SERIAL
#include <ESP8266WebServer.h>
extern ESP8266WebServer webServer;

String methodToString(HTTPMethod method)
{
  switch (method)
  {
  case HTTP_GET:
    return "GET";
  case HTTP_POST:
    return "POST";
  case HTTP_PUT:
    return "PUT";
  case HTTP_PATCH:
    return "PATCH";
  case HTTP_DELETE:
    return "DELETE";
  }
  return "Unknown";
}
#endif //VERBOSE_SERIAL

void logDetails()
{
#ifdef VERBOSE_SERIAL
  Serial.println(String(F("URL is: ")) + webServer.uri());
  Serial.println(String(F("HTTP Method on request was: ")) +
    methodToString(webServer.method()));
  // Print how many properties we received and then print their names
  // and values.
  Serial.println(String(F("Number of query properties: ")) + String(webServer.args()));
  int i;
  for (i = 0; i < webServer.args(); i++)
    Serial.println(" - " + webServer.argName(i) + " = " + webServer.arg(i));
#endif //VERBOSE_SERIAL
}
