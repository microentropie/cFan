/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:  2018-01-18

NTP time interface (http://www.microentropie.com)
Arduino ESP8266 based.

Network Time Protocol and localization business logic
*/

#include <Arduino.h>
#include <time.h>
#include <sys/time.h>                   // struct timeval
#include <coredecls.h>                  // settimeofday_cb()
//#include <sntp.h>
#include <ESP8266WiFi.h>
#include <TimeZone.h>
#include <DateTime.h>
#include "ntp_model.h"

//#define NTP_LOWLEVEL

#ifdef NTP_LOWLEVEL
#include <TimeLib.h>
#include <WiFiUdp.h>
static WiFiUDP Udp;
static unsigned int localPort = 8888;  // local port to listen to for UDP packets
static time_t getNtpTime();
static void sendNTPpacket(IPAddress &address);
static const char *ntpServerName;
static const int timeZone = 0;
#endif //NTP_LOWLEVEL

static int8_t timeSynch_ed = 0;
static void time_has_been_set (void)
{
  timeSynch_ed = 1;
  Serial.println();
  Serial.println(F("------------------ settimeofday() was called ------------------"));
}

void NtpTime_init(struct ntpConfig &ntpc)
{
  char TimeBuf[30];


  timeSynch_ed = 0;

  const char *pBrief, *pLong;
  struct TimeAndDateInfo lcl = GetLocalization();

  Serial.print(F("time zone: "));
  Serial.print(lcl.timeZone);
  Serial.print(F(", DST criteria: "));
  if (DstCriteriaStrings(lcl.dst, &pBrief, &pLong) >= 0)
    Serial.println(pBrief);
  else
    Serial.println(F("ERROR"));
  Serial.print(F("date separator: '"));
  Serial.print(lcl.dateSeparator);
  Serial.print(F("', Date format: ("));
  Serial.print(lcl.dateFormat);
  Serial.print(F(") "));
  if (DateFormatStrings(lcl.dateFormat, &pBrief, &pLong) >= 0)
    Serial.println(pBrief);
  else
    Serial.println(F("ERROR"));
  Serial.print(F("time separator: '"));
  Serial.print(lcl.timeSeparator);
  Serial.print(F("', Time format: ("));
  Serial.print(lcl.timeFormat);
  Serial.print(F(") "));
  if (TimeFormatStrings(lcl.timeFormat, &pBrief, &pLong) >= 0)
    Serial.println(pBrief);
  else
    Serial.println(F("ERROR"));

  WiFiMode wifiMode = WiFi.getMode();

  if (wifiMode == WIFI_STA)
  {
    settimeofday_cb(time_has_been_set);
    
    Serial.print(F("date/time needs to be set: "));
    Serial.print(DateTime::UtcIso8601DateTime(TimeBuf, sizeof(TimeBuf), time(NULL), true));
    Serial.println();

    Serial.println(F("sNTP time servers:"));
    //struct ntpConfig ntpc;
    //NtpConfig_Load(ntpc);
#ifdef NTP_LOWLEVEL
    Serial.print(F(" 1: ")); Serial.println(ntpc.Server1);
    ntpServerName = ntpc.Server1;
    Serial.println("Starting UDP");
    Udp.begin(localPort);
    Serial.print("udp Local port: ");
    Serial.println(Udp.localPort());
    time_t t = getNtpTime();
    Serial.println(t);
    setSyncProvider(getNtpTime);
    setSyncInterval(1800); // sync (call getNtpTime()) every 300"
    
    ESP.eraseConfig();
    timeval tv = { t, 0 };
    timezone tz = { 0, 0 };
    settimeofday(&tv, &tz);
#else //NTP_LOWLEVEL
    Serial.print(F(" 1: ")); Serial.println(ntpc.Server1);
    Serial.print(F(" 2: ")); Serial.println(ntpc.Server2);
    Serial.print(F(" 3: ")); Serial.println(ntpc.Server3);
    Serial.print(F("sNTP time initialization in progress "));
    IPAddress ntpServerIP; // NTP server's ip address
    bool ipFound = false;
    ipFound = WiFi.hostByName(ntpc.Server1, ntpServerIP);
    if(!ipFound) ipFound = WiFi.hostByName(ntpc.Server2, ntpServerIP);
    if(!ipFound) ipFound = WiFi.hostByName(ntpc.Server3, ntpServerIP);
    bool bForcedEmergency = false;
    if(!ipFound)
    {
      // emergency IP
      // 193.204.114.233 Italy Torino Consortium Garr - INRIM - Istituto Nazionale di Ricerca Metrologica
      // 185.19.184.35   Italy Rimini Parvati Srl
      // 212.45.144.206  Italy Rome Bilink Isp
      // 195.120.76.26   Italy Catanzaro Sir Meccanica Spa
      // 213.251.52.250  Italy Interoute Speedtest
      // 193.183.98.38   Sweden Stockholm Iperweb Ltd
      // 37.247.53.178
      ntpServerIP = IPAddress(193, 204, 114, 233);
      ipFound = bForcedEmergency = true;
    }
    if(ipFound)
    {
      Serial.print(F(" NTP IP: "));
      Serial.print(ntpServerIP);
      if (bForcedEmergency) Serial.print(F(" (emg)"));
      configTime(0, 0, ntpServerIP.toString().c_str());
      Serial.println();
    }
    else
      timeSynch_ed = 3; // inhibits loop
#endif //NTP_LOWLEVEL
  }
  else
    timeSynch_ed = 3; // inhibits loop
  if(timeSynch_ed == 3)
    Serial.println(F("date/time can't be set, sNTP can't be synchronized: no internet / DNS !"));
}

void NtpTime_loop()
{
  if(timeSynch_ed == 1)
  {
    char TimeBuf[30];

    Serial.println(String() + F("sNTP time synchronized in ") + String(millis()) + F(" ms after startup:"));
    time_t tm = time(NULL);
    Serial.println(DateTime::UtcIso8601DateTime(TimeBuf, sizeof(TimeBuf), tm, true));
    Serial.print(DateTime::LocalIso8601DateTime(TimeBuf, sizeof(TimeBuf), tm, true));
    Serial.println(F(" local time"));
    timeSynch_ed = 2;
  }
}

#ifdef NTP_LOWLEVEL
/*-------- NTP code ----------*/
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
static byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

static IPAddress defaultNtpServerIP(212, 45, 144, 206);

static time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  if(ntpServerIP == INADDR_NONE)
  {
    Serial.print("Name not resolved, forcing default: ");
    ntpServerIP = defaultNtpServerIP;
    Serial.println(ntpServerIP);
  }
    
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
static void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
#endif //NTP_LOWLEVEL
