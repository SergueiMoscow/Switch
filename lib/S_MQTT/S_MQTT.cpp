#include "S_MQTT.h"

S_MQTT::S_MQTT()
{
    init();
}

void S_MQTT::init()
{
    S_FS fs = S_FS();
    JSONVar mqttJson = JSON.parse(fs.readFile(MQTT_SETTINGS_FILE));
    JSONVar keys = mqttJson.keys();
    bool found = false;
    for (int i = 0; i < keys.length(); i++)
    {
        if (clearValue(mqttJson[keys[i]["active"]]) == "1")
        {
            mqttSettings = mqttJson[keys[i]];
            periodSec = atoi(clearValue(mqttJson[keys[i]]["period"]).c_str());
            found = true;
        }
    }
    if (found) {
        setServer();
    }
}

void S_MQTT::setServer()
{
    const char* server = clearValue(mqttSettings["server"]).c_str();
    int port = atoi(clearValue(mqttSettings["port"]).c_str());
    mqttClient.setServer(server, port);
}

void S_MQTT::connect()
{
    const char* clientId = WiFi.macAddress().c_str();
    const char* user = clearValue(mqttSettings["user"]).c_str();
    const char* password = clearValue(mqttSettings["password"]).c_str();
    if (lastTryConnect == 0 || millis() - lastTryConnect > 10000) {
        mqttClient.connect(clientId, user, password);
        lastTryConnect = millis();
    }
}

String S_MQTT::clearValue(JSONVar value)
{
    String result = JSON.stringify(value);
    result.replace("\"", "");
    result.toLowerCase();
    result.trim();
    return result;
}

String S_MQTT::getSubscribeString()
{
    return getRootTopic() + SUBSCRIBE_POSTFIX + "/#";
}

String S_MQTT::getRootTopic()
{
    S_Settings globalSettings;
    globalSettings.setSettingsFile(GLOBAL_SETTINGS_FILE);
    String object, room, device;
    object = globalSettings.getSetting("object");
    room = globalSettings.getSetting("room");
    device = globalSettings.getSetting("device");
    return object + "/" + room + "/" + device + "/";
}

void S_MQTT::publish(bool force = false)
{
}

void S_MQTT::loop()
{
    if (!mqttClient.connected())
    {
        connect();
    }
    unsigned long curMillis = millis();
    if (max(curMillis, lastPublished) - min(curMillis, lastPublished) > periodSec * 1000)
    {
        publish(false);
        lastPublished = millis();
    }
    mqttClient.loop();
}
