#include "S_DHT.h"

S_DHT::S_DHT(int pin, uint8_t type) {
    this->pin = pin;
    this->type = type;
    dht = new DHT(pin, type);
    lastTemperature = NAN;
    lastHumidity = NAN;
    lastReadTime = 0;
}

void S_DHT::begin() {
    dht->begin();
    Serial.println("S_DHT initialized on pin " + String(pin));
}

bool S_DHT::read() {
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime < READ_INTERVAL) {
        return false; // Ещё рано читать
    }

    float temp = dht->readTemperature();
    float humid = dht->readHumidity();

    if (isnan(temp) || isnan(humid)) {
        Serial.println("Failed to read from DHT sensor on pin " + String(pin));
        return false;
    }

    lastTemperature = temp;
    lastHumidity = humid;
    lastReadTime = currentTime;
    return true;
}

float S_DHT::getTemperature() {
    return lastTemperature;
}

float S_DHT::getHumidity() {
    return lastHumidity;
}
