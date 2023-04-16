#include <Arduino.h>
#include <Arduino_JSON.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "S_FS.h"
#include "S_Settings.h"
#include "buildTime.h"
#include "S_Common.h"
#ifndef _S_OTA_
#define _S_OTA_
class S_OTA
{
    public:
        static inline unsigned long checkPeriod;
        static void autoUpdate();
        static String getBuildVersion();
        static inline String module_type;
        static void loadConfig();
        static void loop();
        static String getServerVersion(String add_parameter);
    private:
        static inline JSONVar otaSettings;
        static inline unsigned long lastCheckUpdate;
};

void update_started();
void update_finished();
void update_progress(int cur, int total);
void update_error(int err);
#endif