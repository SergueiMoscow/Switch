#include <Arduino.h>
#include "S_Mode.h"

class S_WiFi
{

private:
  String activeSSID;
  String wifipass;
  bool debug;
  int mode;

public:
  bool connected;
  void connect();
  bool myConnect();
  void WiFiLoop();
  bool accessPoint();
  bool isSSID(String);
};