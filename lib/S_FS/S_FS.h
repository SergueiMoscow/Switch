#ifndef _S_FS_
#define _S_FS_
#include <Arduino.h>
#include <Arduino_JSON.h>

class S_FS
{
    public:
    int countFiles(const char *dirname);
    JSONVar listDir(const char *dirname);
    String readFile(const char *path);
    void writeFile(const char *path, const char *message);
    static String fileContent(const char *path);
    static bool exists(const char *path);

    // String listDir();
};
#endif