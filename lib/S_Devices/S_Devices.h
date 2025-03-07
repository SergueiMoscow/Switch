#ifndef _S_DEVICES_
#define _S_DEVICES_

#include <Arduino.h>
#include <ArduinoJson.h>
#include "S_Mode.h"
#include "S_FS.h"
#include "S_DS.h"
#include "S_JsonSettings.h" // Заменил S_Settings
#include "S_Common.h"

#define MAX_RELAYS 4

struct RelayConfig {
    String pin;
    String name;
    String description;
    String on; // "LOW" или "HIGH"
    int maxOn; // Максимальное время работы в секундах
};

struct DS18B20Config {
    String pin;
    struct Sensor {
        String name;
        String description;
    };
    Sensor sensors[2]; // Максимум 2 сенсора для примера, можно сделать динамическим позже
    int sensorCount;
};

class S_Devices {
private:
    S_DS* dsInstance;
    int numRelays;
    RelayConfig relays[MAX_RELAYS];
    unsigned long relayTurnedOn[MAX_RELAYS];
    unsigned long relayTurnOff[MAX_RELAYS];
    DS18B20Config dsConfig; // Пока одна конфигурация DS18B20
    bool dsInitialized;

    void initRelay(const JsonObject& device, int index);
    void initDS18B20(const JsonObject& device);
    int getRelayByPin(int pin);
    int getPinByName(const String& name);
    void setTimeToTurnOff(int relay, unsigned long sec, const RelayConfig& config);
    String getDeviceNameFromTopic(const String& topic);
    RelayConfig* getRelayByName(const String& name);

public:
    S_Devices();
    int getPin(const String& pinStr);
    float getTemperature(const String& deviceName, const String& sensorName);
    void changeRelay(int relay, const String& value, const String& caller);
    DynamicJsonDocument getJsonRelayValuesForPublish();
    void callback(const String& topic, const String& value);
    int loop();
    DynamicJsonDocument getJsonSensorValuesForPublish();
    void setupModules();
    DynamicJsonDocument getJsonAllValuesForPublish();
};

#endif