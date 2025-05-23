#ifndef _SSETTINGS_
#define _SSETTINGS_
#include <Arduino.h>
#include <Arduino_JSON.h>
#include <LittleFS.h>
#include "S_FS.h"
#include "S_Mode.h"

  class S_Settings
  {
  private:
    JSONVar settings;
    char settingsFile[20];

  public:
    void readSettings();
    void writeSettings();
    String getSetting(String key, String def_value);
    void setSetting(String key, String value);
    bool setSettingsFile(String settingsFile);
    static String delQuotes(JSONVar value);
    static String delQuotes(String value);
    String stringReplace(String before);
  };
#endif
