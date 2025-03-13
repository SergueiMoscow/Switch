#include "S_MQTT.h"

// Статическая обёртка для передачи в processAutomation
static S_MQTT* mqttInstance = nullptr;
static void publishWrapper(const String& topic, const String& payload) {
    if (mqttInstance) {
        mqttInstance->publish(topic, payload);
    }
}

void S_MQTT::init(PubSubClient* client, S_Devices* devicesPtr) {
    mqttClient = client;
    devices = devicesPtr;
    mqttInstance = this; // Устанавливаем глобальный указатель
    Serial.println("S_MQTT.cpp: Initializing...");
    loadConfig();
}

void S_MQTT::loadConfig() {
    S_FS fs;
    Serial.println("S_MQTT.cpp: Reading MQTT settings from " MQTT_SETTINGS_FILE);

    if (!fs.exists(MQTT_SETTINGS_FILE)) {
        S_Mode::setConfigMQTTMode("No MQTT settings found");
        return;
    }

    DynamicJsonDocument doc = S_FS::readJsonFileDynamic(MQTT_SETTINGS_FILE);
    if (doc.isNull()) {
        Serial.println("Failed to load MQTT settings");
        return;
    }

    bool found = false;
    for (JsonPair kv : doc.as<JsonObject>()) {
        JsonObject settings = kv.value().as<JsonObject>();
        Serial.printf("Found MQTT config: %s\n", kv.key().c_str());
        if (loadActiveConfig(settings)) {
            found = true;
            break;
        }
    }

    if (found) {
        setServer();
        setRootTopic();
        connect();
        Serial.println("MQTT settings loaded successfully");
    } else {
        Serial.println("No active MQTT configuration found");
    }
}

bool S_MQTT::loadActiveConfig(const JsonObject& obj) {
    Serial.println("loadActiveConfig: Raw JSON object: ");
    serializeJson(obj, Serial);
    Serial.println();
    Serial.print("Object Active: ");
    Serial.println(obj["Active"].as<String>());

    if (obj["Active"].as<int>() == 1) {
        activeConfig.server = obj["Server"] | "";
        activeConfig.port = obj["Port"] | 1883;
        activeConfig.user = obj["User"] | "";
        activeConfig.password = obj["Password"] | "";
        activeConfig.periodSec = obj["Period"] | 10;
        activeConfig.active = true;

        if (activeConfig.periodSec < 10) {
            activeConfig.periodSec = 10;
        }
        Serial.printf("Loaded active MQTT config: %s, Port: %d, Period: %d\n",
                      activeConfig.server.c_str(), activeConfig.port, activeConfig.periodSec);
        return true;
    }
    return false;
}

void callback(char* topic, byte* msg, unsigned int len) {
    extern S_Devices devices;
    extern S_MQTT sMQTT;

    String topicStr = String(topic);
    Serial.println("Callback: " + topicStr + ": ");

    if (!msg || len == 0) {
        Serial.println("Invalid message received");
        return;
    }

    String message = String((char*)msg).substring(0, len);
    Serial.println("Callback: " + topicStr + ": " + message);

    if (topicStr.endsWith("/set/time")) {
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, message);
        if (!error && doc.containsKey("unixtime")) {
            time_t unixTime = doc["unixtime"].as<long>();
            setTime(unixTime);
            Serial.println("Time set to: " + String(unixTime));
        } else {
            Serial.println("Failed to parse time JSON or no unixtime field");
        }
    } else {
        devices.callback(topicStr, message);
        sMQTT.publish(false);
    }
}

void S_MQTT::setServer() {
    if (activeConfig.server.isEmpty()) {
        Serial.println("MQTT Server is not configured");
        return;
    }
    Serial.printf("Setting MQTT server: %s, Port: %d\n", activeConfig.server.c_str(), activeConfig.port);
    mqttClient->setServer(activeConfig.server.c_str(), activeConfig.port);
    mqttClient->setKeepAlive(60);
    mqttClient->setCallback(callback);
}

void S_MQTT::connect() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected, skipping MQTT connect");
        return;
    }

    String clientId = WiFi.macAddress();
    String user = activeConfig.user.isEmpty() ? "user" : activeConfig.user;
    String password = activeConfig.password.isEmpty() ? "pass" : activeConfig.password;

    Serial.printf("Connecting to MQTT with ID: %s, User: %s\n", clientId.c_str(), user.c_str());
    if (lastTryConnect == 0 || millis() - lastTryConnect > 10000UL) {
        if (mqttClient->connect(clientId.c_str(), user.c_str(), password.c_str())) {
            publish(false);
            String subscribeString = getSubscribeString();
            Serial.println("Subscribing to: " + subscribeString);
            mqttClient->subscribe(subscribeString.c_str());
            publishStartupInfo();
            isConfigured = true;
        } else {
            Serial.printf("Failed to connect to MQTT, state: %d\n", mqttClient->state());
        }
        lastTryConnect = millis();
    }
}

String S_MQTT::getSubscribeString() {
    return rootTopic + SUBSCRIBE_POSTFIX + "/#";
}

void S_MQTT::setRootTopic() {
    globalSettings.setSettingsFile(GLOBAL_SETTINGS_FILE);
    String object = globalSettings.getSetting("object", "default");
    String room = globalSettings.getSetting("room", "default");
    String device = globalSettings.getSetting("device", "default");
    rootTopic = object + "/" + room + "/" + device + "/";
    Serial.println("Root topic set to: " + rootTopic);
}

void S_MQTT::publish(const String& topic, const String& payload) {
    if (!mqttClient->connected()) return;
    mqttClient->publish(topic.c_str(), payload.c_str(), true);
}

void S_MQTT::publish(bool force) {
    if (!mqttClient->connected() && !force) return;

    String publishTopic = rootTopic + "state";
    DynamicJsonDocument devicesDoc = devices->getJsonAllValuesForPublish();
    JsonObject devicesValues = devicesDoc.as<JsonObject>();
    String payload;
    serializeJson(devicesValues, payload);
    Serial.println("Publishing: " + publishTopic + " " + payload);
    mqttClient->publish(publishTopic.c_str(), payload.c_str(), true);

    DynamicJsonDocument relaysDoc = devices->getJsonRelayValuesForPublish();
    JsonObject relayValues = relaysDoc.as<JsonObject>();
    for (JsonPair kv : relayValues) {
        String topic = rootTopic + kv.key().c_str();
        String value = kv.value().as<String>();
        mqttClient->publish(topic.c_str(), value.c_str(), true);
    }

    DynamicJsonDocument sensorsDoc = devices->getJsonSensorValuesForPublish();
    JsonObject sensorValues = sensorsDoc.as<JsonObject>();
    if (!sensorValues.isNull()) {
        for (JsonPair deviceKv : sensorValues) {
            String deviceName = deviceKv.key().c_str();
            JsonObject sensors = deviceKv.value().as<JsonObject>();
            for (JsonPair sensorKv : sensors) {
                String sensorName = sensorKv.key().c_str();
                String topic = rootTopic + deviceName + "/" + sensorName;
                String value = sensorKv.value().as<String>();
                mqttClient->publish(topic.c_str(), value.c_str(), true);
            }
        }
    } else {
        Serial.println("S_MQTT.cpp: No sensor data available");
    }
}

void S_MQTT::loop() {
    unsigned long curMillis = millis();
    if (max(curMillis, lastPublished) - min(curMillis, lastPublished) >= activeConfig.periodSec * 1000UL)
    {
        if (isConfigured && !mqttClient->connected()) {
            Serial.println("MQTT not connected, attempting to reconnect");
            connect();
        }
        S_Common::S_Common::getUTime();
        if (mqttClient->connected()) {
            Serial.println("S_MQTT.cpp loop publishing: " + S_Common::S_Common::getTime());
            DynamicJsonDocument sensorValues = devices->getJsonSensorValuesForPublish();
            DynamicJsonDocument relayValues = devices->getJsonRelayValuesForPublish();
            int automationResult = devices->processAutomation(sensorValues, relayValues, publishWrapper);
            if (automationResult > 0) {
                Serial.println("Automation triggered " + String(automationResult) + " changes");
            }
            publish(false);
            lastPublished = curMillis;
        }
    }
    mqttClient->loop();
    if (devices->loop() > 0) {
        publish(false);
    }
}

bool S_MQTT::sendTimeRequest() {
    String publishTopic = rootTopic + "gettime";
    mqttClient->publish(publishTopic.c_str(), "", false);
    return now() > getBuildUnixTime();
}

void S_MQTT::publishStartupInfo() {
    if (!mqttClient->connected()) {
        Serial.println("MQTT not connected, skipping publishStartupInfo");
        return;
    }
    String devicesFile = "/devices.json";
    // Отправляем базовую информацию
    DynamicJsonDocument basicDoc(256); // Маленький буфер для базовых данных
    JsonObject basicRoot = basicDoc.to<JsonObject>();
    basicRoot["IP"] = WiFi.localIP().toString();
    basicRoot["topic"] = rootTopic;
    basicRoot["online"] = millis() % 1000;

    String basicPayload;
    serializeJson(basicRoot, basicPayload);
    String startupTopic = rootTopic + "startup";
    Serial.println("Publishing basic info to: " + startupTopic);
    Serial.println("Payload: " + basicPayload);
    mqttClient->publish(startupTopic.c_str(), basicPayload.c_str(), true);

    S_FS fs;
    // Отправляем devices отдельно
    if (fs.exists(devicesFile.c_str())) {
        File file = LittleFS.open(devicesFile, "r");
        if (file) {
            size_t devicesFileSize = file.size();
            file.close();

            DynamicJsonDocument devicesDoc = S_FS::readJsonFileDynamic(devicesFile.c_str());
            if (!devicesDoc.isNull()) {
                String devicesPayload;
                serializeJson(devicesDoc, devicesPayload);
                String devicesTopic = rootTopic + "startup/devices";
                Serial.println("Publishing devices to: " + devicesTopic);
                Serial.println("Payload: " + devicesPayload);
                mqttClient->publish(devicesTopic.c_str(), devicesPayload.c_str(), true);
            }
        }
    }
}
