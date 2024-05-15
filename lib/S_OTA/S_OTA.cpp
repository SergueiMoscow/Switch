#include "S_OTA.h"
#include <iostream>

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
    // TODO: Не нужен ???
    String add_parameter = "";
    JSONVar otaSettings = JSON.parse(S_FS::fileContent("ota.json"));
    String serverVersion = getServerVersion(add_parameter);
    String buildVersion = getBuildVersion();
    String update_url = S_Settings::delQuotes(otaSettings["url_update"]);
    WiFiClient wifiClient;
    Serial.println("Server " + serverVersion +" local: " + buildVersion);
    // Если разница между версиями более 10 минут, то грузим
    if (serverVersion > buildVersion)
    {
      Serial.print("AutoUpdate Server version:");
      Serial.println(serverVersion);
      Serial.print("AutoUpdate Build version:");
      Serial.println(buildVersion);
      Serial.println(update_url);
      String url_new_version = update_url + "/get_bin/" + S_Settings::delQuotes(otaSettings["type"]);
      // String s = S_Settings::delQuotes(otaSettings["url_log"]);
      // s += "&log=";
      // s += String(S_Common::S_Common::getUTime() - lastCheckUpdate);
// #ifdef ESP8266
      Serial.println("Executing update: " + url_new_version);

      ESPhttpUpdate.onStart(update_started);
      ESPhttpUpdate.onEnd(update_finished);
      ESPhttpUpdate.onProgress(update_progress);
      ESPhttpUpdate.onError(update_error);

      t_httpUpdate_return ret = ESPhttpUpdate.update(wifiClient, url_new_version.c_str());
      Serial.print("Ret(update): ");
      Serial.println(ret);
// #endif
// #ifdef ESP32
//       t_httpUpdate_return ret = httpUpdate.update(wifiClient, update_url.c_str());
// #endif
    switch (ret) {
      case HTTP_UPDATE_FAILED: Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str()); break;

      case HTTP_UPDATE_NO_UPDATES: Serial.println("HTTP_UPDATE_NO_UPDATES"); break;

      case HTTP_UPDATE_OK: Serial.println("HTTP_UPDATE_OK"); {
        // Если сохранять версию в файле
        // File file = LittleFS.open("version", "w");
        // file.print(serverVersion);
        // file.close();
        break;
      }
    }
    }
    lastCheckUpdate = S_Common::S_Common::getUTime();
  }


  //------------------------------getServerVersion-------------
  String S_OTA::getServerVersion(String get_parameter)
  {
    WiFiClient wifiClient;
    HTTPClient http;
    JSONVar otaSettings = JSON.parse(S_FS::fileContent("ota.json"));
    // String url_string = S_Settings::delQuotes(otaSettings["url_update"]) + "?info=true&module=" + module_type;
    String url_string = S_Settings::delQuotes(otaSettings["url_update"]) + "/get_info/" + S_Settings::delQuotes(otaSettings["type"]);
    url_string += get_parameter;
    http.begin(wifiClient, url_string);
    Serial.println("GetServerVersion from " + url_string);
    int httpCode = http.GET();
    if (httpCode > 0) {
      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String playload = http.getString();
        JSONVar response = JSON.parse(playload); 
        String version = response["version"];
        http.end();
        return version;
      } // httpCode ok
      else
      {
        Serial.println("HTTP ERROR: " + (String)httpCode);
      }
    }
    http.end();
    return "0";
  }

String S_OTA::getBuildVersion()
{
  String file_build_version = S_FS::fileContent("version");
  // Версия, сформированная при компиляции
  String build_version = String(BUILD_YEAR - 2000);
  if (BUILD_MONTH < 10) build_version += "0";
  build_version += String(BUILD_MONTH);
  if (BUILD_DAY < 10) build_version += "0";
  build_version += String(BUILD_DAY);
  build_version += "_";
  if (BUILD_HOUR < 10) build_version += "0";
  build_version += String(BUILD_HOUR);
  if (file_build_version != build_version) {
      File file = LittleFS.open("version", "w");
      file.print(build_version);
      file.close();
  }
  return build_version;
}

void S_OTA::loop()
{
  unsigned long currentTime = S_Common::S_Common::getUTime();
  if (currentTime - lastCheckUpdate > checkPeriod) {
    autoUpdate();
    lastCheckUpdate = S_Common::S_Common::getUTime();
  }
}

void update_started() {
  Serial.println("CALLBACK:  HTTP update process started");
}

void update_finished() {
  Serial.println("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total) {
  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err) {
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}
