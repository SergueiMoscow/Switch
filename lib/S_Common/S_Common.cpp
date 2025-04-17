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
    // Возвращает текущее время
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
        const bool debug = false;  // Выносим константу на уровень функции
        static unsigned long msCheck = 0;
        static unsigned long lastGetByUrl = 0;
        unsigned long curMillis = millis();
    
        // Проверяем, нужно ли обновлять время
        if ((curMillis > msCheck ? curMillis - msCheck : msCheck - curMillis) > MILLIS_CHECK_TIME 
            || msCheck == 0 
            || force)
        {
            String strJsonDate = getURL(url);
            
            if (debug) {
                Serial.print("URL: ");
                Serial.println(url);
                Serial.print("Response: ");
                Serial.println(strJsonDate);
            }
    
            lastGetByUrl = millis();
    
            // Парсим JSON с помощью ArduinoJson
            DynamicJsonDocument doc(1024);  // Создаем документ с буфером 1024 байта
            DeserializationError error = deserializeJson(doc, strJsonDate);
    
            if (!error) {
                // Извлекаем unixtime
                long unixtime = doc["unixtime"].as<long>();
                
                if (debug) {
                    Serial.println("S_Common.cpp: Debug info:");
                    Serial.print("unixtime: ");
                    Serial.println(unixtime);
                    Serial.print("datetime: ");
                    Serial.println(doc["datetime"].as<String>());
                }
    
                setTime(unixtime);
            } else {
                if (debug) {
                    Serial.print("Failed to parse JSON: ");
                    Serial.println(error.c_str());
                }
                // Можно добавить return false в случае ошибки парсинга
            }
        }
    
        msCheck = millis();
        return now() > getBuildUnixTime();
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

    int getPin(const String& pinStr) {
        if (pinStr.startsWith("D")) {
            if (pinStr == "D0") return D0;
            if (pinStr == "D1") return D1;
            if (pinStr == "D2") return D2;
            if (pinStr == "D3") return D3;
            if (pinStr == "D4") return D4;
            if (pinStr == "D5") return D5;
            if (pinStr == "D6") return D6;
            if (pinStr == "D7") return D7;
            if (pinStr == "D8") return D8;
            if (pinStr == "D9") return D9;
            if (pinStr == "D10") return D10;
            if (pinStr == "D11") return D11;
            if (pinStr == "D12") return D12;
            if (pinStr == "D13") return D13;
            if (pinStr == "D14") return D14;
            if (pinStr == "D15") return D15;
        } else if (pinStr == "A0") {
            return A0;
        }
        return atoi(pinStr.c_str());
    }
    

}