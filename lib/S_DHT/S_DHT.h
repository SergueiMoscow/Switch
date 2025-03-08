#ifndef _S_DHT_
#define _S_DHT_

#include <Arduino.h>
#include <DHT.h> // Библиотека DHT от Adafruit

class S_DHT {
private:
    DHT* dht;
    int pin;
    uint8_t type; // DHT11 или DHT22
    float lastTemperature;
    float lastHumidity;
    unsigned long lastReadTime;
    static const unsigned long READ_INTERVAL = 2000; // Читаем не чаще чем раз в 2 секунды

public:
    S_DHT(int pin, uint8_t type = DHT22); // По умолчанию DHT22
    void begin();
    bool read(); // Читает данные с датчика
    float getTemperature(); // Возвращает последнюю температуру
    float getHumidity(); // Возвращает последнюю влажность
};

#endif