#ifndef _S_MODE_H_
#define _S_MODE_H_

#include <ArduinoJson.h>
#include <Arduino.h>
#include "S_FS.h"

// Перечисление режимов устройства
enum DeviceMode {
    MODE_CONFIG_WIFI,
    MODE_CONFIG_MQTT,
    MODE_CONFIG_OTA,
    MODE_NORMAL
};

class S_Mode {
private:
    // Статические переменные
    static DeviceMode mode;          
    static String cause;             
    static const char* modeFile;     

    // Приватный метод для сохранения состояния в файл
    static void saveModeToFile();

public:
    // Метод инициализации
    static void begin();

    // Методы получения и установки режима
    static DeviceMode getCurrentMode();
    static void setConfigWifiMode(const String& cause);
    static void setConfigMQTTMode(const String& cause);
    static void setConfigOTAMode(const String& cause);
    static void setNormalMode(const String& cause);
};

#endif // _S_MODE_H_