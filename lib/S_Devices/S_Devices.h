#include <Arduino.h>
#include <Arduino_JSON.h>
#include "S_Mode.h"
#include "S_FS.h"
#include "S_DS.h"
#include "S_Settings.h"
#include "S_Common.h"

// для массива relays:
#define RELAY_PIN 0
// значения LOW / HIGH
#define RELAY_ON 1  
#define RELAY_OFF 2
// Максимально сек может быть включенным Default
#define RELAY_MAX_SECONDS_ON 3
#define RELAY_DEFAULT_MAX_SECONDS_ON

#define MAX_RELAYS 4

class S_Devices
{
    private:
        S_DS* dsInstance; // Указатель на экземпляр S_DS
        int num_relays;
        int relays[MAX_RELAYS][4];
        unsigned long relay_turned_on[MAX_RELAYS];
        unsigned long relay_turn_off[MAX_RELAYS];
        JSONVar config;
        String getType(JSONVar device);
        String clearValue(JSONVar key);
        void initRelay(JSONVar device);
        void initDS18B20(JSONVar device);
        int getRelayByPin(int pin);
        int getPinByName(String name);
        void setMillisToTurnOff(int relay, int sec = 0);
        JSONVar getDeviceByName(String relayName);
        String getDeviceNameFromTopic(String topic);
        void setTimeToTurnOff(int relay, unsigned long sec, JSONVar device);
        
    public:
        S_Devices();
        int getPin(JSONVar pin);
        float getTemperature(String deviceName, String sensorName);
        void changeRelay(int relay, String value, String caller);
        JSONVar getJsonRelayValuesForPublish();
        void callback(String topic, String value);
        int loop();
        JSONVar getJsonSensorValuesForPublish();
        void setupModules(); // функция для инициализации модулей
        JSONVar getJsonAllValuesForPublish();
};