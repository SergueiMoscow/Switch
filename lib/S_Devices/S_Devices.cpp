#include "S_Devices.h"

S_Devices::S_Devices()
{
    S_FS fs = S_FS();
    config = JSON.parse(fs.readFile("/devices.json"));
}

void S_Devices::init()
{
    num_relays = -1;
    JSONVar keys = config.keys();
    for (int i = 0; i < keys.length(); i++) {
        String type = clearValue(config[keys[i]]["type"]);
        if (type == "relay") {
            initRelay(config[keys[i]]);
        }
    }
}


String S_Devices::clearValue(JSONVar value)
{
    String result = JSON.stringify(value);
    result.replace("\"", "");
    result.toLowerCase();
    return result;
}

void S_Devices::initRelay(JSONVar device)
{
    num_relays++;
    relays[num_relays][RELAY_PIN] = getPin(device["pin"]);
    String low = "low";
    relays[num_relays][RELAY_ON] = (device["on"] == low ? LOW : HIGH);
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