#include "S_JsonSettings.h"

bool S_JsonSettings::setSettingsFile(const String& p_settingsFile) {
    strncpy(settingsFile, p_settingsFile.c_str(), sizeof(settingsFile) - 1);
    settingsFile[sizeof(settingsFile) - 1] = '\0'; // Гарантируем завершение строки
    readSettings();
    return true;
}

void S_JsonSettings::loadDefaultSettings() {
    settingsDoc.clear();
    // Здесь можно задать значения по умолчанию, если нужно
}

void S_JsonSettings::readSettings() {
    S_FS fs;
    Serial.printf("Reading settings from %s\n", settingsFile);

    if (!fs.exists(settingsFile)) {
        Serial.printf("Settings file %s does not exist\n", settingsFile);
        S_Mode::setConfigMQTTMode("Settings file does not exist");
        loadDefaultSettings();
        return;
    }

    File file = LittleFS.open(settingsFile, "r");
    if (!file) {
        Serial.println("Failed to open settings file");
        loadDefaultSettings();
        return;
    }

    DeserializationError error = deserializeJson(settingsDoc, file);
    file.close();

    if (error) {
        Serial.printf("Failed to parse JSON: %s\n", error.c_str());
        loadDefaultSettings();
    }
}

void S_JsonSettings::writeSettings() {
    Serial.printf("Writing settings to %s\n", settingsFile);
    File file = LittleFS.open(settingsFile, "w");
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    serializeJson(settingsDoc, file);
    file.close();
}

String S_JsonSettings::getSetting(const String& key, const String& defaultValue) {
    if (!settingsDoc.containsKey(key)) {
        return defaultValue;
    }
    return removeQuotes(settingsDoc[key].as<String>());
}

void S_JsonSettings::setSetting(const String& key, const String& value) {
    settingsDoc[key] = value;
    Serial.printf("Setting %s to %s\n", key.c_str(), value.c_str());
}

String S_JsonSettings::stringReplace(const String& input) {
    String result = input;

    // Пример замены для MQTT
    result.replace("%MQTT_SERVER%", getSetting("MQTT_Server", ""));
    result.replace("%MQTT_PORT%", getSetting("MQTT_Port", ""));
    result.replace("%MQTT_USER%", getSetting("MQTT_User", ""));
    result.replace("%MQTT_PASS%", getSetting("MQTT_Pass", ""));
    // Добавить остальные замены из S_Settings, когда будет нужно

    return result;
}

String S_JsonSettings::removeQuotes(const String& value) {
    String result = value;
    result.replace("\"", "");
    return result;
}