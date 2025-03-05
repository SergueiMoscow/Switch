#ifndef _S_DS_
#define _S_DS_

#include <OneWire.h>
#include <DallasTemperature.h>

class S_DS {
public:
    float* getTemperature();
    static S_DS& getInstance(int pin = -1);
    int init(int pin);
    int getSensorsCount();
private:
    S_DS(); // Приватный конструктор
    bool debug;
    static S_DS* instance;
    static bool isInitialized;
    int sensors_count;
    float* temperatures; // Массив для хранения температур
    OneWire oneWire;
    DallasTemperature sensors;
};

#endif
