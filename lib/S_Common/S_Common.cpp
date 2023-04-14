#include "S_Common.h"
namespace S_Common
{

    void S_Common::setUTime()
    {
        S_Settings timeSettings = S_Settings();
        timeSettings.setSettingsFile("time.json");
        String local = timeSettings.getSetting("remote", "");
        if (local != "")
        {
            Serial.println("Common: Local time server: " + local);
            checkTime(local);
            // String time = getURL(local);
            // char **pointer, *stringVar;
            // time.toCharArray(stringVar, sizeof(time));
            // Serial.println("Common: local time: " + time);
            // setTime(strtoul(stringVar, NULL, 10));
        }
    }

    unsigned long S_Common::getUTime()
    {
        bool debug = false;
        if (debug) Serial.println("GETTING TIME:");
        unsigned long uTime = now();
        if (debug) Serial.println(uTime);
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
            } // httpCode ok
            else
            {
                playload = "0";
            }
        }
        http.end();
        return playload;
    }

    // void S_Common::checkTime(String url)
    // {
    //     static unsigned long msCheck = 0;
    //     static unsigned long lastGetByUrl = 0;
    //     static int arrLastGetByUrl[6];
    //     long curMillis = millis();
    //     String formattedDate, strJsonDate;
    //     String dayStamp;
    //     String timeStamp;
    //     JSONVar jsonDate;
    //     int arrTime[6];

    //     if ((curMillis > msCheck ? curMillis - msCheck : msCheck - curMillis) > MILLIS_CHECK_TIME || msCheck == 0)
    //     {
    //         strJsonDate = getURL(url);
    //         Serial.print(url);
    //         Serial.println(strJsonDate);
    //         lastGetByUrl = millis();
    //         StaticJsonDocument<512> jsonDate;
    //         deserializeJson(jsonDate, strJsonDate);
    //         // jsonDate=JSON.parse(strJsonDate);
    //         arrLastGetByUrl[0] = jsonDate["Y"];
    //         arrLastGetByUrl[1] = jsonDate["m"];
    //         arrLastGetByUrl[2] = jsonDate["d"];
    //         arrLastGetByUrl[3] = jsonDate["H"];
    //         arrLastGetByUrl[4] = jsonDate["i"];
    //         arrLastGetByUrl[5] = jsonDate["s"];
    //         setTime(arrLastGetByUrl[HOUR], arrLastGetByUrl[MIN], arrLastGetByUrl[SEC], arrLastGetByUrl[DAY], arrLastGetByUrl[MONTH], arrLastGetByUrl[YEAR]);
    //         Serial.print("Checktime passed(lib): ");
    //         Serial.print(day());
    //         Serial.print(" ");
    //         Serial.print(hour());
    //         Serial.print(":");
    //         Serial.println(minute());
    //         msCheck = millis();
    //     }
    // }

    void S_Common::checkTime(String url)
    {
        static unsigned long msCheck = 0;
        static unsigned long lastGetByUrl = 0;
        static int arrLastGetByUrl[6];
        long curMillis = millis();
        String strJsonDate;
        String dayStamp;
        String timeStamp;
        JSONVar jsonDate;
        int arrTime[6];

        if ((curMillis > msCheck ? curMillis - msCheck : msCheck - curMillis) > MILLIS_CHECK_TIME || msCheck == 0)
        {
            bool debug = false;
            strJsonDate = getURL(url);
            if (debug) Serial.print(url);
            if (debug) Serial.println(strJsonDate);
            lastGetByUrl = millis();
            jsonDate=JSON.parse(strJsonDate);
            Serial.println(JSON.stringify(jsonDate["m"]) + "-" + JSON.stringify(jsonDate["d"]));
            arrLastGetByUrl[0] = deleteQuotes(jsonDate["Y"]).toInt();
            arrLastGetByUrl[1] = deleteQuotes(jsonDate["m"]).toInt();
            arrLastGetByUrl[2] = deleteQuotes(jsonDate["d"]).toInt();
            arrLastGetByUrl[3] = deleteQuotes(jsonDate["H"]).toInt();
            arrLastGetByUrl[4] = deleteQuotes(jsonDate["i"]).toInt();
            arrLastGetByUrl[5] = deleteQuotes(jsonDate["s"]).toInt();
            setTime(arrLastGetByUrl[HOUR], arrLastGetByUrl[MIN], arrLastGetByUrl[SEC], arrLastGetByUrl[DAY], arrLastGetByUrl[MONTH], arrLastGetByUrl[YEAR]);
            if (debug) {
                Serial.print("Checktime passed(lib): ");
                Serial.print(day());
                Serial.print(" ");
                Serial.print(hour());
                Serial.print(":");
                Serial.println(minute());
            }
            msCheck = millis();
        }

    }
        String S_Common::deleteQuotes(String str)
        {
            str.replace("\"", "");
            return str;
        }

}