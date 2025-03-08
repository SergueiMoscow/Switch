#include "S_MQ.h"

S_MQ::S_MQ(int pin) {
    this->pin = pin;
    lastRawValue = 0;
    lastPPM = 0.0;
    lastReadTime = 0;
    R0 = 0.0; // Пока не откалибровано
    calibrated = false;
}

void S_MQ::begin() {
    pinMode(pin, INPUT);
    Serial.println("S_MQ initialized on pin " + String(pin));
}

bool S_MQ::read() {
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime < READ_INTERVAL) {
        return false;
    }

    int value = analogRead(pin);
    if (value < 0 || value > 1023) {
        Serial.println("Invalid reading from MQ sensor on pin " + String(pin));
        return false;
    }

    lastRawValue = value;
    if (calibrated) {
        lastPPM = calculatePPM(value);
    } else {
        lastPPM = 0.0; // Пока не откалибровано
    }
    lastReadTime = currentTime;
    return true;
}

int S_MQ::getRawValue() {
    return lastRawValue;
}

float S_MQ::getPPM() {
    return lastPPM;
}

void S_MQ::calibrate() {
    Serial.println("Calibrating MQ sensor...");
    int samples = 10;
    long sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += analogRead(pin);
        delay(100); // Даём датчику стабилизироваться
    }
    int avgRaw = sum / samples;
    // Предполагаем, что это чистый воздух
    // Rs = ((Vcc * Rl) / Vout - Rl), где Vcc = 5V, Rl = нагрузочный резистор (обычно 10k)
    // Но АЦП видит только 3.3V, корректируем
    float Vout = (avgRaw / 1023.0) * 3.3; // Напряжение на A0
    float Rs = ((5.0 * 10.0) / Vout) - 10.0; // Rl = 10k
    R0 = Rs / 9.8; // Коэффициент для чистого воздуха из datasheet MQ-4
    calibrated = true;
    Serial.println("Calibration complete. R0 = " + String(R0) + ", avgRaw = " + String(avgRaw));
}

// Note: MQ-4 powered at 5V. Use a voltage divider (R1=2kΩ, R2=3.3kΩ) between AOUT and A0
// to scale 0-5V to 0-3.3V for ESP8266 ADC safety.

float S_MQ::calculatePPM(int rawValue) {
    if (R0 == 0.0) return 0.0; // Не откалибровано
    float Vout = (rawValue / 1023.0) * 3.3;
    float Rs = ((5.0 * 10.0) / Vout) - 10.0;
    float ratio = Rs / R0;
    // Примерная формула для CH4 из datasheet: PPM = 1000 * (Rs/R0)^(-2.6)
    float ppm = 1000.0 * pow(ratio, -2.6);
    return ppm;
}


// С делителем из резисторов использовать эту функцию:
// float S_MQ::calculatePPM(int rawValue) {
//     if (R0 == 0.0) return 0.0;
//     // С делителем 2kΩ и 3.3kΩ: масштабируем обратно к 5V
//     float Vout = (rawValue / 1023.0) * 3.3 * (5.3 / 3.3); // Учитываем делитель (R1+R2)/R2
//     float Rs = ((5.0 * 10.0) / Vout) - 10.0;
//     float ratio = Rs / R0;
//     float ppm = 1000.0 * pow(ratio, -2.6);
//     return ppm;
// }