#ifndef _S_MQ_
#define _S_MQ_

#include <Arduino.h>

class S_MQ {
private:
    int pin;
    int lastRawValue; // Сырое значение (0–1023)
    float lastPPM; // Значение в ppm
    unsigned long lastReadTime;
    static const unsigned long READ_INTERVAL = 1000;
    float R0; // Сопротивление в чистом воздухе (калибровочное)
    bool calibrated;

    float calculatePPM(int rawValue);

public:
    S_MQ(int pin);
    void begin();
    bool read();
    int getRawValue(); // Сырое значение
    float getPPM(); // Значение в ppm
    void calibrate(); // Калибровка в чистом воздухе
};

#endif