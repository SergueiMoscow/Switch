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
            if (!checkTime(remote, true)) {
                Serial.println("Common: Can't set time from " + remote);
                if (!checkTime(local, true)) {
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
        // espClient.setInsecure();
        // http.useHTTP10(true);

        if (!http.begin(espClient, url)) {
            Serial.println("HTTPClient: Не удалось инициализировать соединение");
            return "0";
        };
        String host = extractHost(url);
        http.addHeader("Host", host);
        http.addHeader("User-Agent", "X-Device");
        Serial.print("added header: Host: " + host + " User-Agent: X-Device");

        String playload = "0";
        Serial.print("GetUrl: " + url + " (header: Host " + host + "): " );
        int httpCode = http.GET();
        Serial.println("Httpcode: " + httpCode);

        Serial.printf("Response code: %u\n",httpCode);
        Serial.printf("Content length: %u\n",http.getSize());
        // HTTP header has been send and Server response header has been handled
        // USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
        // file found at server
        if (httpCode == HTTP_CODE_OK)
        {
            playload = http.getString();
            Serial.print("S_Common: playload:");
            Serial.println(playload);
        } // httpCode ok

        http.end();
        return playload;
    }

    bool S_Common::checkTime(String url, bool force = false)
    {
        bool debug = false;
        static unsigned long msCheck = 0;
        static unsigned long lastGetByUrl = 0;
        unsigned long curMillis = millis();
        String strJsonDate;
        String dayStamp;
        String timeStamp;
        JSONVar jsonDate;

        if ((curMillis > msCheck ? curMillis - msCheck : msCheck - curMillis) > MILLIS_CHECK_TIME || msCheck == 0 || force)
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


    String S_Common::extractHost(const String& url) {
        String host = "unknown";
        
        int scheme_end = url.indexOf("://");
        int host_start;

        if (scheme_end != -1) {
            host_start = scheme_end + 3; // Пропустить "://"
        } else {
            host_start = 0; // Отсутствие схемы
        }

        int host_end = url.indexOf('/', host_start);
        if (host_end == -1) {
            host_end = url.length();
        }

        if (host_start < host_end) {
            host = url.substring(host_start, host_end);
            
            int port_pos = host.indexOf(':');
            if (port_pos != -1) {
                host = host.substring(0, port_pos);
            }
        }

        return host;
    }

}