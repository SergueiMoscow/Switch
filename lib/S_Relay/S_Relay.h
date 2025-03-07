#ifndef _S_RELAY_
#define _S_RELAY_

#include <Arduino.h>
#include <ArduinoJson.h>
#include "S_Common.h"

#define MAX_RELAYS 4

struct RelayConfig {
    String pin;
    String name;
    String description;
    String on; // "LOW" или "HIGH"
    int maxOn; // Максимальное время работы в секундах
};

class S_Relay {
private:
    int numRelays;
    RelayConfig relays[MAX_RELAYS];
    unsigned long relayTurnedOn[MAX_RELAYS];
    unsigned long relayTurnOff[MAX_RELAYS];

    int getPin(const String& pinStr);
    int getRelayByPin(int pin);
    RelayConfig* getRelayByName(const String& name);
    void setTimeToTurnOff(int relay, unsigned long sec, const RelayConfig& config);

public:
    S_Relay();
    void initRelays(const JsonObject& configDoc);
    void changeRelay(int relay, const String& value, const String& caller);
    DynamicJsonDocument getJsonRelayValuesForPublish();
    void callback(const String& topic, const String& value, const String& (*getDeviceNameFromTopic)(const String&));
    int loop();
};

#endif
