#include "S_Tariff.h"

S_Tariff::S_Tariff(S_Relay* relayInst) {
    relayInstance = relayInst;
    tariffCount = 0;
    currentTariffIdx = -1;
    relaysInitialized = false;
}

void S_Tariff::setup(const JsonObject& config) {
    Serial.println("S_Tariff::setup starting");

    JsonArray tariffArray = config["Tariffs"].as<JsonArray>();
    tariffCount = min((int)tariffArray.size(), 3); // Ограничиваем до 3 тарифов
    Serial.println("Tariff setup point 1");
    for (int i = 0; i < tariffCount; i++) {
        JsonObject tariffObj = tariffArray[i];
        tariffs[i].name = tariffObj["Name"] | "";
        tariffs[i].periodCount = 0;
        Serial.println("Tariff setup point 2");

        JsonArray periods = tariffObj["periods"].as<JsonArray>();
        for (int j = 0; j < min((int)periods.size(), 4); j++) {
            tariffs[i].periods[j] = periods[j].as<String>();
            tariffs[i].periodCount++;
            Serial.println("Tariff setup point 3: " + periods[j].as<String>());
        }
    }

    initRelays(config);
    relaysInitialized = true;
    Serial.println("S_Tariff::setup completed, initialized " + String(tariffCount) + " tariffs");
}

void S_Tariff::initRelays(const JsonObject& config) {
    // Ничего не делаем, так как реле инициализируются через S_Relay
    Serial.println("S_Tariff::initRelays: Relays managed by S_Relay");
}

bool S_Tariff::isTimeInPeriod(const String& period, unsigned long currentTime) {
    int startHour, endHour;
    sscanf(period.c_str(), "%d-%d", &startHour, &endHour);

    time_t now = currentTime;
    struct tm* timeinfo = localtime(&now);
    int currentHour = timeinfo->tm_hour;
    bool inPeriod = currentHour >= startHour && currentHour < endHour;
    return inPeriod;
}

void S_Tariff::loop() {
    if (!relaysInitialized) return;

    unsigned long currentTime = S_Common::S_Common::getUTime();
    int newTariffIdx = -1;

    for (int i = 0; i < tariffCount; i++) {
        for (int j = 0; j < tariffs[i].periodCount; j++) {
            if (isTimeInPeriod(tariffs[i].periods[j], currentTime)) {
                newTariffIdx = i;
                break;
            }
        }
        if (newTariffIdx != -1) break;
    }

    if (newTariffIdx != currentTariffIdx && newTariffIdx != -1) {
        Serial.println("Switching to tariff: " + tariffs[newTariffIdx].name);

        // Сначала включаем новое реле
        RelayConfig* newRelay = relayInstance->getRelayByName(tariffs[newTariffIdx].name);
        if (newRelay != nullptr) {
            int newRelayIdx = relayInstance->getRelayByPin(S_Common::getPin(newRelay->pin));
            if (newRelayIdx != -1) {
                relayInstance->changeRelay(newRelayIdx, "on", "tariff");
            } else {
                Serial.println("Error: Relay pin " + newRelay->pin + " not found");
            }
        } else {
            Serial.println("Error: Relay " + tariffs[newTariffIdx].name + " not found");
        }

        // Затем выключаем остальные
        for (int i = 0; i < tariffCount; i++) {
            if (i != newTariffIdx) {
                RelayConfig* relay = relayInstance->getRelayByName(tariffs[i].name);
                if (relay != nullptr) {
                    int relayIdx = relayInstance->getRelayByPin(S_Common::getPin(relay->pin));
                    if (relayIdx != -1) {
                        relayInstance->changeRelay(relayIdx, "off", "tariff");
                    }
                }
            }
        }

        currentTariffIdx = newTariffIdx;
    }
}

DynamicJsonDocument S_Tariff::getJsonTariffValuesForPublish() {
    DynamicJsonDocument doc(256);
    JsonObject result = doc.to<JsonObject>();
    for (int i = 0; i < tariffCount; i++) {
        result[tariffs[i].name] = (i == currentTariffIdx) ? "on" : "off";
    }
    return doc;
}
