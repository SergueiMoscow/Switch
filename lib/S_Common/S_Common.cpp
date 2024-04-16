#include "S_Common.h"
// #include <ESP8266WiFi.h>
// #include <ArduinoJson.h>

namespace S_Common
{

    void S_Common::setUTime()
    {
        S_Settings timeSettings = S_Settings();
        timeSettings.setSettingsFile("time.json");
        String local = timeSettings.getSetting("local", "");
        String remote = timeSettings.getSetting("remote", "");
        if (remote != "")
        {
            Serial.println("Common: Remote time server: " + remote);
            if (!checkTime(remote)) {
                Serial.println("Common: Can't set time from " + remote);
                if (!checkTime(local)) {
                    Serial.println("Common: Can't set time from " + local);
                }
            }
        }
    }

    unsigned long S_Common::getUTime()
    {
        bool debug = false;
        if (debug) Serial.println("GETTING TIME:");
        unsigned long uTime = now();
        // if (debug) Serial.println(uTime);
        return uTime;
    }

    //===================================
    // getTime (common)
    //===================================
    String S_Common::getTime(String type = "A")
    {
        String cYear = String(year());
        String cMonth = String(month());
        if (cMonth.toInt() < 10)
            cMonth = "0" + cMonth;
        String cDay = String(day());
        if (cDay.toInt() < 10)
            cDay = "0" + cDay;
        String cHour = String(hour());
        if (cHour.toInt() < 10)
            cHour = "0" + cHour;
        String cMinute = String(minute());
        if (cMinute.toInt() < 10)
            cMinute = "0" + cMinute;
        String cSecond = String(second());
        if (cSecond.toInt() < 10)
            cSecond = "0" + cSecond;
        if (type == "D")
            return cYear + "-" + cMonth + "-" + cDay;
        else if (type == "T")
            return cHour + ":" + cMinute + ":" + cSecond;
        else
            return cYear + "-" + cMonth + "-" + cDay + " " + cHour + ":" + cMinute + ":" + cSecond;
    }

    String S_Common::getTime()
    {
        return getTime("A");
    }

    String S_Common::getURL(String url)
    {
        // возвращает содержание страницы (для time.php, например)
        WiFiClient espClient;
        HTTPClient http;
        http.begin(espClient, url);
        String playload = "0";
        Serial.print("GetUrl: " + url + " :");
        int httpCode = http.GET();
        Serial.println(httpCode);

        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            // USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
            // file found at server
            if (httpCode == HTTP_CODE_OK)
            {
                playload = http.getString();
                Serial.print("S_Common: playload:");
                Serial.println(playload);
            } // httpCode ok
            else
            {
                playload = "0";
            }
        }
        http.end();
        return playload;
    }

    bool S_Common::checkTime(String url)
    {
        bool debug = false;
        static unsigned long msCheck = 0;
        static unsigned long lastGetByUrl = 0;
        unsigned long curMillis = millis();
        String strJsonDate;
        String dayStamp;
        String timeStamp;
        JSONVar jsonDate;

        if ((curMillis > msCheck ? curMillis - msCheck : msCheck - curMillis) > MILLIS_CHECK_TIME || msCheck == 0)
        {
            bool debug = false;
            strJsonDate = getURL(url);
            if (debug) Serial.print(url);
            if (debug) Serial.println(strJsonDate);
            lastGetByUrl = millis();
            jsonDate=JSON.parse(strJsonDate);
            long unixtime = (long)jsonDate["unixtime"];
            if (debug) {
                Serial.print("S_Common.cpp: jsonDate: ");
                Serial.println(jsonDate);
                Serial.print("S_Common.cpp: unixtime: ");
                Serial.println(JSON.stringify(jsonDate["unixtime"]));
                Serial.println(JSON.stringify(jsonDate["datetime"]));
                Serial.print("Long unixtime: ");
                Serial.println(unixtime);
            }
            setTime(unixtime);
        }
        msCheck = millis();
        return now() > (time_t)(1681479472UL);
    }

    String S_Common::deleteQuotes(String str)
    {
        str.replace("\"", "");
        return str;
    }
}