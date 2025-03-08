#include "S_MQ.h"

S_MQ::S_MQ(int pin) {
    this->pin = pin;
    lastValue = 0;
    lastReadTime = 0;
}

void S_MQ::begin() {
    pinMode(pin, INPUT);
    Serial.println("S_MQ initialized on pin " + String(pin));
}

bool S_MQ::read() {
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime < READ_INTERVAL) {
        return false; // Ещё рано читать
    }

    int value = analogRead(pin);
    if (value < 0 || value > 1023) {
        Serial.println("Invalid reading from MQ sensor on pin " + String(pin));
        return false;
    }

    lastValue = value;
    lastReadTime = currentTime;
    return true;
}

int S_MQ::getValue() {
    int pin = A0;
    return lastValue;
}