#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <LittleFS.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

// class S_Web
// {
//     private:
//        ESP8266WebServer server;
// public:
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
static String getBuildVersion();
// };