#include "S_Relay.h"

S_Relay::S_Relay() {
    numRelays = 0;
    for (int i = 0; i < MAX_RELAYS; i++) {
        relayTurnedOn[i] = 0;
        relayTurnOff[i] = 0;
    }
}

void S_Relay::initRelays(const JsonObject& configDoc) {
    for (JsonPair kv : configDoc) {
        JsonObject device = kv.value().as<JsonObject>();
        String type = device["type"] | "";
        if (type == "Relay" && numRelays < MAX_RELAYS) {
            relays[numRelays].pin = device["pin"] | "";
            relays[numRelays].name = device["name"] | "";
            relays[numRelays].description = device["description"] | "";
            relays[numRelays].on = device["on"] | "LOW";
            relays[numRelays].maxOn = device["max_on"] | 0;

            int pin = S_Common::getPin(relays[numRelays].pin);
            Serial.println("initRelay " + String(numRelays) + " name: " + relays[numRelays].name + " pin: " + String(pin));
            int onValue = (relays[numRelays].on == "LOW") ? LOW : HIGH;
            int offValue = (onValue == LOW) ? HIGH : LOW;

            pinMode(pin, OUTPUT);
            digitalWrite(pin, offValue);
            Serial.println("Set " + String(pin) + " to " + String(digitalRead(pin)));
            numRelays++;
        }
    }
    Serial.println("Initialized " + String(numRelays) + " relays");
}

void S_Relay::changeRelay(int relay, const String& value, const String& caller) {
    if (relay < 0 || relay >= numRelays) {
        Serial.println("Invalid relay index: " + String(relay));
        return;
    }
    int pin = S_Common::getPin(relays[relay].pin);
    int onValue = (relays[relay].on == "LOW") ? LOW : HIGH;
    int offValue = (onValue == LOW) ? HIGH : LOW;
    digitalWrite(pin, (value == "on") ? onValue : offValue);
    Serial.println("Set " + String(pin) + " to " + String(digitalRead(pin)));
}

DynamicJsonDocument S_Relay::getJsonRelayValuesForPublish() {
    DynamicJsonDocument doc(512);
    JsonObject result = doc.to<JsonObject>();
    for (int i = 0; i < numRelays; i++) {
        int pin = S_Common::getPin(relays[i].pin);
        String value = (digitalRead(pin) == ((relays[i].on == "LOW") ? LOW : HIGH)) ? "on" : "off";
        result[relays[i].name] = value;
    }
    return doc;
}

int S_Relay::getRelayByPin(int pin) {
    for (int i = 0; i < numRelays; i++) {
        if (S_Common::getPin(relays[i].pin) == pin) return i;
    }
    return -1;
}

RelayConfig* S_Relay::getRelayByName(const String& name) {
    for (int i = 0; i < numRelays; i++) {
        if (relays[i].name == name) return &relays[i];
    }
    return nullptr;
}

void S_Relay::setTimeToTurnOff(int relay, unsigned long sec, const RelayConfig& config) {
    if (relay < 0 || relay >= numRelays) {
        Serial.println("Invalid relay index in setTimeToTurnOff: " + String(relay));
        return;
    }
    unsigned long currentTime = S_Common::S_Common::getUTime();
    unsigned long maxSec = (sec == 0) ? config.maxOn : min(sec, (unsigned long)config.maxOn);
    relayTurnOff[relay] = currentTime + maxSec;
    Serial.println("SetTimeToTurnOff: Turn off at " + String(relayTurnOff[relay]) + " now " + String(now()));
}

void S_Relay::callback(const String& topic, const String& value, const String& (*getDeviceNameFromTopic)(const String&)) {
// void S_Relay::callback(const String& topic, const String& value, const String& name) {
    Serial.println("S_Relay::callback");
    String name = getDeviceNameFromTopic(topic);
    Serial.println("Relay callback: topic = " + topic + ", value = " + value + ", name = " + name);
    RelayConfig* relay = getRelayByName(name);
    if (relay == nullptr) {
        Serial.println("Relay not found: " + name);
        return;
    }

    int relayIdx = getRelayByPin(S_Common::getPin(relay->pin));
    Serial.println("Relay index: " + String(relayIdx));
    if (relayIdx == -1) {
        Serial.println("Relay pin not found: " + String(S_Common::getPin(relay->pin)));
        return;
    }

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

int S_Relay::loop() {
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