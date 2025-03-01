#include "S_Mode.h"

// Инициализация статических переменных
DeviceMode S_Mode::mode = MODE_NORMAL;
String S_Mode::cause = "";
const char* S_Mode::modeFile = "/mode.json";

// Метод инициализации
void S_Mode::begin() {
    S_FS::begin();          // Инициализация файловой системы
    setNormalMode("Ok");       // Загрузка текущего режима из файла
}

// Получить текущий режим
DeviceMode S_Mode::getCurrentMode() {
    if (S_FS::exists(modeFile)) {
        String content = S_FS::readFile(modeFile);
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, content);
        if (!error) {
            mode = static_cast<DeviceMode>(doc["mode"].as<int>());
            cause = doc["cause"].as<String>();
        } else {
            Serial.println("Ошибка десериализации JSON: " + String(error.c_str()));
            // Можно установить режим по умолчанию или обработать ошибку иначе
            mode = MODE_NORMAL;
            cause = "Deserialization error";
        }
    } else {
        // Файл не существует, использовать значения по умолчанию
        mode = MODE_NORMAL;
        cause = "";
        saveModeToFile(); // Опционально: сохранить режим по умолчанию
    }
//    Serial.println("Текущий режим: " + String(mode) + " причина: " + cause);
    return mode;
}

// Установить режим CONFIG_WIFI
void S_Mode::setConfigWifiMode(const String& newCause) {
    if (mode > MODE_CONFIG_WIFI) {
        mode = MODE_CONFIG_WIFI;
        cause = newCause;
        saveModeToFile();
    }
}

// Установить режим CONFIG_MQTT
void S_Mode::setConfigMQTTMode(const String& newCause) {
    if (mode > MODE_CONFIG_MQTT) {
        mode = MODE_CONFIG_MQTT;
        cause = newCause;
        saveModeToFile();
    }
}

// Установить режим CONFIG_OTA
void S_Mode::setConfigOTAMode(const String& newCause) {
    if (mode > MODE_CONFIG_OTA) {
        mode = MODE_CONFIG_OTA;
        cause = newCause;
        saveModeToFile();
    }
}

// Установить режим NORMAL
void S_Mode::setNormalMode(const String& newCause) {
    mode = MODE_NORMAL;
    cause = newCause;
    saveModeToFile();
}

// Сохранить текущий режим и причину в файл
void S_Mode::saveModeToFile() {
    DynamicJsonDocument doc(1024);
    doc["mode"] = static_cast<int>(mode);
    doc["cause"] = cause;

    String output;
    serializeJson(doc, output);
    bool success = S_FS::writeFile(modeFile, output.c_str());
    if (!success) {
        Serial.println("Ошибка записи в файл режима");
        // Можно обработать ошибку, например, повторить попытку или сохранить в другом месте
    }
}