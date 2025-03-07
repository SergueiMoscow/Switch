#ifndef _S_DEVICES_
#define _S_DEVICES_

#include <Arduino.h>
#include <ArduinoJson.h>
#include "S_Mode.h"
#include "S_FS.h"
#include "S_DS.h"
#include "S_Relay.h"
#include "S_JsonSettings.h"
#include "S_Common.h"

struct DS18B20Config {
    String pin;
    struct Sensor {
        String name;
        String description;
    };
    Sensor sensors[2]; // Максимум 2 сенсора
    int sensorCount;
};

class S_Devices {
private:
    S_DS* dsInstance;
    S_Relay* relayInstance;
    DS18B20Config dsConfig;
    bool dsInitialized;

    void initDS18B20(const JsonObject& device);
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
