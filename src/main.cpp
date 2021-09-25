// This work is heavilly based on
// https://www.hackster.io/erkr/wake-on-lan-wol-gateway-a6a639
//
// This small scetch allows you to wake a device - like a rasperry pi - by standard wake on lan.
// How it works is simple: Power your device with a "smart" power device, like a tasmota enabled power plug.
// this sketch will wait for the WOL package and and send the command to (e.g.) tasmota ...
//

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#include "wifi.h"

ESP8266WiFiMulti WiFiMulti;

unsigned int PORT = 12287;                             // WOL Port - do not change!
byte SecureOn[7] = "123456";                           // your password for WOL
byte destMAC[] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36}; // "virtual mac", which you use to wake your device

class BroadCastDeamon : public WiFiUDP
{
public:
  //  void PostUDP(const char *Packet, IPAddress ip, unsigned int destPort)
  //  {
  //    PostUDP(Packet, strlen(Packet), ip, destPort);
  //  }
  //  void PostUDP(const char *Packet, int len, IPAddress ip, unsigned int destPort)
  //  {
  //    WiFiUDP::beginPacket(ip, destPort);
  //    WiFiUDP::write(Packet, len);
  //    WiFiUDP::endPacket();
  //  }
  //  void BroadcastUDP(const char *Packet, unsigned int destPort)
  //  {
  //    BroadcastUDP(Packet, strlen(Packet), destPort);
  //  }
  //  void BroadcastUDP(const char *Packet, int len, unsigned int destPort)
  //  {
  //    IPAddress broadcastIP = (uint32_t)WiFi.localIP() | (~(uint32_t)WiFi.subnetMask());
  //    PostUDP(Packet, len, broadcastIP, destPort);
  //  }

  bool CheckReceived()
  {
    // if there's data available, read a packet and return length
    packetSize = parsePacket();
    if (packetSize)
    {
      Serial.print("Received UDP packet from [");
      Serial.print(remoteIP());
      Serial.println("]");
      // read the packet into packetBufffer
      int len = read(packetBuffer, 255);
      packetBuffer[len] = 0;
      //Serial.println(packetBuffer);
      return true;
    }
    return false;
  }
  char *GetPacketData()
  {
    return packetBuffer;
  }
  int GetPacketSize()
  {
    return packetSize;
  }

private:
  char packetBuffer[256]; //buffer to hold last packet retreived
  int packetSize;
};

BroadCastDeamon WOLDeamon;

void setup()
{
  Serial.begin(115200);
  Serial.flush();
    Serial.printf("\nSketchname: %s\nBuild: %s\t\tIDE: %d.%d.%d\n%s\n\n",
                (__FILE__), (__TIMESTAMP__), ARDUINO / 10000, ARDUINO % 10000 / 100, ARDUINO % 100 / 10 ? ARDUINO % 100 : ARDUINO % 10, ESP.getFullVersion().c_str());

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(STASSID, STAPSK);

  int count = 0;

  while ((WiFiMulti.run() != WL_CONNECTED))
  {
    delay(1000);
    count++;
    if (count > 1 * 10)
    {
      Serial.println("WiFi not coming up ... !? Rebooting!");
      ESP.restart();
    }
    Serial.print(".");
  }
  Serial.println("\Connected! Esp8266 IP: " + WiFi.localIP().toString());
  Serial.printf("Listening on port: %d\n\n", PORT);

  WOLDeamon.begin(PORT);
}

#define WOL_SECURE_PACKET_SIZE 108
bool HandleWOLReceived(byte *pwd, byte *mac)
{
  if (WOLDeamon.CheckReceived() && WOLDeamon.GetPacketSize() == WOL_SECURE_PACKET_SIZE)
  {
    byte *pPrefix = (byte *)WOLDeamon.GetPacketData();
    byte *pMAC = pPrefix + 6;   // start of target mac address
    byte *pPWD = pPrefix + 102; // start position of the SecureOn
    // check if it is has both a valid WOL prefix (6 times FF) and that it contains the defined SecureOn password
    for (int i = 0; i < 6; i++)
    {
      if (pPrefix[i] != 255 || pPWD[i] != pwd[i] || pMAC[i] != mac[i])
      {
        Serial.println("INVALID WOL packet received");
        return false;
      }
    }

    Serial.println("Valid WOL packet received");

    HTTPClient http;
    WiFiClient client;
    Serial.println("Sending command to tasmota!");
    http.begin(client, "http://192.168.20.100/cm?cmnd=Power%20on");
    int httpCode = http.GET();
    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK)
      {

        String payload = http.getString();
        Serial.println(payload);
      }
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  return true;
}

void loop()
{
  HandleWOLReceived(SecureOn, destMAC);
  delay(10);
}
