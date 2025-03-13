#ifndef _S_MQTT_
#define _S_MQTT_

#define MQTT_MAX_PACKET_SIZE 4096

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "S_JsonSettings.h"
#include "S_FS.h"
#include "S_Devices.h"
#include "S_Common.h"
#include <TimeLib.h>

#define MQTT_SETTINGS_FILE "/mqtt.json"
#define GLOBAL_SETTINGS_FILE "/global.json"
#define SUBSCRIBE_POSTFIX "set"

struct MQTTConfig {
    String server;
    uint16_t port;
    String user;
    String password;
    uint16_t periodSec;
    bool active;
};

class S_MQTT {
private:
    S_JsonSettings mqttSettings;
    S_JsonSettings globalSettings;
    PubSubClient* mqttClient;
    S_Devices* devices;
    MQTTConfig activeConfig;
    unsigned long lastPublished = 0;
    unsigned long lastTryConnect = 0;
    bool isConfigured = false;
    String rootTopic;

    void loadConfig();
    void setRootTopic();
    void setServer();
    bool loadActiveConfig(const JsonObject& obj);

public:
    void init(PubSubClient* client, S_Devices* devices);
    void publish(bool force = false);
    void publish(const String& topic, const String& payload); // Новая перегрузка
    void loop();
    String getSubscribeString();
    void connect();
    bool sendTimeRequest();
    void publishStartupInfo();
};

#endif
