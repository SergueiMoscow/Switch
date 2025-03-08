#include "S_Devices.h"

S_Devices::S_Devices() {
    dsInstance = nullptr;
    relayInstance = new S_Relay();
    dhtInstance = nullptr;
    mqInstance = nullptr; // Инициализируем как nullptr
    dsInitialized = false;
    dhtInitialized = false;
    mqInitialized = false;
    setupModules();
}

void S_Devices::setupModules() {
    S_FS fs;
    if (!fs.exists("/devices.json")) {
        S_Mode::setConfigMQTTMode("No devices.json found");
        return;
    }

    DynamicJsonDocument doc = S_FS::readJsonFileDynamic("/devices.json");
    if (doc.isNull()) {
        Serial.println("Error: Failed to parse devices.json");
        return;
    }

    Serial.println("Devices config parsed:");
    serializeJson(doc, Serial);
    Serial.println();

    JsonObject configObj = doc.as<JsonObject>();
    relayInstance->initRelays(configObj);

    for (JsonPair kv : configObj) {
        JsonObject device = kv.value().as<JsonObject>();
        String type = device["type"] | "";
        if (type == "DS18B20" && !dsInitialized) {
            initDS18B20(device);
            dsInitialized = true;
        } else if (type == "DHT" && !dhtInitialized) {
            initDHT(device);
            dhtInitialized = true;
        } else if (type == "MQ" && !mqInitialized) {
            initMQ(device);
            mqInitialized = true;
        }
    }
}

void S_Devices::initDS18B20(const JsonObject& device) {
    Serial.println("S_Devices::initDS18B20 starting for device:");
    serializeJson(device, Serial);
    Serial.println();

    dsConfig.pin = device["pin"] | "";
    dsConfig.name = device["name"] | "DS18B20_Default";
    int pin = getPin(dsConfig.pin);
    dsInstance = &S_DS::getInstance(pin);

    JsonArray sensors = device["sensors"].as<JsonArray>();
    dsConfig.sensorCount = min(int(sensors.size()), 2);
    for (int i = 0; i < dsConfig.sensorCount; i++) {
        dsConfig.sensors[i].name = sensors[i]["name"] | "";
        dsConfig.sensors[i].description = sensors[i]["description"] | "";
    }

    Serial.println("S_Devices::initDS18B20 completed, dsInstance: " + String((unsigned long)dsInstance, HEX));
}

void S_Devices::initDHT(const JsonObject& device) {
    Serial.println("S_Devices::initDHT starting for device:");
    serializeJson(device, Serial);
    Serial.println();

    dhtConfig.pin = device["pin"] | "";
    dhtConfig.name = device["name"] | "DHT_Default";
    dhtConfig.description = device["description"] | "";
    int pin = getPin(dhtConfig.pin);
    dhtInstance = new S_DHT(pin, DHT22);
    dhtInstance->begin();

    Serial.println("S_Devices::initDHT completed on pin " + String(pin));
}

void S_Devices::initMQ(const JsonObject& device) {
    Serial.println("S_Devices::initMQ starting for device:");
    serializeJson(device, Serial);
    Serial.println();

    mqConfig.pin = device["pin"] | "";
    mqConfig.name = device["name"] | "MQ_Default";
    mqConfig.description = device["description"] | "";
    int pin = getPin(mqConfig.pin);
    mqInstance = new S_MQ(pin);
    mqInstance->begin();

    Serial.println("S_Devices::initMQ completed on pin " + String(pin));
}

int S_Devices::getPin(const String& pinStr) {
    if (pinStr.startsWith("D")) {
        if (pinStr == "D0") return D0;
        if (pinStr == "D1") return D1;
        if (pinStr == "D2") return D2;
        if (pinStr == "D3") return D3;
        if (pinStr == "D4") return D4;
        if (pinStr == "D5") return D5;
        if (pinStr == "D6") return D6;
        if (pinStr == "D7") return D7;
        if (pinStr == "D8") return D8;
        if (pinStr == "D9") return D9;
        if (pinStr == "D10") return D10;
        if (pinStr == "D11") return D11;
        if (pinStr == "D12") return D12;
        if (pinStr == "D13") return D13;
        if (pinStr == "D14") return D14;
        if (pinStr == "D15") return D15;
    } else if (pinStr == "A0") {
        return A0;
    }
    return atoi(pinStr.c_str());
}

float S_Devices::getTemperature(const String& deviceName, const String& sensorName) {
    if (dsInitialized && dsInstance != nullptr && deviceName == dsConfig.name) {
        float* temperatures = dsInstance->getTemperature();
        for (int i = 0; i < dsConfig.sensorCount; i++) {
            if (dsConfig.sensors[i].name == sensorName) {
                return temperatures[i];
            }
        }
        Serial.println("Sensor " + sensorName + " not found in device " + deviceName);
        return NAN;
    }
    if (dhtInitialized && dhtInstance != nullptr && deviceName == dhtConfig.name && sensorName == "temperature") {
        return dhtInstance->getTemperature();
    }
    Serial.println("Device " + deviceName + " not found or unsupported sensor " + sensorName);
    return NAN;
}

DynamicJsonDocument S_Devices::getJsonRelayValuesForPublish() {
    return relayInstance->getJsonRelayValuesForPublish();
}

DynamicJsonDocument S_Devices::getJsonSensorValuesForPublish() {
    DynamicJsonDocument doc(512);
    JsonObject result = doc.to<JsonObject>();

    if (dsInitialized && dsInstance != nullptr) {
        float* temperatures = dsInstance->getTemperature();
        JsonObject sensorValues = result.createNestedObject(dsConfig.name);
        if (temperatures == nullptr) {
            for (int i = 0; i < dsConfig.sensorCount; i++) {
                sensorValues[dsConfig.sensors[i].name] = NAN;
            }
        } else {
            for (int i = 0; i < dsConfig.sensorCount; i++) {
                sensorValues[dsConfig.sensors[i].name] = temperatures[i];
            }
        }
    }

    if (dhtInitialized && dhtInstance != nullptr) {
        dhtInstance->read();
        JsonObject dhtValues = result.createNestedObject(dhtConfig.name);
        dhtValues["temperature"] = dhtInstance->getTemperature();
        dhtValues["humidity"] = dhtInstance->getHumidity();
    }

    if (mqInitialized && mqInstance != nullptr) {
        mqInstance->read();
        JsonObject mqValues = result.createNestedObject(mqConfig.name);
        mqValues["gas"] = mqInstance->getValue();
    }

    return doc;
}

void S_Devices::callback(const String& topic, const String& value) {
    relayInstance->callback(topic, value, &S_Devices::getDeviceNameFromTopic);
}

const String& S_Devices::getDeviceNameFromTopic(const String& topic) {
    Serial.println("S_Devices.cpp: getDeviceNameFromTopic");
    static String result;
    int lastSlash = topic.lastIndexOf("/");
    result = topic.substring(lastSlash + 1);
    Serial.println("getDeviceNameFromTopic result: " + result);
    return result;
}

int S_Devices::loop() {
    return relayInstance->loop();
}

DynamicJsonDocument S_Devices::getJsonAllValuesForPublish() {
    DynamicJsonDocument doc(1024);
    JsonObject result = doc.to<JsonObject>();

    DynamicJsonDocument relaysDoc = getJsonRelayValuesForPublish();
    for (JsonPair kv : relaysDoc.as<JsonObject>()) {
        result[kv.key()] = kv.value();
    }

    DynamicJsonDocument sensorsDoc = getJsonSensorValuesForPublish();
    for (JsonPair kv : sensorsDoc.as<JsonObject>()) {
        result[kv.key()] = kv.value();
    }

    result["time"] = S_Common::S_Common::getUTime();
    Serial.print("getJsonAllValuesForPublish: ");
    serializeJson(result, Serial);
    Serial.println();
    return doc;
}
