#include "S_Buzzer.h"

S_Buzzer::S_Buzzer(int pin) {
    this->pin = pin;
    active = false;
}

void S_Buzzer::begin() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW); // Выключаем по умолчанию
    Serial.println("S_Buzzer initialized on pin " + String(pin));
}

void S_Buzzer::on() {
    digitalWrite(pin, HIGH);
    active = true;
    Serial.println("Buzzer ON");
}

void S_Buzzer::off() {
    digitalWrite(pin, LOW);
    active = false;
    Serial.println("Buzzer OFF");
}

bool S_Buzzer::isActive() {
    return active;
}
