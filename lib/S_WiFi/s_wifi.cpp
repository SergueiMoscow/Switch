#include <Arduino.h>
#include <ArduinoJson.h>
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

  // Определяем размер JSON-документа. Размер зависит от структуры вашего JSON-файла.
  // Здесь 1024 байта используется как пример. При необходимости увеличьте размер.
  StaticJsonDocument<1024> doc;

  // Чтение JSON-файла
  if (!fs.readJsonFile("/wifi.json", doc)) {
    Serial.println("Не удалось прочитать /wifi.json");
    myConnect();
    return;
  }

  // Проверяем, является ли корневой элемент объектом
  if (!doc.is<JsonObject>()) {
    Serial.println("/wifi.json имеет неверный формат");
    myConnect();
    return;
  }

  JsonObject config = doc.as<JsonObject>();

  // Перебираем все пары ключ-значение в JSON-объекте
  for (JsonPair kv : config) {
    const char* ssid = kv.key().c_str();
    const char* pass = kv.value().as<const char*>();

    Serial.print("Поиск SSID: ");
    Serial.println(ssid);

    if (isSSID(ssid)) {
      Serial.print("Найден SSID: ");
      Serial.println(ssid);

      WiFi.mode(WIFI_STA);
      mode = STA;
      WiFi.begin(ssid, pass);

      int attempt = 0;
      while (WiFi.status() != WL_CONNECTED && attempt < 50) {
        delay(500);
        Serial.print(".");
        attempt++;
      }

      isConnected = (WiFi.status() == WL_CONNECTED);
      if (isConnected) {
        Serial.println();
        Serial.print("Подключено. IP: ");
        Serial.println(WiFi.localIP());
        break; // Выходим из цикла, так как подключение успешно
      } else {
        Serial.println();
        Serial.println("Не удалось подключиться");
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
    S_Mode::setConfigWifiMode("no networks found");
  }
  else
  {
    for (int i = 0; i < wifiCount; ++i)
    {
      tmpSSID = WiFi.SSID(i);
      if (tmpSSID == ssid)
      {
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
  Serial.println("Trying to create Access Point");
  WiFi.mode(WIFI_AP);
  mode = AP;
  boolean result = WiFi.softAP("ESPsoftAP_01", "pass-to-soft-AP");
  if (result) {
    Serial.print("AP created: ");
    Serial.print(WiFi.softAPSSID());
    Serial.print(" IP: ");
    Serial.println(WiFi.softAPIP());
    S_Mode::setConfigWifiMode("AP created " + WiFi.softAPSSID());
  }
  return result;
}
