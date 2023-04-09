#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "S_Settings.h"
#include "S_FS.h"

#define MQTT_SETTINGS_FILE "/mqtt.json"
#define GLOBAL_SETTINGS_FILE "/global.json"
#define SUBSCRIBE_POSTFIX "set"

class S_MQTT
{
    private:
        JSONVar mqttSettings;
        PubSubClient* mqttClient;
        unsigned long lastPublished;
        unsigned int periodSec;
        unsigned long lastTryConnect;
        bool isConfigured = false;
        void publish(bool force);
        String getRootTopic();
        String clearValue(JSONVar value);
        String clearValue(JSONVar value, String default_value);
        void setServer();
        String mqttServer;
    public:
        void loop();
        void init(PubSubClient* client);
        String getSubscribeString();
        void connect();

};