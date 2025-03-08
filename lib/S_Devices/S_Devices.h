#ifndef _S_DEVICES_
#define _S_DEVICES_

#include <Arduino.h>
#include <ArduinoJson.h>
#include "S_Mode.h"
#include "S_FS.h"
#include "S_DS.h"
#include "S_Relay.h"
#include "S_DHT.h" // Добавили новый класс
#include "S_JsonSettings.h"
#include "S_Common.h"

struct DS18B20Config {
    String pin;
    String name; // Добавили имя устройства
    struct Sensor {
        String name;
        String description;
    };
    Sensor sensors[2]; // Максимум 2 сенсора
    int sensorCount;
};

struct DHTConfig {
    String pin;
    String name; // Имя устройства
    String description;
};

class S_Devices {
private:
    S_DS* dsInstance;
    S_Relay* relayInstance;
    S_DHT* dhtInstance; // Добавили экземпляр DHT
    DS18B20Config dsConfig;
    DHTConfig dhtConfig;
    bool dsInitialized;
    bool dhtInitialized;

    void initDS18B20(const JsonObject& device);
    void initDHT(const JsonObject& device);
    static const String& getDeviceNameFromTopic(const String& topic);

public:
    S_Devices();
    int getPin(const String& pinStr);
    float getTemperature(const String& deviceName, const String& sensorName);
    DynamicJsonDocument getJsonRelayValuesForPublish();
    void callback(const String& topic, const String& value);
    int loop();
    DynamicJsonDocument getJsonSensorValuesForPublish();
    void setupModules();
    DynamicJsonDocument getJsonAllValuesForPublish();
};

#endif