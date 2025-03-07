#include "S_FS.h"

S_FS::S_FS()
{
    if (!LittleFS.begin())
    {
        Serial.println("Ошибка монтирования файловой системы");
    }
}


bool S_FS::begin() {
    return LittleFS.begin();
}

int S_FS::countFiles(const char *dirname)
{
    int count = 0;
    Dir root = LittleFS.openDir(dirname);
    while (root.next())
    {
        count++;
    }
    return count;
}

String S_FS::listDir(const char *dirname)
{
    StaticJsonDocument<1024> doc; // Размер документа нужно подобрать в зависимости от ожидаемых данных
    JsonObject root = doc.to<JsonObject>();

    Serial.printf("Listing directory: %s\n", dirname);
    Dir dir = LittleFS.openDir(dirname);
    int count = 0;

    while (dir.next())
    {
        File file = dir.openFile("r");
        if (!file)
        {
            Serial.println("Не удалось открыть файл");
            continue;
        }

        JsonObject fileObj = root.createNestedObject("f" + String(count));
        fileObj["name"] = dir.fileName();
        fileObj["size"] = file.size();
        fileObj["created"] = file.getCreationTime();
        fileObj["updated"] = file.getLastWrite();

        file.close();
        count++;
    }

    String output;
    serializeJson(doc, output);
    Serial.println(output);
    return output;
}

String S_FS::readFile(const char *path)
{
//    Serial.printf("Reading file: %s\n", path);

    File file = LittleFS.open(path, "r");
    if (!file)
    {
        Serial.println("Не удалось открыть файл для чтения");
        return "";
    }

    String result;
    while (file.available())
    {
        result += (char)file.read();
    }
    file.close();
    return result;
}

bool S_FS::exists(const char *path)
{
    return LittleFS.exists(path);
}

bool S_FS::readJsonFile(const char *path, JsonDocument& doc)
{
    Serial.printf("Reading JSON file: %s\n", path);

    File file = LittleFS.open(path, "r");
    if (!file)
    {
        Serial.printf("Не удалось открыть JSON файл: %s\n", path);
        return false;
    }

    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
        Serial.print(F("Ошибка парсинга JSON в файле "));
        Serial.print(path);
        Serial.print(F(": "));
        Serial.println(error.f_str());
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool S_FS::writeFile(const char *path, const char *message)
{
    Serial.printf("Writing file: %s\n", path);

    File file = LittleFS.open(path, "w");
    if (!file)
    {
        Serial.println("Не удалось открыть файл для записи");
        return false;
    }

    if (file.print(message))
    {
        Serial.println("Файл успешно записан");
        return true;
    }
    else
    {
        Serial.println("Не удалось записать данные в файл");
        return false;
    }

    delay(2000); // Убедиться, что CREATE и LASTWRITE разные
    file.close();
    return true;
}

DynamicJsonDocument S_FS::readJsonFileDynamic(const char *path) {
    Serial.printf("Reading JSON file dynamically: %s\n", path);

    File file = LittleFS.open(path, "r");
    if (!file) {
        Serial.printf("Failed to open JSON file: %s\n", path);
        return DynamicJsonDocument(0); // Возвращаем пустой документ при ошибке
    }

    size_t fileSize = file.size();
    if (fileSize == 0) {
        Serial.printf("File %s is empty\n", path);
        file.close();
        return DynamicJsonDocument(0);
    }

    // Создаём документ с запасом памяти (fileSize + запас для метаданных)
    DynamicJsonDocument doc(fileSize + 256);
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.printf("Failed to parse JSON in file %s: %s\n", path, error.c_str());
        return DynamicJsonDocument(0); // Пустой документ при ошибке парсинга
    }

    Serial.printf("Successfully parsed JSON file %s, size: %d bytes\n", path, fileSize);
    return doc;
}

bool S_FS::writeJsonFile(const char *path, const JsonDocument& doc) {
    Serial.printf("Writing JSON file: %s\n", path);

    File file = LittleFS.open(path, "w");
    if (!file) {
        Serial.printf("Failed to open JSON file for writing: %s\n", path);
        return false;
    }

    serializeJson(doc, file);
    file.close();

    return true;
}
