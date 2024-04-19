#include "S_MQTT.h"

// S_MQTT::S_MQTT()
// {
//     init();
// }

// S_MQTT::S_MQTT(PubSubClient* client, S_Devices* devicesPtr)
// {
//     mqttClient = client;
//     devices = devicesPtr;
// }

// void S_MQTT::init(PubSubClient *client, S_Devices *devicesPtr)
void S_MQTT::init(PubSubClient* client, S_Devices* devicesPtr)
{
    mqttClient = client;
    devices = devicesPtr;
    loadConfig();
}

void S_MQTT::loadConfig()
{
    S_FS fs = S_FS();
    bool found = false;
    Serial.println("S_MQTT.cpp reading mqtt settings");
    JSONVar mqttJson = JSON.parse(fs.readFile(MQTT_SETTINGS_FILE));
    if (JSON.stringify(mqttJson).length() < 10)
        return;
    JSONVar keys = mqttJson.keys();
    for (int i = 0; i < keys.length(); i++)
    {
        Serial.println("Checking MQTT " + JSON.stringify(keys[i]));
        if (clearValue(mqttJson[keys[i]]["Active"]) == "1")
        {
            mqttSettings = mqttJson[keys[i]];
            periodSec = atoi(clearValue(mqttJson[keys[i]]["Period"]).c_str());
            if (periodSec < 10) {
                periodSec = 10;
            }
            Serial.println("PeriodSec: " + (String)periodSec);
            found = true;
            break;
        }
    }
    if (found)
    {
        setServer();
        setRootTopic();
        connect();
    }
    else
    {
        Serial.println("Active MQTT is not configured");
    }
}

void callback(char *topic, byte *msg, unsigned int len)
{
    extern S_Devices devices;
    extern S_MQTT sMQTT;
    String message;
    for (int i = 0; i < len; i++)
    {
        message += (char)msg[i];
    }
    Serial.print("Callback: " + (String)topic + ": " + message);
    devices.callback((String)topic, message);
    sMQTT.publish(false);
    return;
}

void S_MQTT::setServer()
{
    mqttServer = clearValue(mqttSettings["Server"]);
    const char *server = mqttServer.c_str();
    if (server == "")
    {
        Serial.println("MQTT Server is not configured");
        loadConfig();
        return;
    }
    int port = atoi(clearValue(mqttSettings["Port"]).c_str());
    Serial.print("Trying to set MQTT server: \"");
    Serial.print(server);
    Serial.println("\" port: " + (String)port);
    mqttClient->setServer(server, port);
    mqttClient->setCallback(callback);
}

void S_MQTT::connect()
{
    JSONVar jUser, jPass;
    jUser = mqttSettings["User"];
    jPass = mqttSettings["Password"];
    if (jUser == null || jPass == null)
    {
        Serial.println("User or password is not configured");
        Serial.println(JSON.stringify(mqttSettings));
        loadConfig();
        return;
    }
    String clientId = WiFi.macAddress().c_str();
    String user = clearValue(mqttSettings["User"], "user").c_str();
    String password = clearValue(mqttSettings["Password"], "pass").c_str();
    if (user == "")
        user = "user";
    if (password == "")
        password = "pass";
    Serial.print("LastTryConnect: ");
    Serial.println(lastTryConnect);
    Serial.print("Passed: ");
    Serial.print(millis() - lastTryConnect);
    Serial.println("id: " + clientId + ", user: " + user + ", pass: " + password);
    if (lastTryConnect == 0 || millis() - lastTryConnect > 10000UL)
    {
        if (mqttClient->connect(clientId.c_str(), user.c_str(), password.c_str()), true)
        {
            Serial.println("Connected to MQTT ");
            publish(false);
            String subscribeString = getSubscribeString();
            Serial.print("Subscribe to: " + subscribeString);
            mqttClient->subscribe(subscribeString.c_str());
            isConfigured = true;
        }
        else
        {
            Serial.println("Couldn't connect to MQTT");
        }
        lastTryConnect = millis();
    }
}

String S_MQTT::clearValue(JSONVar value)
{
    String result = JSON.stringify(value);
    result.replace("\"", "");
    // result.toLowerCase();
    // result.trim();
    return result;
}

String S_MQTT::clearValue(JSONVar value, String default_value)
{
    String result = JSON.stringify(value);
    if (result == null)
        return default_value;
    result.replace("\"", "");
    // result.toLowerCase();
    // result.trim();
    return result;
}

String S_MQTT::getSubscribeString()
{
    return rootTopic + SUBSCRIBE_POSTFIX + "/#";
}

void S_MQTT::setRootTopic()
{
    S_Settings globalSettings;
    Serial.print("S_MQTT setSettingsFile (Global)");
    globalSettings.setSettingsFile(GLOBAL_SETTINGS_FILE);
    String object, room, device;
    object = globalSettings.getSetting("object", "default");
    room = globalSettings.getSetting("room", "default");
    device = globalSettings.getSetting("device", "default");
    rootTopic = object + "/" + room + "/" + device + "/";
}

void S_MQTT::publish(bool force)
{
    String publishTopic = rootTopic + "state";
    JSONVar devicesValues = devices->getJsonRelayValuesForPublish();
    devicesValues["time"] = S_Common::S_Common::getUTime();
    // for IoT нужно ли???
    // devicesValues["order"] = (hour() * 60 * 60) + (minute() * 60) + second();
    // devicesValues["id"] = rootTopic;
    // devicesValues["pageId"] = (String)year() + String(month()) + String(day());
    Serial.println("Mqtt.publish: " + publishTopic + JSON.stringify(devicesValues));

    mqttClient->publish((publishTopic).c_str(), JSON.stringify(devicesValues).c_str(), true);
    JSONVar keys = devicesValues.keys();
    for (int i = 0; i < keys.length(); i++) {
        String topic = rootTopic + clearValue(keys[i]);
        String value = clearValue(devicesValues[keys[i]]);
        mqttClient->publish(topic.c_str(), value.c_str(), true);
    }

}

void S_MQTT::loop()
{
    unsigned long curMillis = millis();
    if (max(curMillis, lastPublished) - min(curMillis, lastPublished) > periodSec * 1000)
    {
        if (isConfigured && !mqttClient->connected())
        {
            Serial.println("MQTT not connected");
            loadConfig();
            connect();
        }
        S_Common::S_Common::getUTime();
        Serial.println(S_Common::S_Common::getTime());
        if (mqttClient->connected())
        {
            publish(false);
            lastPublished = millis();
        }
    }
    mqttClient->loop();
    if (devices->loop() > 0) {
        publish(false);
    }
}
