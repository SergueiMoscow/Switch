#include "S_Devices.h"

S_Devices::S_Devices() {
    dsInstance = nullptr;
    numRelays = 0;
    dsInitialized = false;
    for (int i = 0; i < MAX_RELAYS; i++) {
        relayTurnedOn[i] = 0;
        relayTurnOff[i] = 0;
    }
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

    for (JsonPair kv : doc.as<JsonObject>()) {
        JsonObject device = kv.value().as<JsonObject>();
        String type = device["type"] | "";
        Serial.println("Initializing device: " + String(kv.key().c_str()) + ", type: " + type);
        if (type == "Relay" && numRelays < MAX_RELAYS) {
            initRelay(device, numRelays);
            numRelays++;
        } else if (type == "DS18B20" && !dsInitialized) {
            initDS18B20(device);
            dsInitialized = true; // Пока поддерживаем только один DS18B20
        }
    }
}

void S_Devices::initDS18B20(const JsonObject& device) {
    Serial.println("S_Devices::initDS18B20 starting for device:");
    serializeJson(device, Serial);
    Serial.println();

    dsConfig.pin = device["pin"] | "";
    int pin = getPin(dsConfig.pin);
    dsInstance = &S_DS::getInstance(pin);

    JsonArray sensors = device["sensors"].as<JsonArray>();
    dsConfig.sensorCount = min(int(sensors.size()), 2); // Ограничиваем 2 сенсорами
    for (int i = 0; i < dsConfig.sensorCount; i++) {
        dsConfig.sensors[i].name = sensors[i]["name"] | "";
        dsConfig.sensors[i].description = sensors[i]["description"] | "";
    }

    Serial.println("S_Devices::initDS18B20 completed, dsInstance: " + String((unsigned long)dsInstance, HEX));
}

float S_Devices::getTemperature(const String& deviceName, const String& sensorName) {
    if (!dsInitialized || dsInstance == nullptr) {
        Serial.println("Error: DS18B20 instance not initialized");
        return NAN;
    }

    if (deviceName == "device3") { // Пока хардкодим, так как одна конфигурация
        float* temperatures = dsInstance->getTemperature();
        for (int i = 0; i < dsConfig.sensorCount; i++) {
            if (dsConfig.sensors[i].name == sensorName) {
                return temperatures[i];
            }
        }
        Serial.println("Sensor " + sensorName + " not found in device " + deviceName);
    }
    return NAN;
}

void S_Devices::initRelay(const JsonObject& device, int index) {
    relays[index].pin = device["pin"] | "";
    relays[index].name = device["name"] | "";
    relays[index].description = device["description"] | "";
    relays[index].on = device["on"] | "LOW";
    relays[index].maxOn = device["max_on"] | 0;

    int pin = getPin(relays[index].pin);
    Serial.println("initRelay " + String(index) + " pin " + String(pin));
    int onValue = (relays[index].on == "LOW") ? LOW : HIGH;
    int offValue = (onValue == LOW) ? HIGH : LOW;

    pinMode(pin, OUTPUT);
    digitalWrite(pin, offValue);
    Serial.println("Set " + String(pin) + " to " + String(digitalRead(pin)));
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
    }
    return atoi(pinStr.c_str());
}

void S_Devices::changeRelay(int relay, const String& value, const String& caller) {
    int pin = getPin(relays[relay].pin);
    int onValue = (relays[relay].on == "LOW") ? LOW : HIGH;
    int offValue = (onValue == LOW) ? HIGH : LOW;
    digitalWrite(pin, (value == "on") ? onValue : offValue);
    Serial.println("Set " + String(pin) + " to " + String(digitalRead(pin)));
}

DynamicJsonDocument S_Devices::getJsonRelayValuesForPublish() {
    DynamicJsonDocument doc(512);
    JsonObject result = doc.to<JsonObject>();
    for (int i = 0; i < numRelays; i++) {
        int pin = getPin(relays[i].pin);
        String value = (digitalRead(pin) == ((relays[i].on == "LOW") ? LOW : HIGH)) ? "on" : "off";
        result[relays[i].name] = value;
    }
    return doc;
}

int S_Devices::getRelayByPin(int pin) {
    for (int i = 0; i < numRelays; i++) {
        if (getPin(relays[i].pin) == pin) return i;
    }
    return -1;
}

DynamicJsonDocument S_Devices::getJsonSensorValuesForPublish() {
    DynamicJsonDocument doc(512);
    JsonObject result = doc.to<JsonObject>();

    if (dsInitialized && dsInstance != nullptr) {
        float* temperatures = dsInstance->getTemperature();
        JsonObject sensorValues = result.createNestedObject("device3");
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
    return doc;
}

void S_Devices::callback(const String& topic, const String& value) {
    String name = getDeviceNameFromTopic(topic);
    RelayConfig* relay = getRelayByName(name);
    if (relay == nullptr) return;

    int relayIdx = getRelayByPin(getPin(relay->pin));
    String upperValue = value;
    upperValue.toUpperCase();

    if (upperValue == "ON") {
        changeRelay(relayIdx, "on", "callback");
        relayTurnedOn[relayIdx] = millis();
        setTimeToTurnOff(relayIdx, 0, *relay);
    } else if (upperValue == "OFF") {
        changeRelay(relayIdx, "off", "callback");
        relayTurnedOn[relayIdx] = 0UL;
    } else if (value.toInt() > 0) {
        changeRelay(relayIdx, "on", "callback");
        relayTurnedOn[relayIdx] = millis();
        setTimeToTurnOff(relayIdx, value.toInt(), *relay);
    }
}

String S_Devices::getDeviceNameFromTopic(const String& topic) {
    int lastSlash = topic.lastIndexOf("/");
    return topic.substring(lastSlash + 1);
}

void S_Devices::setTimeToTurnOff(int relay, unsigned long sec, const RelayConfig& config) {
    unsigned long currentTime = S_Common::S_Common::getUTime();
    unsigned long maxSec = (sec == 0) ? config.maxOn : min(sec, (unsigned long)config.maxOn);
    relayTurnOff[relay] = currentTime + maxSec;
    Serial.println("SetTimeToTurnOff: Turn off at " + String(relayTurnOff[relay]) + " now " + String(now()));
}

RelayConfig* S_Devices::getRelayByName(const String& name) {
    for (int i = 0; i < numRelays; i++) {
        if (relays[i].name == name) return &relays[i];
    }
    return nullptr;
}

int S_Devices::loop() {
    unsigned long currentTime = S_Common::S_Common::getUTime();
    int result = 0;
    for (int relay = 0; relay < numRelays; relay++) {
        if (relayTurnOff[relay] != 0 && relayTurnOff[relay] <= currentTime) {
            changeRelay(relay, "off", "loop timer");
            Serial.println("Turning off by timer relay " + String(relay));
            relayTurnOff[relay] = 0;
            result++;
        }
    }
    return result;
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
