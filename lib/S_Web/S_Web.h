#ifndef _S_WEB_
#define _S_WEB_

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <Arduino_JSON.h>
#include <LittleFS.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "S_FS.h"
#include "S_Settings.h"
#include "buildTime.h"
#include "S_OTA.h"
#include <SPI.h>

extern ESP8266WebServer server;
void webServerSetup();
void webServerLoop();
void webFSBrowser();
void setup();
void webSettings();
void webWifi();
void webStyle();
String webMenu(String current);
void test();
void webReset();
void webUpdate();
void handleFileUpload();
void handleFileRead();

bool checkSecret();
#endif
