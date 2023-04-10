#include "S_Devices.h"

S_Devices::S_Devices()
{
    init();
}

void S_Devices::init()
{
    S_FS fs = S_FS();
    config = JSON.parse(fs.readFile("/devices.json"));
    Serial.println("Devices config:");
    Serial.println(JSON.stringify(config));
    num_relays = -1;
    JSONVar keys = config.keys();
    for (int i = 0; i < keys.length(); i++) {
        String type = clearValue(config[keys[i]]["type"]);
        if (type == "Relay") {
            initRelay(config[keys[i]]);
        }
    }
}


String S_Devices::clearValue(JSONVar value)
{
    String result = JSON.stringify(value);
    result.replace("\"", "");
    return result;
}

void S_Devices::initRelay(JSONVar device)
{
    num_relays++;
    relays[num_relays][RELAY_PIN] = getPin(device["pin"]);
    String low = "LOW";
    relays[num_relays][RELAY_ON] = (device["on"] == low ? HIGH : LOW);
    relays[num_relays][RELAY_OFF] = (relays[num_relays][RELAY_ON] == LOW ? HIGH : LOW);
    pinMode(relays[num_relays][RELAY_PIN], OUTPUT);
    digitalWrite(relays[num_relays][RELAY_PIN], relays[num_relays][RELAY_OFF]);
}

int S_Devices::getPin(JSONVar pin)
{
    String spin = clearValue(pin);
    if (spin.startsWith("D")) {
        if (spin == "D0") return D0;
        if (spin == "D1") return D1;
        if (spin == "D2") return D2;
        if (spin == "D3") return D3;
        if (spin == "D5") return D5;
        if (spin == "D6") return D6;
        if (spin == "D7") return D7;
        if (spin == "D8") return D8;
        if (spin == "D9") return D9;
        if (spin == "D10") return D10;
        if (spin == "D11") return D11;
        if (spin == "D12") return D12;
        if (spin == "D13") return D13;
        if (spin == "D14") return D14;
        if (spin == "D15") return D15;
    }
    return atoi(spin.c_str());
}

void S_Devices::changeRelay(int relay, String value, String caller)
{
  digitalWrite(relay, (value == "on" ? relays[relay][RELAY_ON] : relays[relay][RELAY_OFF]));
}

JSONVar S_Devices::getForPublish()
{
    JSONVar keys = config.keys();
    JSONVar result = JSON.parse("{}");
    Serial.println("Keys lenght: " + (String)keys.length());
    for (int i = 0; i < keys.length(); i++) {
        String type = clearValue(config[keys[i]]["type"]);
        String name = clearValue(config[keys[i]]["name"]);
        Serial.println("Devices: " + type + " - " + name);
        if (type == "Relay") {
            int relay = getRelayByPin(getPin(config[keys[i]]["pin"]));
            String value = (digitalRead(relays[relay][RELAY_PIN]) == relays[relay][RELAY_ON] ? "on" : "off");
            result[name] = value;
            Serial.println("Devices: " + name + " = " + value);
        }
    }
    return result;
}

int S_Devices::getRelayByPin(int pin)
{
    for (int i = 0; i < num_relays; i++) {
        if (relays[i][RELAY_PIN] == pin) {
            return i;
        }
    }
    return -1;
}