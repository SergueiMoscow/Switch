#ifndef S_TARIFF_H
#define S_TARIFF_H

#include <Arduino.h>
#include <Arduino_JSON.h>
#include "S_Common.h"
#include "S_Relay.h"

struct Tariff {
    String name;
    String periods[4]; // Массив для хранения временных периодов
    int periodCount;
};

class S_Tariff {
private:
    Tariff tariffs[3]; // Массив для хранения до 3 тарифов
    int tariffCount;
    int currentTariffIdx; // Индекс активного тарифа
    bool relaysInitialized;
    S_Relay* relayInstance; // Указатель на S_Relay для управления реле

    void initRelays(const JsonObject& config);
    bool isTimeInPeriod(const String& period, unsigned long currentTime);

public:
    S_Tariff(S_Relay* relayInst);
    void setup(const JsonObject& config);
    void loop();
    DynamicJsonDocument getJsonTariffValuesForPublish();
};

#endif
