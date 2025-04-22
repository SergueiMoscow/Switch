#include "S_Devices.h"

S_Devices::S_Devices() {
    dsInstance = nullptr;
    relayInstance = new S_Relay();
    dhtInstance = nullptr;
    mqInstance = nullptr;
    buzzerInstance = nullptr;
    dsInitialized = false;
    dhtInitialized = false;
    mqInitialized = false;
    buzzerInitialized = false;
    tariffInstance = new S_Tariff(relayInstance);
    triggerCount = 0;
    setupModules();
}

void S_Devices::setupModules() {
    S_FS fs;
    if (fs.exists("/devices.json")) {

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
            } else if (type == "Buzzer" && !buzzerInitialized) {
                initBuzzer(device);
                buzzerInitialized = true;
            } else if (type == "Relay") {
                JsonArray triggersArray = device["triggers"];
                for (JsonVariant v : triggersArray) {
                    if (triggerCount >= 8) break;
                    triggers[triggerCount].relayName = device["name"] | "";
                    triggers[triggerCount].device = v["device"] | "";
                    triggers[triggerCount].parameter = v["parameter"] | "";
                    triggers[triggerCount].condition = v["condition"] | "";
                    triggers[triggerCount].threshold = v["threshold"] | 0.0;
                    triggers[triggerCount].action = v["action"] | "";
                    triggers[triggerCount].active = v["active"] | true;
                    triggerCount++;
                }
            }
        }
        Serial.println("Initialized " + String(triggerCount) + " automation triggers");
    } else {
        S_Mode::setConfigMQTTMode("No devices.json found");
        return;
    }
    if (fs.exists("/tariffs.json")) {
        DynamicJsonDocument tariffDoc = S_FS::readJsonFileDynamic("/tariffs.json");
        if (!tariffDoc.isNull()) {
            Serial.println("Tariffs config parsed:");
            serializeJson(tariffDoc, Serial);
            Serial.println();
            tariffInstance->setup(tariffDoc.as<JsonObject>());
        } else {
            Serial.println("Error: Failed to parse tariffs.json");
        }
    }

}

void S_Devices::initDS18B20(const JsonObject& device) {
    Serial.println("S_Devices::initDS18B20 starting for device:");
    serializeJson(device, Serial);
    Serial.println();

    dsConfig.pin = device["pin"] | "";
    dsConfig.name = device["name"] | "DS18B20_Default";
    int pin = S_Common::getPin(dsConfig.pin);
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
    int pin = S_Common::getPin(dhtConfig.pin);
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
    int pin = S_Common::getPin(mqConfig.pin);
    mqInstance = new S_MQ(pin);
    mqInstance->begin();
    mqInstance->calibrate();

    Serial.println("S_Devices::initMQ completed on pin " + String(pin));
}

void S_Devices::initBuzzer(const JsonObject& device) {
    Serial.println("S_Devices::initBuzzer starting for device:");
    serializeJson(device, Serial);
    Serial.println();

    buzzerConfig.pin = device["pin"] | "";
    buzzerConfig.name = device["name"] | "Buzzer_Default";
    int pin = S_Common::getPin(buzzerConfig.pin);
    buzzerInstance = new S_Buzzer(pin);
    buzzerInstance->begin();

    JsonArray triggers = device["triggers"].as<JsonArray>();
    buzzerConfig.triggerCount = min(int(triggers.size()), 3);
    for (int i = 0; i < buzzerConfig.triggerCount; i++) {
        buzzerConfig.triggers[i].device = triggers[i]["device"] | "";
        buzzerConfig.triggers[i].parameter = triggers[i]["parameter"] | "";
        buzzerConfig.triggers[i].condition = triggers[i]["condition"] | "";
        buzzerConfig.triggers[i].threshold = triggers[i]["threshold"] | 0.0;
    }

    Serial.println("S_Devices::initBuzzer completed on pin " + String(pin));
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
        mqValues["gas_raw"] = mqInstance->getRawValue();
        mqValues["gas_ppm"] = mqInstance->getPPM();
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

void S_Devices::buzzerLoop() {
    if (buzzerInitialized && buzzerInstance != nullptr) {
        bool shouldBeActive = false;
        for (int i = 0; i < buzzerConfig.triggerCount; i++) {
            BuzzerTrigger& trigger = buzzerConfig.triggers[i];
            float value = 0.0;

            if (trigger.device == mqConfig.name && mqInstance != nullptr) {
                if (trigger.parameter == "gas_ppm") value = mqInstance->getPPM();
                else if (trigger.parameter == "gas_raw") value = mqInstance->getRawValue();
            } else if (trigger.device == dhtConfig.name && dhtInstance != nullptr) {
                if (trigger.parameter == "temperature") value = dhtInstance->getTemperature();
                else if (trigger.parameter == "humidity") value = dhtInstance->getHumidity();
            }

            bool conditionMet = (trigger.condition == "above" && value > trigger.threshold) ||
                                (trigger.condition == "below" && value < trigger.threshold);
            if (conditionMet) {
                shouldBeActive = true;
                Serial.println("Buzzer trigger met: " + trigger.device + " " + trigger.parameter + " = " + String(value));
                break;
            }
        }

        if (shouldBeActive && !buzzerInstance->isActive()) {
            buzzerInstance->on();
        } else if (!shouldBeActive && buzzerInstance->isActive()) {
            buzzerInstance->off();
        }
    }
}

int S_Devices::loop() {
    int relayResult = relayInstance->loop();
    buzzerLoop();
    tariffInstance->loop();
    return relayResult;
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

int S_Devices::processAutomation(DynamicJsonDocument& sensorValues, DynamicJsonDocument& relayValues, void (*publish)(const String&, const String&)) {
    int result = 0;
    for (int i = 0; i < triggerCount; i++) {
        if (!triggers[i].active) continue;

        float value = sensorValues[triggers[i].device][triggers[i].parameter].as<float>();
        if (isnan(value)) continue;

        bool conditionMet = (triggers[i].condition == "above" && value > triggers[i].threshold) ||
                            (triggers[i].condition == "below" && value < triggers[i].threshold);
        if (!conditionMet) continue;

        String currentState = relayValues[triggers[i].relayName].as<String>();
        if (currentState == triggers[i].action) continue;

        RelayConfig* relay = relayInstance->getRelayByName(triggers[i].relayName);
        if (relay) {
            int relayIdx = relayInstance->getRelayByPin(S_Common::getPin(relay->pin));
            if (relayIdx != -1) {
                relayInstance->changeRelay(relayIdx, triggers[i].action, "automation");
                result++;

                if (publish) {
                    String topic = "device/relay/" + triggers[i].relayName;
                    String payload = triggers[i].action;
                    publish(topic, payload);
                    Serial.println("Automation: Published " + topic + " = " + payload);
                }
            }
        }
    }
    return result;
}