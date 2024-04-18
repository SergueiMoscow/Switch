#include "S_Devices.h"

S_Devices::S_Devices()
{
    init();
}

void S_Devices::init()
{
    S_FS fs = S_FS();
    if (!fs.exists("/devices.json")) {return;}
    config = JSON.parse(fs.readFile("/devices.json"));
    Serial.println("Devices config:");
    Serial.println(JSON.stringify(config));
    num_relays = 0;
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
    bool debug = false;
    relays[num_relays][RELAY_PIN] = getPin(device["pin"]);
    if (debug) Serial.println("initRelay " + (String)num_relays + " pin " + (String)relays[num_relays][RELAY_PIN]);
    String low = "LOW";
    Serial.println("Device on: " + clearValue(device["on"]));
    relays[num_relays][RELAY_ON] = (clearValue(device["on"]).equals(low) ? LOW : HIGH);
    if (debug) Serial.println("ON "  + (String)relays[num_relays][RELAY_ON]);
    relays[num_relays][RELAY_OFF] = (relays[num_relays][RELAY_ON] == LOW ? HIGH : LOW);
    if (debug) Serial.println("OFF "  + (String)relays[num_relays][RELAY_OFF]);
    if (debug) Serial.println("LOW "  + (String)LOW);
    if (debug) Serial.println("HIGH "  + (String)HIGH);
    pinMode(relays[num_relays][RELAY_PIN], OUTPUT);
    // digitalWrite(relays[num_relays][RELAY_PIN], relays[num_relays][RELAY_OFF]);
    changeRelay(num_relays, "off", "init");
    num_relays++;
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
    bool debug = false;
  digitalWrite(relays[relay][RELAY_PIN], (value.equals("on") ? relays[relay][RELAY_ON] : relays[relay][RELAY_OFF]));
  if (debug) Serial.println("Set " + (String)relays[relay][RELAY_PIN] + " to " + (String)digitalRead(relays[relay][RELAY_PIN]));
  
}

JSONVar S_Devices::getForPublish()
{
    bool debug = false;
    JSONVar keys = config.keys();
    JSONVar result = JSON.parse("{}");
    if (debug) Serial.println("S_Devices.cpp: Keys lenght: " + (String)keys.length());
    for (int i = 0; i < keys.length(); i++) {
        String type = clearValue(config[keys[i]]["type"]);
        String name = clearValue(config[keys[i]]["name"]);
        if (debug) Serial.println("Devices: " + type + " - " + name);
        if (type == "Relay") {
            int relay = getRelayByPin(getPin(config[keys[i]]["pin"]));
            String value = (digitalRead(relays[relay][RELAY_PIN]) == relays[relay][RELAY_ON] ? "on" : "off");
            result[name] = value;
            if (debug) Serial.println("Devices: " + name + " = " + value);
        }
    }
    return result;
}

int S_Devices::getRelayByPin(int pin)
{
    bool debug = true;
    if (debug) Serial.println("getRelayByPin " + (String)pin);
    if (debug) Serial.println("numRelays " + (String)num_relays);
    for (int i = 0; i < num_relays; i++) {
        if (debug) Serial.println("Checking " + (String)i + " pin " + (String)relays[i][RELAY_PIN]);
        if (relays[i][RELAY_PIN] == pin) {
            if (debug) Serial.println("Searching " + (String)relays[i][RELAY_PIN]);
            if (debug) Serial.println("Found " + (String)i);
            return i;
        }
    }
    if (debug) Serial.println("Relay by pin not found");
    return -1;
}

void S_Devices::callback(String topic, String value)
{
    bool debug = false;
    String name = getDeviceNameFromTopic(topic);
    Serial.println("Devices callback: " + name);
    JSONVar device = getDeviceByName(name);
    int pin = getPin(device["pin"]);
    Serial.print("Pin (getPin): " + (String)pin);
    int relay = getRelayByPin(pin);
    value.toUpperCase();
    if (debug) Serial.println("Device callback value received: " + value);
    if (value.equals("ON")) {
        Serial.println("ON: Pin: " + (String)relays[relay][RELAY_PIN] + " value " + relays[relay][RELAY_ON]);
        // digitalWrite(relays[relay][RELAY_PIN], relays[relay][RELAY_ON]);
        changeRelay(relay, "on", "callback");
        relay_turned_on[relay] = millis();
    }
    if (value.equals("OFF")) {
        Serial.println("OFF: Pin: " + (String)relays[relay][RELAY_PIN] + " value " + relays[relay][RELAY_OFF]);
        // digitalWrite(relays[relay][RELAY_PIN], relays[relay][RELAY_OFF]);
        changeRelay(relay, "off", "callback");
        relay_turned_on[relay] = 0UL;
    }
    if (value.toInt() > 0) {
        Serial.println("NUM: Pin: " + (String)relays[relay][RELAY_PIN] + " value " + relays[relay][RELAY_ON]);
        // digitalWrite(relays[relay][RELAY_PIN], relays[relay][RELAY_ON]);
        changeRelay(relay, "on", "callback");
        relay_turned_on[relay] = millis();
        setTimeToTurnOff(relay, value.toInt(), &device);
    }
}


String S_Devices::getDeviceNameFromTopic(String topic) {
  int lastSlashIndex = topic.lastIndexOf('/');
  int secondLastSlashIndex = -1;
  for(int i = lastSlashIndex - 1; i >= 0; i--) {
    if(topic[i] == '/') {
      secondLastSlashIndex = i;
      break;
    }
  }
  
  if(secondLastSlashIndex == -1) {
    // нет второго слеша
    return "";
  }

  return topic.substring(secondLastSlashIndex + 1, lastSlashIndex);
}

void S_Devices::setTimeToTurnOff(int relay, unsigned long sec, JSONVar device)
{
    bool debug = true;
    unsigned long currentTime = S_Common::S_Common::getUTime();
    if (sec == 0) {
        unsigned long defaultMaxSec = strtoul(clearValue(device["max_on"]).c_str(), 0, 10);
        relay_turn_off[relay] = currentTime + defaultMaxSec;
    } else {
        relay_turn_off[relay] = currentTime + sec;
    }
    if (debug) Serial.println("SetTimeToTurnOff: Turn off at " + (String)relay_turn_off[relay] + " now " + (String)(now()));

}

int S_Devices::getPinByName(String relayName)
{
    JSONVar keys = config.keys();
    for (int i = 0; i < keys.length(); i++) {
        String name = clearValue(config[keys[i]]["name"]);
        if (name == relayName) {
            return getRelayByPin(getPin(config[keys[i]]["pin"]));
        }
    }
    return -1;
}

JSONVar S_Devices::getDeviceByName(String relayName)
{
    bool debug = false;
    JSONVar keys = config.keys();
    for (int i = 0; i < keys.length(); i++) {
        String name = clearValue(config[keys[i]]["name"]);
        if (debug) Serial.println("getDeviceByName " + relayName);
        if (name == relayName) {
            if (debug) Serial.print("Found ");
            if (debug) Serial.println(config[keys[i]]);
            return config[keys[i]];
        }
        if (debug) Serial.println("Not found");
    }
    return {};
}

void S_Devices::loop()
{
    /// TODO: check relay_turn_off array
    bool debug = true;
    unsigned long currentTime = S_Common::S_Common::getUTime();
    for (int relay = 0; relay < num_relays; relay++) {
        if (relay_turn_off[relay] != 0 && relay_turn_off[relay] <= currentTime) {
            changeRelay(relay, "off", "loop timer");
            if (debug) Serial.println("Turning off by timer relay " + (String)relay);
            relay_turn_off[relay] = 0;
        }
    }
    
    // Serial.println("Devices loop Unsigned long max");
    // Serial.println(0UL - 1UL);
}
