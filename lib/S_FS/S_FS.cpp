#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <Arduino_JSON.h>
#include "S_FS.h"

struct FileInfo
{
    String name;
    int size;
    time_t created;
    time_t updated;
};

int S_FS::countFiles(const char *dirname)
{
    int count = 0;
    Dir root = LittleFS.openDir(dirname);
    while (root.next())
    {
        count++;
    }
    return count;
};

JSONVar S_FS::listDir(const char *dirname)
{
    Serial.printf("Listing directory: %s\n", dirname);
    Dir root = LittleFS.openDir(dirname);
    int count = 0;
    String filesList = "{";
    while (root.next())
    {
        File file = root.openFile("r");
        Serial.print("  FILE: ");
        Serial.print(root.fileName());
        Serial.print("  SIZE: ");
        Serial.print(file.size());
        time_t cr = file.getCreationTime();
        time_t lw = file.getLastWrite();
        filesList += "\"f" + (String)count + "\":{\"name\":\"" + root.fileName() + "\", \"size\":" + (String)(file.size()) + "}";
        // filelist[count].name = root.fileName();
        // filelist[count].size = file.size();
        // filelist[count].created = file.getCreationTime();
        // filelist[count].updated = file.getLastWrite();
        file.close();
        struct tm *tmstruct = localtime(&cr);
        Serial.printf("    CREATION: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
        tmstruct = localtime(&lw);
        Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
        count++;
    }
    filesList += "}";
    Serial.println(filesList);
    Serial.println(count);
    Serial.println(JSON.stringify(JSON.parse(filesList)));
    return JSON.parse(filesList);
}

String S_FS::readFile(const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return "";
  }
  String result = "";
  while (file.available()) {
    result += file.readString();
  }
  file.close();
  return result;
}

String S_FS::fileContent(const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return "";
  }
  String result = "";
  while (file.available()) {
    result += file.readString();
  }
  file.close();
  return result;
}


void writeFile(const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = LittleFS.open(path, "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  delay(2000);  // Make sure the CREATE and LASTWRITE times are different
  file.close();
}

