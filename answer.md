Хорошо, давай приведём весь код в `S_MQTT.cpp` к единому стилю с использованием `ArduinoJson` вместо `JSONVar`. Я заменю все оставшиеся места, где используется `JSONVar`, включая `publish()`, где ты получаешь данные от `devices->getJsonRelayValuesForPublish()`. Предположим, что `S_Devices::getJsonRelayValuesForPublish()` возвращает `JSONVar`, и его тоже нужно будет адаптировать (если это так, я покажу, как это сделать).

---

### Обновлённый `S_MQTT.cpp`

Вот полный код с заменой `JSONVar` на `ArduinoJson`:

```cpp
#include "S_MQTT.h"

void S_MQTT::init(PubSubClient* client, S_Devices* devicesPtr)
{
    mqttClient = client;
    devices = devicesPtr;
    loadConfig();
}

void S_MQTT::loadConfig()
{
    S_FS fs = S_FS();
    bool found = false;
    Serial.println("S_MQTT.cpp reading mqtt settings");

    if (!S_FS::exists(MQTT_SETTINGS_FILE)) {
        S_Mode::setConfigMQTTMode("S_MQTT.cpp No MQTT settings found");
        return;
    }

    DynamicJsonDocument doc(512); // Подбери размер под свой JSON
    if (!fs.readJsonFile(MQTT_SETTINGS_FILE, doc)) {
        Serial.println("Failed to read or parse MQTT settings");
        return;
    }

    if (doc.isNull() || doc.as<JsonObject>().size() == 0) {
        Serial.println("MQTT settings JSON is empty or invalid");
        return;
    }

    JsonObject root = doc.as<JsonObject>();
    for (JsonPair kv : root) {
        Serial.println("Checking MQTT " + String(kv.key().c_str()));
        JsonObject settings = kv.value().as<JsonObject>();
        if (clearValue(settings["Active"]) == "1") {
            mqttSettings = settings;
            periodSec = atoi(clearValue(settings["Period"]).c_str());
            if (periodSec < 10) {
                periodSec = 10;
            }
            Serial.println("PeriodSec: " + String(periodSec));
            found = true;
            break;
        }
    }

    if (found) {
        setServer();
        setRootTopic();
        connect();
        Serial.println("MQTT settings loaded");
    } else {
        Serial.println("Active MQTT is not configured");
    }
}

void callback(char *topic, byte *msg, unsigned int len)
{
    extern S_Devices devices;
    extern S_MQTT sMQTT;
    Serial.println("Callback: " + String(topic) + ": ");
    String message;
    for (int i = 0; i < len; i++) {
        message += (char)msg[i];
    }
    Serial.print("Callback: " + String(topic) + ": " + message);
    devices.callback(String(topic), message);
    sMQTT.publish(false);
}

void S_MQTT::setServer()
{
    mqttServer = clearValue(mqttSettings["Server"]);
    const char *server = mqttServer.c_str();
    if (server[0] == '\0') { // Проверяем пустую строку
        Serial.println("MQTT Server is not configured");
        return;
    }
    int port = atoi(clearValue(mqttSettings["Port"]).c_str());
    Serial.print("Trying to set MQTT server: \"");
    Serial.print(server);
    Serial.println("\" port: " + String(port));
    mqttClient->setServer(server, port);
    mqttClient->setKeepAlive(60); // Keep-alive 60 секунд
    mqttClient->setCallback(callback);
}

void S_MQTT::connect()
{
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected, skipping MQTT connect");
        return;
    }

    // Проверяем наличие пользователя и пароля
    if (mqttSettings["User"].isNull() || mqttSettings["Password"].isNull()) {
        Serial.println("User or password is not configured");
        Serial.println("Current settings: ");
        serializeJson(mqttSettings, Serial);
        loadConfig();
        return;
    }

    String clientId = WiFi.macAddress();
    String user = clearValue(mqttSettings["User"], "user");
    String password = clearValue(mqttSettings["Password"], "pass");
    if (user == "") user = "user";
    if (password == "") password = "pass";

    Serial.print("LastTryConnect: ");
    Serial.println(lastTryConnect);
    Serial.print("Passed: ");
    Serial.println(millis() - lastTryConnect);
    Serial.println("id: " + clientId + ", user: " + user + ", pass: " + password);

    if (lastTryConnect == 0 || millis() - lastTryConnect > 10000UL) {
        if (mqttClient->connect(clientId.c_str(), user.c_str(), password.c_str()), true) {
            Serial.println("Connected to MQTT ");
            publish(false);
            String subscribeString = getSubscribeString();
            Serial.println("Subscribe to: " + subscribeString);
            mqttClient->subscribe(subscribeString.c_str());
            isConfigured = true;
        } else {
            Serial.print("Failed to connect to MQTT, state: ");
            Serial.println(mqttClient->state());
        }
        lastTryConnect = millis();
    }
}

String S_MQTT::clearValue(JsonVariant value)
{
    if (value.isNull()) {
        return "";
    }
    String result = value.as<String>();
    result.replace("\"", "");
    return result;
}

String S_MQTT::clearValue(JsonVariant value, String default_value)
{
    if (value.isNull()) {
        return default_value;
    }
    String result = value.as<String>();
    result.replace("\"", "");
    return result;
}

String S_MQTT::getSubscribeString()
{
    return rootTopic + SUBSCRIBE_POSTFIX + "/#";
}

void S_MQTT::setRootTopic()
{
    S_Settings globalSettings;
    Serial.print("S_MQTT setSettingsFile (Global)");
    globalSettings.setSettingsFile(GLOBAL_SETTINGS_FILE);
    String object = globalSettings.getSetting("object", "default");
    String room = globalSettings.getSetting("room", "default");
    String device = globalSettings.getSetting("device", "default");
    rootTopic = object + "/" + room + "/" + device + "/";
}

void S_MQTT::publish(bool force)
{
    String publishTopic = rootTopic + "state";
    
    // Предполагаем, что devices->getJsonRelayValuesForPublish() возвращает JSONVar
    // Нужно адаптировать под ArduinoJson
    DynamicJsonDocument doc(512); // Подбери размер под данные от devices
    JsonObject devicesValues = doc.to<JsonObject>();
    
    // Копируем данные из devices (предполагаем адаптацию в S_Devices)
    JsonObject relayValues = devices->getJsonRelayValuesForPublish(); // Должно возвращать JsonObject
    for (JsonPair kv : relayValues) {
        devicesValues[kv.key()] = kv.value();
    }
    
    devicesValues["time"] = S_Common::S_Common::getUTime();

    String jsonString;
    serializeJson(devicesValues, jsonString);
    Serial.println("Mqtt.publish: " + publishTopic + " " + jsonString);

    mqttClient->publish(publishTopic.c_str(), jsonString.c_str(), true);

    for (JsonPair kv : devicesValues) {
        String topic = rootTopic + clearValue(kv.key());
        String value = clearValue(kv.value());
        mqttClient->publish(topic.c_str(), value.c_str(), true);
    }
}

void S_MQTT::loop()
{
    unsigned long curMillis = millis();
    
    if (curMillis - lastPublished >= periodSec * 1000UL) {
        if (isConfigured && !mqttClient->connected()) {
            Serial.println("MQTT not connected");
            connect();
        }
        S_Common::S_Common::getUTime();
        Serial.print("S_MQTT.cpp loop ");
        Serial.println(S_Common::S_Common::getTime());
        if (mqttClient->connected()) {
            publish(false);
            lastPublished = curMillis;
        }
    }
    
    mqttClient->loop();
    
    if (devices->loop() > 0) {
        publish(false);
    }
}
```

---

### Основные изменения

1. **Метод `connect`**:
   - Убраны `JSONVar jUser, jPass`, теперь проверка `isNull()` напрямую через `mqttSettings`.
   - Для отладки используется `serializeJson` вместо `JSON.stringify`.

2. **Метод `publish`**:
   - Заменил `JSONVar devicesValues` на `JsonObject` из `DynamicJsonDocument`.
   - Предполагается, что `S_Devices::getJsonRelayValuesForPublish()` нужно адаптировать (см. ниже).
   - Для сериализации используется `serializeJson` вместо `JSON.stringify`.
   - Итерация по ключам теперь через `JsonPair`.

3. **Метод `loop`**:
   - Заменил `max/min` на более надёжную проверку с учётом переполнения `millis()`.

---

### Адаптация `S_Devices::getJsonRelayValuesForPublish`

Если `S_Devices::getJsonRelayValuesForPublish()` возвращает `JSONVar`, его нужно переписать под `ArduinoJson`. Вот пример, как это может выглядеть (без полного кода `S_Devices`, предполагаю структуру):

```cpp
// В S_Devices.h
JsonObject getJsonRelayValuesForPublish();

// В S_Devices.cpp
JsonObject S_Devices::getJsonRelayValuesForPublish()
{
    StaticJsonDocument<256> doc; // Подбери размер под данные
    JsonObject obj = doc.to<JsonObject>();
    
    // Пример заполнения (адаптируй под свои данные)
    obj["relay1"] = "on";
    obj["relay2"] = "off";
    
    return obj;
}
```

- Теперь метод возвращает `JsonObject`, который можно использовать в `publish()`.

---

### Проверка и отладка
1. **Размер `DynamicJsonDocument`**:
   - В `loadConfig` и `publish` я указал `512` байт. Если твой JSON больше, используй [ArduinoJson Assistant](https://arduinojson.org/v6/assistant/) для расчёта точного размера.
2. **Совместимость**:
   - Проверь, что `S_Devices` и другие зависимости работают с новым кодом.
3. **Логи**:
   - Если что-то не загрузится или не опубликуется, добавь больше `Serial.println` для отладки.

Этот код должен полностью избавиться от `JSONVar`. Если остались вопросы или что-то не работает, дай знать — доработаем!