#ifndef _S_FS_
#define _S_FS_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

class S_FS
{
public:
    S_FS(); // Конструктор для инициализации файловой системы
    static bool begin();
    int countFiles(const char *dirname);
    String listDir(const char *dirname);
    static String readFile(const char *path);
    static bool writeFile(const char *path, const char *message);
    // static String fileContent(const char *path);
    static bool exists(const char *path);
    static bool readJsonFile(const char *path, JsonDocument& doc);
    static DynamicJsonDocument readJsonFileDynamic(const char *path);
    static bool writeJsonFile(const char *path, const JsonDocument& doc);

};

#endif