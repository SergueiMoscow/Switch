#include "S_Settings.h"
#include <Arduino.h>
#include <Arduino_JSON.h>
// #include <ArduinoJson.h>
#include <S_DS.h>
#include <FS.h>
#include <LittleFS.h>
#include "S_FS.h"
#ifndef COMPONENT_RELIES
#define COMPONENT_RELIES 1
#endif
// #define SETTINGS_FILE "/settings.json"

bool S_Settings::setSettingsFile(String p_settingsFile)
{
  strcpy(settingsFile, p_settingsFile.c_str());
  this->readSettings();
  return true;
}

//--------------- readSettings-------------
void S_Settings::readSettings()
{
  bool debug = true;
  S_FS* fs = new S_FS();
  if (debug) Serial.print("Settings.cpp readSettings ");
  if (debug) Serial.println(settingsFile);
  settings = JSON.parse(fs->readFile(settingsFile));
  delete fs;
}

//--------------- writeSettings-------------
void S_Settings::writeSettings()
{
  Serial.print("SSettings writeSettings point 1, File: ");
  Serial.println(settingsFile);
  File file = LittleFS.open(settingsFile, "w");
  file.print(JSON.stringify(settings));
  Serial.println(JSON.stringify(settings));
  file.close();
}

//--------------------getSetting----------------
String S_Settings::getSetting(String key, String def_value)
{
  // возвращает settings[key] в форме строки БЕЗ КАВЫЧЕК
  // или def_value
  String setting = JSON.stringify(this->settings[key]);
  if (this->settings[key] == null)
    return def_value;
  setting.replace("\"", "");
  //  Serial.print("GetSetting replacement:");
  //  Serial.print(key);
  //  Serial.print("->");
  //  Serial.println(setting);
  return setting;
}

void S_Settings::setSetting(String key, String value)
{
  this->settings[key] = value;
  Serial.print("Setting ");
  Serial.print(key);
  Serial.print(" is set to ");
  Serial.println(this->settings[key]);
}

String S_Settings::stringReplace(String before)
{
  // extern PubSubClient client;
  //  extern String otaWebModuleType;
  //  extern String activeSSID, buildVersion;
  //  extern int relies[4];
  //  extern String getServerVersion(String get_parameter);
  //  extern String getRootTopic();
  //  extern String millis_to_dhms(long millis1);
  //  extern S_DS ds_ctrl;
  //  extern String getTime(String type="A");

  bool debug = false;
  String after = before;
  String str1, str2;
  JSONVar stringReplace;
  // stringReplace["TIME"] = getTime();

  // MQTT
  stringReplace["MQTT_SERVER"] = this->getSetting("MQTT_Server", "");
  stringReplace["MQTT_PORT"] = this->getSetting("MQTT_Port", "");
  stringReplace["MQTT_USER"] = this->getSetting("MQTT_User", "");
  stringReplace["MQTT_PASS"] = this->getSetting("MQTT_Pass", "");
  stringReplace["MQTT_OBJECT"] = this->getSetting("MQTT_Object", "");
  stringReplace["MQTT_ROOM"] = this->getSetting("MQTT_Room", "");
  stringReplace["MQTT_DEVICE"] = this->getSetting("MQTT_Device", "");
  // stringReplace["MQTT_STATUS"] = (client.connected() ? "Connected" : "NOT connected");
  //  stringReplace["MQTT_TOPIC"] = (getRootTopic()+"/set/#");
  stringReplace["MQTT_PERIODFAST"] = this->getSetting("MQTT_PeriodFast", "");
  stringReplace["MQTT_PERIODSLOW"] = this->getSetting("MQTT_PeriodFast", "");
  // stringReplace["MQTT_CONNECTED"] = String(client.state());
  //  WIFI
  String tmp = this->getSetting("Wifi_Ssid", "default");
  stringReplace["WIFI_SSID"] = this->getSetting("Wifi_Ssid", "default");
  Serial.print("SSID: ");
  Serial.println(stringReplace["WIFI_SSID"]);
  stringReplace["WIFI_PASS"] = this->getSetting("Wifi_Pass", "default");
  // stringReplace["WIFI_HOSTNAME"] = this->getSetting("Wifi_HostName", otaWebModuleType);
  // stringReplace["WIFI_CONNECTED"] = (WiFi.status() == WL_CONNECTED ? activeSSID : "not connected");

  // Comfig admin
  stringReplace["CONFIG_USER"] = this->getSetting("Config_User", "admin");
  stringReplace["CONFIG_PASS"] = this->getSetting("Config_Pass", "admin");
// Components
// Relies
// // #ifdef COMPONENT_RELIES >= 1
// //   stringReplace["MQTT_RELY1"] = this->getSetting("MQTT_Rely1", "rely1");
// //   stringReplace["RELY1_NAME"] = this->getSetting("Rely1_Name", "Rely1");
// //   stringReplace["RELY1_COLOR"] = (digitalRead(relies[0]) == LOW ? "red" : "green");
// //   stringReplace["RELY1_STATUS"] = (digitalRead(relies[0]) == LOW ? "ON" : "OFF");
// //   stringReplace["RELY1_SWITCH_TEXT"] = (digitalRead(relies[0]) == LOW ? "Выключить" : "Включить");
// //   stringReplace["RELY1_SWITCH_VALUE"] = (digitalRead(relies[0]) == LOW ? "OFF" : "ON");
// // #endif
// #ifdef COMPONENT_RELIES >=2
//   stringReplace["MQTT_RELY2"] = this->getSetting("MQTT_Rely2", "rely2");
//   stringReplace["RELY2_NAME"] = this->getSetting("Rely2_Name", "Rely2");
//   stringReplace["RELY2_COLOR"] = (digitalRead(relies[1]) == LOW ? "red" : "green");
//   stringReplace["RELY2_STATUS"] = (digitalRead(relies[1]) == LOW ? "ON" : "OFF");
//   stringReplace["RELY2_SWITCH_TEXT"] = (digitalRead(relies[1]) == LOW ? "Выключить" : "Включить");
//   stringReplace["RELY2_SWITCH_VALUE"] = (digitalRead(relies[1]) == LOW ? "OFF" : "ON");
// #endif
// #ifdef COMPONENT_RELIES >=3
//   stringReplace["MQTT_RELY3"] = this->getSetting("MQTT_Rely3", "rely3");
//   stringReplace["RELY3_NAME"] = this->getSetting("Rely3_Name", "Rely3");
//   stringReplace["RELY3_COLOR"] = (digitalRead(relies[2]) == LOW ? "red" : "green");
//   stringReplace["RELY3_STATUS"] = (digitalRead(relies[2]) == LOW ? "ON" : "OFF");
//   stringReplace["RELY3_SWITCH_TEXT"] = (digitalRead(relies[2]) == LOW ? "Выключить" : "Включить");
//   stringReplace["RELY3_SWITCH_VALUE"] = (digitalRead(relies[2]) == LOW ? "OFF" : "ON");
// #endif
#ifdef COMPONENT_WATT_COUNTER
  stringReplace["Period1_Begin"] = this->getSetting("Period1_Begin", "0");
  stringReplace["Period1_End"] = this->getSetting("Period1_End", "7");
  stringReplace["Period2_Begin"] = this->getSetting("Period2_BEGIN", "7");
  stringReplace["Period2_End"] = this->getSetting("Period2_End", "10");
  stringReplace["Period3_Begin"] = this->getSetting("Period3_BEGIN", "10");
  stringReplace["Period3_End"] = this->getSetting("Period3_End", "17");
  stringReplace["Period4_Begin"] = this->getSetting("Period4_BEGIN", "17");
  stringReplace["Period4_End"] = this->getSetting("Period4_End", "21");
  stringReplace["Period5_Begin"] = this->getSetting("Period5_BEGIN", "21");
  stringReplace["Period5_End"] = this->getSetting("Period5_End", "23");
  stringReplace["Period6_Begin"] = this->getSetting("Period6_BEGIN", "23");
  stringReplace["Period6_End"] = this->getSetting("Period6_End", "24");
  stringReplace["Period1_Mode"] = this->getSetting("Period1_Mode", "3");
  stringReplace["Period2_Mode"] = this->getSetting("Period2_Mode", "1");
  stringReplace["Period3_Mode"] = this->getSetting("Period3_Mode", "2");
  stringReplace["Period4_Mode"] = this->getSetting("Period4_Mode", "1");
  stringReplace["Period5_Mode"] = this->getSetting("Period5_Mode", "2");
  stringReplace["Period6_Mode"] = this->getSetting("Period6_Mode", "3");
#endif
  // stringReplace["TIMEUP"] = millis_to_dhms(millis());
  // DS
  // for(int i=0;i<ds_ctrl.col_ds();i++)
  //   stringReplace["DS"+String(i)+"_VALUE"] = ds_ctrl.getTemp(i);

  // Hasta ahi NO se reconnecta MQTT

  //    stringReplace["ESP32_VALUE"] = getExtTemp(); //getTempESP();
  //    stringReplace["TEMP1_VALUE"] = p_dhtTemp;
  //    stringReplace["TEMP2_VALUE"] = p_dsTemp;
  //    stringReplace["HUM_VALUE"] = p_humidity;
  //    stringReplace["SMOKE_VALUE"] = p_smoke;

  // Module type, version
  // stringReplace["MODULE"] = otaWebModuleType;
  // stringReplace["BUILD_VERSION"] = buildVersion;
  // Проблемная строка (перезагрузка MQTT)
  // stringReplace["SERVER_VERSION"] = getServerVersion("");

#ifdef COMPONENT_BLYNK
  stringReplace["BLYNK_TOKEN"] = this->getSetting("Blynk_Token", "");
#endif

  stringReplace["URL_EXEC_STRING"] = this->getSetting("URL_Exec_String", "");
  stringReplace["URL_EXEC_PERIOD"] = this->getSetting("URL_Exec_Period", "");

  JSONVar keys = stringReplace.keys();
  JSONVar tmp2;
  JSONVar strTmp;
  debug = false;
  if (debug)
  {
    Serial.print("Key length (stringReplace):");
    Serial.println(keys.length());
  }
  for (int i = 0; i < keys.length(); i++)
  {
    str1 = "%";
    str1 += JSON.stringify(keys[i]);
    str1 += "%";
    str1.replace("\"", "");
    tmp2 = (keys[i]);
    strTmp = stringReplace[tmp2];
    str2 = JSON.stringify(stringReplace[tmp2]);
    str2.replace("\"", "");
    if (debug)
    {
      Serial.print("Replacing :");
      Serial.print(str1);
      Serial.print(" with ");
      Serial.println(str2);
      Serial.println(strTmp);
    }
    after.replace(str1, str2);
  }
  return after;
}
