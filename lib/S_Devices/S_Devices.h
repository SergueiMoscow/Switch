#include <Arduino.h>
#include <Arduino_JSON.h>
#include "S_FS.h"
#include "S_DS.h"
#include "S_Settings.h"

#define RELAY_PIN 0
#define RELAY_ON 1
#define RELAY_OFF 1
#define RELAY_DEFAULT_ON
#define RELAY_DEFAULT_OFF 1

class S_Devices
{
    private:
        int num_relays;
        int relays[8][3];
        unsigned long relay_turned_on[8];
        JSONVar config;
        String getType(JSONVar device);
        String clearValue(JSONVar key);
        void initRelay(JSONVar device);
        int getPin(JSONVar pin);
    public:
        void init();
        void changeRelay(int relay, String value, String caller);
        S_Devices();
};