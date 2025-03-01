// https://kotyara12.ru/sitemap/
#include <Arduino.h>
// #include <ESP8266WiFi.h>
#include "S_Mode.h"
#include "S_Settings.cpp"
#include <FS.h>
#include <Arduino_JSON.h>
#include <LittleFS.h>
// #include "S_FS.h"
#include "S_Web.h"
#include "S_WiFi.h"
#include "S_MQTT.h"
// using std::string;
#include "S_Common.h"
#include "S_OTA.h"

S_Settings commonSettings;
// #define SETTINGS_FILE "/settings.json"

S_WiFi sWiFi;
S_MQTT sMQTT;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
PubSubClient* mqttClientPtr = &mqttClient;
S_Devices devices = S_Devices();
S_Devices* devicesPtr = &devices;

int loopCounter = 0;

void setup()
{
  Serial.begin(57200);
  Serial.println("Begin setup");
  LittleFS.begin();
  S_Mode::begin();
  // S_Mode::setNormalMode("Ok");
  // delay(10000);
  Serial.println("main.cpp. Trying to setup wifi");
  sWiFi.connect();
  Serial.println("main.cpp. Wifi connected, trying to setup web server");
  webServerSetup();
  devices.init();
  Serial.println("main.cpp. Trying to setup MQTT");
  sMQTT.init(mqttClientPtr, devicesPtr);
  Serial.println("main.cpp. Trying to set setUTime");
  S_Common::S_Common::setUTime();
  Serial.println("main.cpp. Trying to load OTA settings");
  S_OTA::loadConfig();
  Serial.println("main.cpp. Setup complete");
}

void loop()
{
  webServerLoop();
  sWiFi.WiFiLoop();
  DeviceMode currentMode = S_Mode::getCurrentMode();
  if (loopCounter >= 100) {
    Serial.println("main.cpp. Current mode: " + (String)currentMode);
    loopCounter = 0;
  }
  if (currentMode > MODE_CONFIG_MQTT)
    {sMQTT.loop();}
  if (currentMode == MODE_NORMAL)
    {S_OTA::loop();}
  delay(500);
  loopCounter ++;
}