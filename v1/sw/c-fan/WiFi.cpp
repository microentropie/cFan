/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

WiFi initialization and handling

Sources repository: https://github.com/microentropie/
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#ifdef _USE_MDNS_
#include <ESP8266mDNS.h>
#endif //_USE_MDNS_
#ifdef _USE_UPNP_
#include <ESP8266SSDP.h>
#endif //_USE_UPNP_

#include "common.h"
#include "debugUtl.h"
#include "settings.h"
#include "machine_io.h"


extern baseConnectionConfig cfg;

WiFiMode wifiMode = WIFI_OFF;

#ifdef _USE_MDNS_
static MDNSResponder mdns;
#endif //_USE_MDNS_

void ZeroWiFi()
{
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  wifiMode = WIFI_OFF;
}

void setupWiFiAP()
{
  if (WiFi.getMode() != WIFI_AP)
  {
    WiFi.mode(WIFI_AP);
    delay(100);
  }
  WiFi.forceSleepWake();
  Serial.print("Starting AP ... ");

  const char *p = PublicName(NULL);
  WiFi.softAPConfig(cfg.ApIp, cfg.ApIp, IPAddress(255, 255, 255, 0));
  WiFi.softAP(p, cfg.ApPassword); // pw MUST be min 8 max 63 chars
  delay(10);
  Serial.println("running.");
  Serial.print("ssid: '");
  Serial.print(p);
  Serial.print("', pw: '");
  Serial.print(cfg.ApPassword);
  Serial.print("', ip: ");
  Serial.println(WiFi.softAPIP());
}

bool setupWiFiStation()
{
  if (WiFi.getMode() != WIFI_STA)
  {
    WiFi.mode(WIFI_STA);
    delay(100);
  }
  const char *Msg[] =
  {
    "Connecting to ",
  };
  WiFi.forceSleepWake();

  const char *p = PublicName(NULL);
  WiFi.hostname(p);
  Serial.print(String(Msg[0]) + "'" + String(cfg.ssid) + "',");
#ifdef VERBOSE_SERIAL
  Serial.print(" password: '");
  Serial.print(cfg.password);
  Serial.print("'");
#else VERBOSE_SERIAL
#endif VERBOSE_SERIAL

  WiFi.begin(cfg.ssid, cfg.password);
  if (cfg.bUseStaticIp)
    WiFi.config(cfg.ip, cfg.gateway, cfg.subnet); // force a fixed IP
  yield(); //delay(10);
  if (cfg.bUseStaticIp)
  {
    Serial.print(" forcing fixed IP: ");
    Serial.print(cfg.ip);
    Serial.print(" (gateway: ");
    Serial.print(cfg.gateway);
    Serial.print(", mask: ");
    Serial.print(cfg.subnet);
    Serial.print(")");
  }
  Serial.println();
  Serial.flush();
  // Wait for connection
  int timeout = 60; // 30" timeout
  /* file: wl_definitions.h
    WL_IDLE_STATUS      = 0,
    WL_NO_SSID_AVAIL    = 1,
    WL_SCAN_COMPLETED   = 2,
    WL_CONNECTED        = 3,
    WL_CONNECT_FAILED   = 4,
    WL_CONNECTION_LOST  = 5,
    WL_DISCONNECTED     = 6
  */
  while (timeout >= 0 && WiFi.status() != WL_CONNECTED)
  {
    machineIOs.SetLeds((timeout & 1) ? On : Off); // 1 Hz blink with 50% duty cycle
    delay(500);
#ifdef VERBOSE_SERIAL
    Serial.print('[');
    Serial.print(WiFi.status()),
      Serial.print(']'),
#else VERBOSE_SERIAL
    Serial.print(".");
#endif VERBOSE_SERIAL
    --timeout;
  }
  if (timeout < 0)
  {
    Serial.print(F("\nConnection error: ["));
    Serial.print(WiFi.status());
    Serial.print("]=");
    switch(WiFi.status())
    {
      case WL_NO_SHIELD:
        Serial.print(F("NO_SHIELD"));
        break;
      case WL_IDLE_STATUS:
        Serial.print(F("IDLE_STATUS"));
        break;
      case WL_NO_SSID_AVAIL:
        Serial.print(F("NO_SSID_AVAIL"));
        break;
      case WL_SCAN_COMPLETED:
        Serial.print(F("SCAN_COMPLETED"));
        break;
      case WL_CONNECTED:
        Serial.print(F("CONNECTED"));
        break;
      case WL_CONNECT_FAILED:
        Serial.print(F("CONNECT_FAILED"));
        break;
      case WL_CONNECTION_LOST:
        Serial.print(F("CONNECTION_LOST"));
        break;
      case WL_DISCONNECTED:
        Serial.print(F("DISCONNECTED"));
        break;
        default:
        Serial.print("?");
        break;
    }
    Serial.println();

    return false;
  }
  Serial.println("");
  Serial.print("Connected, ");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

#ifdef _USE_MDNS_
  if (mdns.begin(p, WiFi.localIP())) // advertize domain name
  {
    mdns.addService("http", "tcp", 80); // advertise DNS-SD services

    Serial.print("mDNS responder started: http://");
    Serial.print(p);
    Serial.print(".local = ");
    Serial.println(WiFi.localIP());
    Serial.println("NOTE: a bonjour/avahi/nss-mdns service active is required on host");
  }
  else
    Serial.println("Error, mDNS responder NOT started.");
#else //_USE_MDNS_
    Serial.println("mDNS disabled.");
#endif //_USE_MDNS_
  return true;
}

void disconnectWiFi()
{
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}

#ifdef _USE_UPNP_
void setupSSDP()
{
  Serial.println("Starting SSDP...");
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName("Philips hue clone");
  SSDP.setSerialNumber("001788102201");
  SSDP.setURL("index.html");
  SSDP.setModelName("Philips hue bridge 2012");
  SSDP.setModelNumber("929000226503");
  SSDP.setModelURL("http://www.meethue.com");
  SSDP.setManufacturer("Royal Philips Electronics");
  SSDP.setManufacturerURL("http://www.philips.com");
  SSDP.begin();
}
#endif //_USE_UPNP_

WiFiMode setupWiFi(WiFiMode mode)
{
  WiFi.persistent(false);

  if (mode == WIFI_STA)
  {
    Serial.println("WiFi mode = WIFI_STA");
    // the standard mode
    if (setupWiFiStation())
#ifdef _USE_UPNP_
      setupSSDP();
#else
      ;
#endif //_USE_UPNP_
    else
      setupWiFiAP();
  }
  else
  {
    // setup needed
    setupWiFiAP();
  }
  wifiMode = WiFi.getMode();
  return wifiMode;
}

void loopWiFi()
{
#ifdef _USE_MDNS_
  if (wifiMode == WIFI_STA)
    mdns.update();
#endif //_USE_MDNS_
  yield();
}

