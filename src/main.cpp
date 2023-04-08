//https://kotyara12.ru/sitemap/
#include <Arduino.h>
//#include <ESP8266WiFi.h>
// #include "S_Settings.cpp"
#include <FS.h>
#include <Arduino_JSON.h>
#include <LittleFS.h>
//#include "S_FS.h"
#include "S_Web.h"
#include "S_WiFi.h"
using std::string;

//SSettings commonSettings;
#define SETTINGS_FILE "/settings.json"

S_WiFi sWiFi;



void setup() {
  Serial.begin(57200);
  LittleFS.begin();
  //commonSettings.setSettingsFile(SETTINGS_FILE);
  sWiFi.connect();
  webServerSetup();
}

void loop() {
  webServerLoop();
  sWiFi.WiFiLoop();
  delay(300);
}