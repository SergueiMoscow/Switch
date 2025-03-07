#include "S_DS.h"

S_DS* S_DS::instance = nullptr;
bool S_DS::isInitialized = false;

S_DS::S_DS() {
    sensors_count = 0;
    temperatures = nullptr;
    debug = false;
}

S_DS& S_DS::getInstance(int pin) {
    if (!isInitialized) {
        instance = new S_DS();
        if (pin != -1) {
            instance->init(pin);
        }
        isInitialized = true;
    }
    return *instance;
}

int S_DS::init(int pin) {
    if (debug) Serial.println("S_DS::init() pin: " + String(pin));
    oneWire = OneWire(pin);
    sensors = DallasTemperature(&oneWire);
    sensors.begin();
    sensors_count = sensors.getDeviceCount();
    if (debug) Serial.println("S_DS::init() sensors_count: " + String(sensors_count));

    if (temperatures != nullptr) {
        delete[] temperatures;
    }
    temperatures = new float[sensors_count];
    for (int i = 0; i < sensors_count; i++) {
        temperatures[i] = NAN;
    }
    return sensors_count;
}

float* S_DS::getTemperature() {
    if (debug) Serial.println("S_DS::getTemperature() request temp");
    if (sensors_count == 0) {
        if (debug) Serial.println("S_DS::getTemperature: No sensors available, returning null");
        return nullptr;
    }
    sensors.requestTemperatures();
    if (debug) Serial.println("S_DS::getTemperature() before cycle");
    for (int i = 0; i < sensors_count; i++) {
        if (debug) Serial.println("S_DS::getTemperature() in cycle");
        temperatures[i] = sensors.getTempCByIndex(i);
        if (debug) Serial.println("S_DS::getTemperature() Sensor " + String(i) + ": " + String(temperatures[i]));
        if (isnan(temperatures[i])) {
            Serial.println("S_DS::getTemperature: Sensor " + String(i) + " returned NAN");
        }
    }
    return temperatures;
}

int S_DS::getSensorsCount() {
    return sensors_count;
}