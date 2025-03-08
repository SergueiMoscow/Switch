#ifndef _S_MQ_
#define _S_MQ_

#include <Arduino.h>

class S_MQ {
private:
    int pin;
    int lastValue;
    unsigned long lastReadTime;
    static const unsigned long READ_INTERVAL = 1000; // Читаем раз в секунду

public:
    S_MQ(int pin);
    void begin();
    bool read(); // Читает данные с датчика
    int getValue(); // Возвращает последнее значение (0–1023)
};

#endif