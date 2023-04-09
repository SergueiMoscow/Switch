// https://kotyara12.ru/sitemap/
#include <Arduino.h>
// #include <ESP8266WiFi.h>
//  #include "S_Settings.cpp"
#include <FS.h>
#include <Arduino_JSON.h>
#include <LittleFS.h>
// #include "S_FS.h"
#include "S_Web.h"
#include "S_WiFi.h"
#include "S_MQTT.h"
// using std::string;

// SSettings commonSettings;
#define SETTINGS_FILE "/settings.json"

S_WiFi sWiFi;
S_MQTT sMQTT;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
PubSubClient* pMqttClient = &mqttClient;

void setup()
{
  Serial.begin(57200);
  LittleFS.begin();
  // commonSettings.setSettingsFile(SETTINGS_FILE);
  sWiFi.connect();
  webServerSetup();
  sMQTT.init(pMqttClient);
}

void loop()
{
  webServerLoop();
  sWiFi.WiFiLoop();
  sMQTT.loop();
  delay(300);
}