// https://kotyara12.ru/sitemap/
#include <Arduino.h>
// #include <ESP8266WiFi.h>
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

void setup()
{
  Serial.begin(57200);
  Serial.println("Begin setup");
  LittleFS.begin();
  // commonSettings.setSettingsFile(SETTINGS_FILE);
  Serial.println("main.cpp. Trying to setup wifi");
  sWiFi.connect();
  Serial.println("main.cpp. Wifi connected, trying to setup web server");
  webServerSetup();
  devices.init();
  sMQTT.init(mqttClientPtr, devicesPtr);
  S_Common::S_Common::setUTime();
  S_OTA::loadConfig();
}

void loop()
{
  webServerLoop();
  sWiFi.WiFiLoop();
  sMQTT.loop();
  S_OTA::loop();
  delay(500);
}