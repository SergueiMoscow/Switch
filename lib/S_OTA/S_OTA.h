#include <Arduino.h>
#include <Arduino_JSON.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "S_FS.h"
#include "S_Settings.h"
#include "buildTime.h"
#include "S_Common.h"

class S_OTA
{
    public:
        static void autoUpdate();
        static String getBuildVersion();
        static String module_type;
        static void loadConfig();
    private:
        static JSONVar otaSettings;
        static unsigned long lastCheckUpdate;

        static String getServerVersion(String add_parameter);
};