#include "S_MQTT.h"

void S_MQTT::init(PubSubClient* client, S_Devices* devicesPtr)
{
    mqttClient = client;
    devices = devicesPtr;
    Serial.println("S_MQTT.cpp loadConfig from init");
    loadConfig();
}

void S_MQTT::loadConfig()
{
    S_FS fs = S_FS();
    bool found = false;
    Serial.println("S_MQTT.cpp reading mqtt settings");
    if (!S_FS::exists(MQTT_SETTINGS_FILE)) {
        S_Mode::setConfigMQTTMode("No MQTT settings found");
        return;
    }
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
        Serial.println("MQTT settings loaded");
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
    
    String topicStr = String(topic);
    Serial.println("Callback: " + topicStr + ": ");

    // Проверка на валидность входных данных
    if (!msg || len == 0) {
        Serial.println("Invalid message received");
        return;
    }

    String message = String((char*)msg).substring(0, len);
    Serial.println("Callback: " + topicStr + ": " + message);

    if (topicStr.endsWith("/set/time")) {
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, message);
        
        if (!error) {
            if (doc.containsKey("unixtime")) {
                time_t unixTime = doc["unixtime"].as<long>();
                setTime(unixTime);
                Serial.println("Time set to: " + String(unixTime));
            } else {
                Serial.println("No unixtime field in JSON");
            }
        } else {
            Serial.println("JSON parse error: " + String(error.c_str()));
        }
    } else {
        devices.callback(topicStr, message);
        sMQTT.publish(false);
    }
}

void S_MQTT::setServer()
{
    mqttServer = clearValue(mqttSettings["Server"]);
    const char *server = mqttServer.c_str();
    if (server == "")
    {
        Serial.println("MQTT Server is not configured");
        // loadConfig();
        return;
    }
    int port = atoi(clearValue(mqttSettings["Port"]).c_str());
    Serial.print("Trying to set MQTT server: \"");
    Serial.print(server);
    Serial.println("\" port: " + (String)port);
    mqttClient->setServer(server, port);
    mqttClient->setKeepAlive(60); // Установить keep-alive 60 секунд
    mqttClient->setCallback(callback);
}

void S_MQTT::connect()
{
    bool debug = false;
    Serial.println("S_MQTT.cpp connect 1");
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected, skipping MQTT connect");
        return;
    }
    Serial.println("S_MQTT.cpp connect 2");
    JSONVar jUser, jPass;
    jUser = mqttSettings["User"];
    jPass = mqttSettings["Password"];
    Serial.println("S_MQTT.cpp connect 3");
    if (jUser == null || jPass == null)
    {
        Serial.println("User or password is not configured");
        Serial.println(JSON.stringify(mqttSettings));
        Serial.println("S_MQTT.cpp loadConfig from connect");
        Serial.println("S_MQTT.cpp connect 4");
        loadConfig();
        return;
    }
    Serial.println("S_MQTT.cpp connect 5");
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
            Serial.println("Subscribe to: " + subscribeString);
            mqttClient->subscribe(subscribeString.c_str());
            isConfigured = true;
        }
        else
        {
            Serial.print("Failed to connect to MQTT, state: ");
            Serial.println(mqttClient->state());
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
    if (max(curMillis, lastPublished) - min(curMillis, lastPublished) >= periodSec * 1000UL)
    {
        if (isConfigured && !mqttClient->connected())
        {
            Serial.println("MQTT not connected");
            Serial.println("S_MQTT.cpp connect from loop");
            // loadConfig();
            connect();
        }
        S_Common::S_Common::getUTime();
        Serial.print("S_MQTT.cpp loop ");
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

bool S_MQTT::sendTimeRequest() {
    String publishTopic = rootTopic + "gettime";
    mqttClient->publish((publishTopic).c_str(), "", false);
    return now() > getBuildUnixTime();
}