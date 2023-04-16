#include "S_OTA.h"

  void S_OTA::loadConfig()
  {
    otaSettings = JSON.parse(S_FS::fileContent("ota.json"));
    checkPeriod = otaSettings["period"];
    module_type = S_Settings::delQuotes(otaSettings["type"]);
    Serial.print("Check updates every ");
    Serial.print(checkPeriod);
    Serial.println(" seconds (module" + module_type + ")");
  }

  //------------------------------autoUpdate-------------
  void S_OTA::autoUpdate()
  {
    Serial.println("Begin autoupdate");
    String add_parameter = "&log=var:";
    JSONVar otaSettings = JSON.parse(S_FS::fileContent("ota.json"));
    String serverVersion = getServerVersion(add_parameter);
    String buildVersion = getBuildVersion();
    String update_url = S_Settings::delQuotes(otaSettings["url_update"]);
    WiFiClient wifiClient;
    Serial.println("Server " + serverVersion +" local: " + buildVersion);
    if (serverVersion > buildVersion)
    {
      Serial.print("Server version:");
      Serial.println(serverVersion);
      Serial.print("Build version:");
      Serial.println(buildVersion);
      String s = S_Settings::delQuotes(otaSettings["url_log"]);
      s += "&log=";
      s += String(S_Common::S_Common::getUTime() - lastCheckUpdate);
#ifdef ESP8266
      t_httpUpdate_return ret = ESPhttpUpdate.update(wifiClient, update_url.c_str(), buildVersion);
#endif
#ifdef ESP32
      WiFiClient espClient;
      t_httpUpdate_return ret = httpUpdate.update(espClient, OTAWEB_URL);
#endif
      switch (ret) {
        case HTTP_UPDATE_FAILED:
          break;

        case HTTP_UPDATE_NO_UPDATES:
          break;

        case HTTP_UPDATE_OK:
          break;
      }
    }
    lastCheckUpdate = S_Common::S_Common::getUTime();
  }


  //------------------------------getServerVersion-------------
  String S_OTA::getServerVersion(String get_parameter)
  {
    WiFiClient wifiClient;
    HTTPClient http;
    String url_string = S_Settings::delQuotes(otaSettings["url_update"]) + "?info=true&module=" + module_type;
    url_string += get_parameter;
    http.begin(wifiClient, url_string);
    Serial.println("GetServerVersion from " + url_string);
    String playload = "0";
    int httpCode = http.GET();
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      //USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        playload = http.getString();
      } // httpCode ok
      else
      {
        playload = "0";
        Serial.println("HTTP ERROR: " + (String)httpCode);
      }
    }
    http.end();
    return playload;
  }

String S_OTA::getBuildVersion()
{
  String build_version = String(BUILD_YEAR);
  if (BUILD_MONTH < 10) build_version += "0";
  build_version += String(BUILD_MONTH);
  if (BUILD_DAY < 10) build_version += "0";
  build_version += String(BUILD_DAY);
  build_version += "_";
  if (BUILD_HOUR < 10) build_version += "0";
  build_version += String(BUILD_HOUR);
  return build_version;
}

void S_OTA::loop()
{
  unsigned long currentTime = S_Common::S_Common::getUTime();
  if (currentTime - lastCheckUpdate > checkPeriod) {
    autoUpdate();
  }
}