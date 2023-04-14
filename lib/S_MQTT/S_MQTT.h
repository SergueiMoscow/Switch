#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "S_Settings.h"
#include "S_FS.h"
#include "S_Devices.h"
#include "S_Common.h"

#define MQTT_SETTINGS_FILE "/mqtt.json"
#define GLOBAL_SETTINGS_FILE "/global.json"
#define SUBSCRIBE_POSTFIX "set"

class S_MQTT
{
    private:
        JSONVar mqttSettings;
        JSONVar globalSettings;
        PubSubClient* mqttClient;
        S_Devices* devices;
        unsigned long lastPublished;
        unsigned int periodSec;
        unsigned long lastTryConnect;
        bool isConfigured = false;
        void publish(bool force);
        void setRootTopic();
        String rootTopic;
        String clearValue(JSONVar value);
        String clearValue(JSONVar value, String default_value);
        void setServer();
        String mqttServer;
    public:
        // S_MQTT(PubSubClient* client, S_Devices* devices);
        void loop();
        void init(PubSubClient* client, S_Devices* devices);
        // void init();
        String getSubscribeString();
        void connect();

};