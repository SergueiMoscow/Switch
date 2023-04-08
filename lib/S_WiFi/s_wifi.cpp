#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "S_WiFi.h"
#include "S_FS.h"
#define MY_PASS "MyPass"
#define MY_SSIDS {"Test1", "Test2", "Test3"}
#define NUM_MY_SSIDS 3
#define STA 0
#define AP 1

//------------------- myConnect -------------------
bool S_WiFi::myConnect()
//------------------- myConnect -------------------
// connects only to hard-coded SSIDs.
{

  bool debug = false;
  if (debug)
    Serial.println("myConnect begin");
  int numMySSID = NUM_MY_SSIDS; // sizeof(ssid);
  String ssid[numMySSID] = MY_SSIDS;
  bool lFound = false;
  // String wifiSsid = commonSettings.getSetting("Wifi_Ssid", "default");
  String wifiSsid = "default";
  if (debug)
    Serial.print("wifiSsid: ");
  int nCompare = wifiSsid.substring(0, 7).compareTo("default");
  if (nCompare == 0)
  {
    if (debug)
      Serial.println("scan start");
    int n = WiFi.scanNetworks();
    String curSSID;
    if (debug)
      Serial.println("Мои сети " + numMySSID);
    if (n == 0)
    {
      Serial.println("no networks found");
    }
    else
    {
      if (debug)
        Serial.print(n);
      if (debug)
        Serial.println(" networks found");
      if (debug)
        Serial.print("Наших сетей ");
      if (debug)
        Serial.println(numMySSID);
      for (int i = 0; i < n; ++i)
      {
        if (lFound)
          break;
        // проверка наша ли сеть
        curSSID = WiFi.SSID(i);
        if (debug)
          Serial.print("Проверяем ");
        if (debug)
          Serial.println(curSSID);
        for (int y = 0; y < numMySSID; y++)
        {
          if (lFound)
            break;
          if (debug)
            Serial.print("Y: ");
          if (debug)
            Serial.println(y);
          if (curSSID.substring(0, sizeof(ssid[y])) == ssid[y])
          {
            // Нашли нашу сеть
            if (debug)
              Serial.println("Нашли нашу сеть " + curSSID);
            activeSSID = curSSID;
            if (debug)
              Serial.println("Наша сеть " + String(activeSSID));
            lFound = true;
            wifipass = MY_PASS;
            break;
          }
          if (lFound)
            break;
        } // end for my ssid
      }   // end for i (found ssid)
    }     // end if n (found ssid)
  }       // end if SSID=default
  else
  {
    // activeSSID = commonSettings.getSetting("Wifi_Ssid", "default");
    // wifipass = commonSettings.getSetting("Wifi_Pass", "default");
    lFound = true;
  }
  if (lFound)
  {
    Serial.print("Connecting to ");
    Serial.println(activeSSID);

    WiFi.mode(WIFI_STA);
    mode = STA;
    WiFi.begin(activeSSID.c_str(), wifipass.c_str());
    // Это работает
    // TODO: Считать из конфига
    // WiFi.setHostname("Test");

    int i = 0;
    while (WiFi.status() != WL_CONNECTED && i < 50)
    {
      delay(500);
      Serial.print(".");
      i++;
    }
  }
  connected = (WiFi.status() == WL_CONNECTED);
  if (connected)
  {
    // WiFi.hostname(myHostName);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    // lcd.setCursor(0,0);
    // lcd.print(activeSSID);
    // lcd.print("/");
    // lcd.print(WiFi.localIP());
    return true;
  }
  else
  {
    accessPoint();
  }
  return WiFi.isConnected();
}; // end myConnect function

//------------------- connect -------------------
void S_WiFi::connect()
//------------------- connect -------------------
{
  bool isConnected = false;
  S_FS fs = S_FS();
  JSONVar config;
  config = JSON.parse(fs.readFile("/wifi.json"));
  JSONVar keys = config.keys();
  String ssid, pass;
  for (int i = 0; i < keys.length(); i++)
  {
    ssid = JSON.stringify(keys[i]);
    ssid.replace("\"", "");
    Serial.println("Searching " + ssid);
    if (isSSID(ssid))
    {
      Serial.println("Found " + ssid);
      pass = JSON.stringify(config[keys[i]]);
      pass.replace("\"", "");
      WiFi.mode(WIFI_STA);
      mode = STA;
      Serial.println("Trying with pass " + pass);
      WiFi.begin(ssid.c_str(), pass.c_str());
      int i = 0;
      while (WiFi.status() != WL_CONNECTED && i < 50)
      {
        delay(500);
        Serial.print(".");
        i++;
      }
      isConnected = (WiFi.status() == WL_CONNECTED);
      if (isConnected)
      {
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
      } else {
        Serial.println("Not connected");
      }
    }
  }
  if (!isConnected) {
    myConnect();
  }
}

//------------------- isSSID -------------------
bool S_WiFi::isSSID(String ssid)
//------------------- isSSID -------------------
{
  int wifiCount = WiFi.scanNetworks();
  String tmpSSID;
  if (wifiCount == 0)
  {
    Serial.println("no networks found");
  }
  else
  {
    for (int i = 0; i < wifiCount; ++i)
    {
      tmpSSID = WiFi.SSID(i);
      Serial.println("Comparing " + ssid + " with " + tmpSSID);
      if (tmpSSID == ssid)
      {
        Serial.println("Found " + ssid);
        return true;
      }
    }
  }
  return false;
}

//------------------- WiFiLoop -------------------
void S_WiFi::WiFiLoop()
//------------------- WiFiLoop -------------------
{
  if (mode == AP) {
    return;
  }
  int status = WiFi.status();
  if (status != WL_CONNECTED)
  {
    myConnect();
  }
};

//------------------- accessPoint -------------------
bool S_WiFi::accessPoint()
//------------------- accessPoint -------------------
{
  Serial.println("Trying to create AP");
  WiFi.mode(WIFI_AP);
  mode = AP;
  boolean result = WiFi.softAP("ESPsoftAP_01", "pass-to-soft-AP");
  return result;
}