#include "S_Devices.h"

S_Devices::S_Devices() {
    dsInstance = nullptr;
    num_relays = 0;
    setupModules();
}

void S_Devices::setupModules() {
    S_FS fs = S_FS();
    if (!fs.exists("/devices.json")) {
        S_Mode::setConfigMQTTMode("No devices.json found");
        return;
    }
    config = JSON.parse(fs.readFile("/devices.json"));
    Serial.println("Devices config parsed: " + JSON.stringify(config));
    
    JSONVar keys = config.keys();
    if (keys.length() == 0) {
        Serial.println("Error: No devices found in config");
        return;
    }

    num_relays = 0;
    for (int i = 0; i < keys.length(); i++) {
        String type = clearValue(config[keys[i]]["type"]);
        Serial.println("Initializing device: " + String(keys[i]) + ", type: " + type);
        if (type == "Relay") {
            initRelay(config[keys[i]]);
        } else if (type == "DS18B20") {
            initDS18B20(config[keys[i]]);
        }
    }
}

void S_Devices::initDS18B20(JSONVar device) {
    Serial.println("S_Devices::initDS18B20 starting for device: " + JSON.stringify(device));
    Serial.print("S_Devices::initDS18B20 - device['type'] raw: ");
    Serial.println(JSON.stringify(device["type"])); // Проверяем точное значение conf["type"]
    Serial.print("S_Devices::initDS18B20 - device['pin'] raw: ");
    Serial.println(JSON.stringify(device["pin"])); // Проверяем точное значение conf["pin"]
    Serial.print("S_Devices::initDS18B20 - device address: ");
    Serial.println((unsigned long)&device, HEX);
    int pin = getPin(device["pin"]);
    dsInstance = &S_DS::getInstance(pin);
    Serial.println("S_Devices::initDS18B20 completed, dsInstance: " + String((unsigned long)dsInstance, HEX));
}

float S_Devices::getTemperature(String deviceName, String sensorName) {
    JSONVar keys = config.keys();
    for (int i = 0; i < keys.length(); i++) {
        String name = clearValue(keys[i]);
        if (name == deviceName && clearValue(config[keys[i]]["type"]) == "DS18B20") {
            if (dsInstance == nullptr) {
                Serial.println("Error: DS18B20 instance not initialized for device " + deviceName);
                return NAN;
            }
            float* temperatures = dsInstance->getTemperature(); // Получаем указатель на массив
            JSONVar sensors = config[keys[i]]["sensors"];
            for (int j = 0; j < sensors.length(); j++) {
                String sName = clearValue(sensors[j]["name"]);
                if (sName == sensorName) {
                    return temperatures[j]; // Возвращаем температуру конкретного датчика
                }
            }
            Serial.println("Sensor " + sensorName + " not found in device " + deviceName);
            return NAN;
        }
    }
    Serial.println("Device " + deviceName + " not found or not DS18B20");
    return NAN;
}

String S_Devices::clearValue(JSONVar value)
{
    String result = JSON.stringify(value);
    result.replace("\"", "");
    return result;
}

void S_Devices::initRelay(JSONVar device)
{
    bool debug = true;
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
    if (debug) Serial.print("Max_on for relay " + (String)device["name"]);
    if (debug) Serial.println(device["max_on"]);
    relays[num_relays][RELAY_MAX_SECONDS_ON] = device["max_on"];
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
        if (spin == "D3") return D3; // 5
        if (spin == "D4") return D4; // 4
        if (spin == "D5") return D5;
        if (spin == "D6") return D6;
        if (spin == "D7") return D7;
        if (spin == "D8") return D8; // 0
        if (spin == "D9") return D9; // 2
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
  bool debug = true;
  digitalWrite(relays[relay][RELAY_PIN], (value.equals("on") ? relays[relay][RELAY_ON] : relays[relay][RELAY_OFF]));
  if (debug) Serial.println("Set " + (String)relays[relay][RELAY_PIN] + " to " + (String)digitalRead(relays[relay][RELAY_PIN]));
  
}

JSONVar S_Devices::getJsonRelayValuesForPublish()
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
    bool debug = false;
    if (debug) Serial.println("getRelayByPin " + (String)pin);
    if (debug) Serial.println("numRelays " + (String)num_relays);
    for (int i = 0; i < num_relays; i++) {
        if (debug) Serial.println("Devices::getRelayByPin Checking " + (String)i + " pin " + (String)relays[i][RELAY_PIN]);
        if (relays[i][RELAY_PIN] == pin) {
            if (debug) Serial.println("S_Devices::getRelayByPin Searching " + (String)relays[i][RELAY_PIN]);
            if (debug) Serial.println("Found " + (String)i);
            return i;
        }
    }
    if (debug) Serial.println("Relay by pin not found");
    return -1;
}

JSONVar S_Devices::getJsonSensorValuesForPublish() {
    JSONVar result;

    JSONVar keys = config.keys();
    for (int i = 0; i < keys.length(); i++) {
        String deviceName = String(keys[i]);
        String deviceType = String(config[keys[i]]["type"]);
        Serial.println("S_Device::getJsonSensorValuesForPublish, Device: " + deviceName + " Type: " + deviceType);
        if (deviceType != "DS18B20") continue;

        JSONVar sensors = config[keys[i]]["sensors"];
        JSONVar sensorValues;
        Serial.println("S_Device::getJsonSensorValuesForPublish, DS18B20");
        Serial.println("trying to get temperature");
        float* temperatures = dsInstance->getTemperature();
        if (temperatures == nullptr) {
            Serial.println("S_Device::getJsonSensorValuesForPublish: No temperature data available");
            for (int j = 0; j < sensors.length(); j++) {
                String sensorName = String(sensors[j]["name"]);
                sensorValues[sensorName] = NAN; // Устанавливаем NAN для всех датчиков
                Serial.println("S_Device::getJsonSensorValuesForPublish, SensorName: " + sensorName + " temp: NAN");
            }
        } else {
            Serial.println("S_Device::getJsonSensorValuesForPublish, Sensors: " + JSON.stringify(sensors));
            for (int j = 0; j < sensors.length(); j++) {
                String sensorName = String(sensors[j]["name"]);
                float temp = temperatures[j];
                sensorValues[sensorName] = temp;
                Serial.println("S_Device::getJsonSensorValuesForPublish, SensorName: " + sensorName + " temp: " + String(temp));
            }
        }
        result[deviceName] = sensorValues;
    }
    Serial.println("S_Device::getJsonSensorValuesForPublish, Result: " + JSON.stringify(result));
    return result;
}

void S_Devices::callback(String topic, String value)
{
    bool debug = true;
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
        setTimeToTurnOff(relay, 0, &device);
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
  // Возвращает последний элемент топика:
  // Home/Room/Device/set/relay_name - возвращает relay_name.
  // Конфигурится в devices.json $.<device>.relay_name
  int lastSlash = topic.lastIndexOf("/");
  String name = topic.substring(lastSlash + 1);
  return name;
  //  Эта часть кода возвращала предпоследний элемент из строки:
  //  Home/Room/<name>/set
//   int lastSlashIndex = topic.lastIndexOf('/');
//   int secondLastSlashIndex = -1;
//   for(int i = lastSlashIndex - 1; i >= 0; i--) {
//     if(topic[i] == '/') {
//       secondLastSlashIndex = i;
//       break;
//     }
//   }
  
//   if(secondLastSlashIndex == -1) {
//     // нет второго слеша
//     return "";
//   }

//   return topic.substring(secondLastSlashIndex + 1, lastSlashIndex);
}

void S_Devices::setTimeToTurnOff(int relay, unsigned long sec, JSONVar device)
{
    bool debug = true;
    unsigned long currentTime = S_Common::S_Common::getUTime();
    unsigned long defaultMaxSec = relays[relay][RELAY_MAX_SECONDS_ON];
    if (sec == 0) {
        if (debug) Serial.print("setTimeToTurnOff defaultMaxSec = " + (String)defaultMaxSec);
        relay_turn_off[relay] = currentTime + defaultMaxSec;
    } else {
        relay_turn_off[relay] = currentTime + min(sec, defaultMaxSec);
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

int S_Devices::loop()
// Возвращает количество изменённых значений реле (выключений)
{
    bool debug = true;
    unsigned long currentTime = S_Common::S_Common::getUTime();
    int result = 0;
    for (int relay = 0; relay < num_relays; relay++) {
        if (relay_turn_off[relay] != 0 && relay_turn_off[relay] <= currentTime) {
            changeRelay(relay, "off", "loop timer");
            if (debug) Serial.println("Turning off by timer relay " + (String)relay);
            relay_turn_off[relay] = 0;
            result ++;
        }
    }
    return result;
}

JSONVar S_Devices::getJsonAllValuesForPublish() {
    JSONVar devicesValues = getJsonRelayValuesForPublish();
    JSONVar sensorValues = getJsonSensorValuesForPublish();
    Serial.println("getJsonAllValuesForPublish devicesValues: " + JSON.stringify(devicesValues));
    Serial.println("getJsonAllValuesForPublish sensorValues: " + JSON.stringify(sensorValues));
    
    JSONVar keys = sensorValues.keys();
    for (int i = 0; i < keys.length(); i++) {
        String deviceName = String(keys[i]);
        JSONVar sensorData = sensorValues[keys[i]]; // Получаем вложенный объект
        JSONVar tempObject = JSON.parse(JSON.stringify(sensorData)); // Явно копируем объект
        Serial.println("Adding device: " + deviceName + " with value: " + JSON.stringify(tempObject));
        devicesValues[deviceName] = tempObject;
    }
    
    devicesValues["time"] = S_Common::S_Common::getUTime();
    Serial.print("getJsonAllValuesForPublish: ");
    Serial.println(JSON.stringify(devicesValues));
    return devicesValues;
}