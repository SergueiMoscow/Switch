#ifndef _SJSONSETTINGS_
#define _SJSONSETTINGS_
#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "S_FS.h"
#include "S_Mode.h"

class S_JsonSettings {
private:
    DynamicJsonDocument settingsDoc{1024}; // Размер можно настроить под твои JSON-файлы
    char settingsFile[20];

    void loadDefaultSettings(); // Новый метод для инициализации по умолчанию

public:
    S_JsonSettings() = default;
    bool setSettingsFile(const String& filePath);
    void readSettings();
    void writeSettings();
    String getSetting(const String& key, const String& defaultValue = "");
    void setSetting(const String& key, const String& value);
    String stringReplace(const String& input);
    static String removeQuotes(const String& value);
};

#endif