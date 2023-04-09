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
        WiFiClient wifiClient;
        PubSubClient mqttClient;
        unsigned long lastPublished;
        unsigned int periodSec;
        unsigned long lastTryConnect;
        void loop();
        void publish(bool force = false);
        String getRootTopic();
        String clearValue(JSONVar value);
        void setServer();
    public:
        void init();
        String getSubscribeString();
        void connect();

};