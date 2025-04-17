#ifndef _S_DEVICES_
#define _S_DEVICES_

#include <Arduino.h>
#include <ArduinoJson.h>
#include "S_Mode.h"
#include "S_FS.h"
#include "S_DS.h"
#include "S_Relay.h"
#include "S_DHT.h"
#include "S_MQ.h"
#include "S_Buzzer.h"
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

struct BuzzerTrigger {
    String device;
    String parameter;
    String condition;
    float threshold;
};

struct BuzzerConfig {
    String pin;
    String name;
    BuzzerTrigger triggers[3];
    int triggerCount;
};

struct AutomationTrigger {
    String relayName;
    String device;
    String parameter;
    String condition;
    float threshold;
    String action;
    bool active;
};

class S_Devices {
private:
    S_DS* dsInstance;
    S_Relay* relayInstance;
    S_DHT* dhtInstance;
    S_MQ* mqInstance;
    S_Buzzer* buzzerInstance;
    DS18B20Config dsConfig;
    DHTConfig dhtConfig;
    MQConfig mqConfig;
    BuzzerConfig buzzerConfig;
    AutomationTrigger triggers[8];
    int triggerCount;
    bool dsInitialized;
    bool dhtInitialized;
    bool mqInitialized;
    bool buzzerInitialized;

    void initDS18B20(const JsonObject& device);
    void initDHT(const JsonObject& device);
    void initMQ(const JsonObject& device);
    void initBuzzer(const JsonObject& device);
    static const String& getDeviceNameFromTopic(const String& topic);
    void buzzerLoop();

public:
    S_Devices();
    // int getPin(const String& pinStr);
    float getTemperature(const String& deviceName, const String& sensorName);
    DynamicJsonDocument getJsonRelayValuesForPublish();
    void callback(const String& topic, const String& value);
    int loop();
    DynamicJsonDocument getJsonSensorValuesForPublish();
    void setupModules();
    DynamicJsonDocument getJsonAllValuesForPublish();
    int processAutomation(DynamicJsonDocument& sensorValues, DynamicJsonDocument& relayValues, void (*publish)(const String&, const String&));
};

#endif