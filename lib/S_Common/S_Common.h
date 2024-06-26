#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
//#include <WiFiClientSecure.h> // HTTPS library 
#include "S_Settings.h"
#include "TimeLib.h"
#include <Arduino_JSON.h>
#include <time.h>

#ifndef _S_Common_
#define _S_Common_
#define YEAR 0
#define MONTH 1
#define DAY 2
#define HOUR 3
#define MIN 4
#define SEC 5
// #define MILLIS_CHECK_TIME 60*60*1000
#define MILLIS_CHECK_TIME  10000


namespace S_Common
{
    class S_Common
    {
        private:
        static String deleteQuotes(String);
        public:
        static String getURL(String url);
        static void setUTime();
        static unsigned long getUTime();
        static bool checkTime(String url);
        static String getTime(String type);
        static String getTime();
        static String generate_uuid_v4();
    };
}
#endif