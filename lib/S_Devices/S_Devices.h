#ifndef _S_DEVICES_
#define _S_DEVICES_

#include <Arduino.h>
#include <ArduinoJson.h>
#include "S_Mode.h"
#include "S_FS.h"
#include "S_DS.h"
#include "S_Relay.h"
#include "S_DHT.h"
#include "S_MQ.h" // Добавили новый класс
#include "S_JsonSettings.h"
#include "S_Common.h"

struct DS18B20Config {
    String pin;
    String name;
    struct Sensor {
        String name;
        String description;
    };
    Sensor sensors[2];
    int sensorCount;
};

struct DHTConfig {
    String pin;
    String name;
    String description;
};

struct MQConfig {
    String pin;
    String name;
    String description;
};

class S_Devices {
private:
    S_DS* dsInstance;
    S_Relay* relayInstance;
    S_DHT* dhtInstance;
    S_MQ* mqInstance; // Добавили экземпляр MQ
    DS18B20Config dsConfig;
    DHTConfig dhtConfig;
    MQConfig mqConfig;
    bool dsInitialized;
    bool dhtInitialized;
    bool mqInitialized;

    void initDS18B20(const JsonObject& device);
    void initDHT(const JsonObject& device);
    void initMQ(const JsonObject& device);
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