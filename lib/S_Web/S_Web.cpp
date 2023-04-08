#include <Arduino.h>
#include "S_Web.h"
#include "S_FS.h"
#include <Arduino_JSON.h>
#include "S_Settings.h"

ESP8266WebServer server(80);
#define WIFI_JSON "/wifi.json"

// void S_Web::setup()
// {
//     server.on("/", test);
//     server.on("/spiffs", webFSBrowser);
//     server.begin();
// }

//-------------------------------------------
void webServerSetup()
//-------------------------------------------
{
  // void S_Web::setup()
  server.on("/", test);
  server.on("/settings", webSettings);
  server.on("/wifi", webWifi);
  server.on("/spiffs", webFSBrowser);
  server.on("/style.css", webStyle);
  server.begin();
} // end webServerStart function

void webServerLoop()
{
  server.handleClient();
}

void test()
{
  S_FS fs = S_FS();
  JSONVar fileList = fs.listDir("/"); // fs.listDir("/");
  server.send(200, "text/plain", JSON.stringify(fileList));
}

//---------------------webMenu--------------
String webMenu(String current)
//-------------------------------------------
{
  static const String menuItems[] = {"Wifi", "Operate", "Settings", "Servers", "Automation", "System", "Update"};
  static const String menuLinks[] = {"/wifi", "/operate", "/settings", "/servers", "/automation", "/system", "/update"};
  int menuCount = 7;
  String output = "<div class=\"page_top\"><div class='page_tabs'><table class='menu'><tr>";
  for (int i = 0; i < menuCount; i++)
  {
    output += "<td>";
    if (current != menuLinks[i])
    {
      output += "<a href='";
      output += menuLinks[i];
      output += "'>";
    }
    output += "<span>";
    output += menuItems[i];
    output += "</span>";
    if (current != menuLinks[i])
      output += "</a>";
    output += "</td>";
  }
  output += "</table></div></div>";
  return output;
}

  //--------------------webSettings---------------
  void webSettings()
  //-------------------------------------------
  {
    S_FS fs = S_FS();
    S_Settings settings = S_Settings(); //JSON.parse(fs.readFile(WIFI_JSON)); // fs.listDir("/");
    settings.setSettingsFile(WIFI_JSON);
    String output = "";
    if (server.hasArg("ssid") && server.hasArg("wifi_pass") && server.hasArg("hostname"))
    {
      // write settings to file /settings.json
      settings.setSetting("Wifi_Ssid", server.arg("ssid"));
      settings.setSetting("Wifi_Pass", server.arg("wifi_pass"));
      settings.setSetting("Wifi_HostName", server.arg("hostname"));
      settings.setSetting("Config_User", server.arg("config_user"));
      settings.setSetting("Config_Pass", server.arg("config_pass"));
      settings.setSetting("Rely1_Name", server.arg("rely1"));
      settings.setSetting("Rely2_Name", server.arg("rely2"));
      settings.setSetting("Manage_Buttons", (server.hasArg("manageButtons") ? "on" : "off"));

      settings.writeSettings();
      //fs.writeFile(WIFI_JSON, JSON.stringify(settings).c_str());
      //writeSettings();
    }
    // web page
    output += fs.readFile("/header.htm");
    Serial.println(output);
    output += webMenu("/settings");
    output += settings.stringReplace(fs.readFile("/settings.htm"));
    output += fs.readFile("/footer.htm");
    //String output2 = settings.stringReplace(output);
    server.send(200, "text/html", output);
  }


//------------------------------------ webFSBrowser -------
void webFSBrowser()
//-------------------------------------------
{
  String page_content = "";
  String fileName;
  int fileSize;
  LittleFS.begin();
  // download
  // https://esp32.com/viewtopic.php?t=11307
  // AsyncWebServerResponse *response = server.beginResponse(LittleFS, server.arg("filename"), String(), true);
  Serial.println("Вход webFSBrowser");
  if (server.hasArg("format"))
  {
    Serial.println("Formatting");
    LittleFS.format();
    page_content += "Formatted";
  }

  if (server.hasArg("upload"))
  {
    // Добавить upload отсюда:
    //  https://github.com/espressif/arduino-esp32/blob/master/libraries/WebServer/examples/FSBrowser/FSBrowser.ino
    //  для 8266 интересный пример
    //  https://esp8266-arduinoide.ru/fswebserver/
    page_content += "Upload file ";
    page_content += server.arg("filename");
    Serial.println("File: " + server.arg("filename"));
    Serial.println("Upload: " + server.arg("upload"));
  }
  Serial.println("Test" + server.arg("test"));
  if (server.hasArg("NewFile"))
  {
    String page_content = "";
    page_content += "NewFile ";
    page_content += server.arg("NewItemName");
    File file = LittleFS.open(server.arg("NewItemName"), "w");
    if (!file)
    {
      Serial.println("Failed to open file for reading");
      file.print("");
      page_content += " Can't create file";
    }
    Serial.println("After file.print");
    file.close();
  }
  if (server.hasArg("save"))
  {
    File file = LittleFS.open(server.arg("filename"), "w");
    file.print(server.arg("fileContent"));
    Serial.print("writing file ");
    Serial.println(server.arg("filename"));
    Serial.print(server.arg("fileContent"));
    Serial.println();
    file.close();
  }
  if (server.hasArg("delete"))
  {
    LittleFS.remove(server.arg("delete"));
  }
#ifdef ESP32
  File root = LittleFS.open("/");
#elif ESP8266
  Dir root = LittleFS.openDir("/");
#endif
  page_content += "<form method ='POST' action=''>";
  page_content += "<input type='text' name='NewItemName'><br>";
  page_content += "<button name='NewFile'>New File</button>";
  page_content += "</form>";
  page_content += "<form method ='POST' action='' enctype='multipart/form-data'>";
  page_content += "<br><input name='filename' type='file'><br>";
  page_content += "<input type='submit' name='upload' value='upload file'>";
  page_content += "</form>";
  // page_content += "[";
  page_content += "<br>";
  page_content += "<table border='1'>";
#ifdef ESP32
  File file = root.openNextFile();
  while (file)
  {
    fileName = file.name();
    fileSize = file.size();
#endif
#ifdef ESP8266
    Dir file = LittleFS.openDir("/");
    while (root.next())
    {
      File file = root.openFile("r");
      fileName = root.fileName();
      fileSize = file.size();
#endif
      page_content += "<tr><td>";
      if (file.isDirectory())
        page_content += "<b>";
      page_content += (fileName);
      if (file.isDirectory())
        page_content += "</b>";
      page_content += "</td><td>";
      page_content += String(file.size());
      page_content += "</td>";
      // view
      page_content += "<td><a href=\"?view=";
      page_content += file.name();
      page_content += "\"><button>View</button></a></td>";
      // edit
      page_content += "<td><a href=\"?edit=";
      page_content += file.name();
      page_content += "\"><button>Edit</button></a></td>";
      // delete
      page_content += "<td><a href=\"?delete=";
      page_content += file.name();
      page_content += "\"><button>Delete</button></a></td>";

      page_content += "</tr>";
#ifdef ESP32
      file = root.openNextFile();
    }
#elif ESP8266
}
#endif
    page_content += "</table>";
    if (server.hasArg("view"))
    {
      File file = LittleFS.open(server.arg("view"), "r");
      page_content += "<hr>";
      while (file.available())
      {
        page_content += file.readString();
        page_content += "<br>";
      }
      page_content += "<br>";
      page_content += "<hr>";
      file.close();
      // for serial
      file = LittleFS.open(server.arg("view"), "r");
      while (file.available())
        Serial.print(file.readString());
      Serial.println();
      file.close();
    }
    if (server.hasArg("edit"))
    {
      page_content += "<form method='POST' action=''>";
      File file = LittleFS.open(server.arg("edit"), "r");
      page_content += "<hr><textarea rows=\"10\" cols=\"45\" name=\"fileContent\">";
      while (file.available())
        page_content += file.readString();
      page_content += "</textarea><br>";
      page_content += "<input type='hidden' name='filename' value='";
      page_content += server.arg("edit");
      page_content += "'>";
      page_content += "<button name='save'>Save</button>";
      page_content += "<hr>";
      page_content += "</form>";
    }
    LittleFS.end();
    server.send(200, "text/html", page_content);
  }

//-------------------- webWifi --------------
void webWifi()
//-------------------------------------------
  {
    S_FS fs = S_FS();
    S_Settings settings = S_Settings();
    settings.setSettingsFile(WIFI_JSON);
    String output = "";
    if (server.hasArg("ssid") && server.hasArg("wifi_pass"))
    {
      // write settings to file /wifi.json
      settings.setSetting(server.arg("ssid"), server.arg("wifi_pass"));
      settings.writeSettings();
    }
    // web page
    output += fs.readFile("/header.htm");
    Serial.println(output);
    output += webMenu("/wifi");
    output += settings.stringReplace(fs.readFile("/wifi.htm"));
    output += fs.readFile("/footer.htm");
    //String output2 = settings.stringReplace(output);
    server.send(200, "text/html", output);
  }

  //------------------- webStyle --------------
  void webStyle()
  //-------------------------------------------
  {
    S_FS fs = S_FS();
    String output = fs.readFile("/style.css");
    server.send(200, "text/plain", output);

  }

  //--------------------- webLogin ------------
  // void webLogin()
  //-------------------------------------------
  // {
  //   Serial.println("Web login");
  //   if (server.hasArg("login") && server.hasArg("pass"))
  //   {
  //     if (String(server.arg("login")) == commonSettings.getSetting("Config_User", "admin") &&
  //         String(server.arg("pass")) == commonSettings.getSetting("Config_Pass", "admin"))
  //     {
  //     //  webOperate(); Нет в этом проекте
  //       return;
  //     }
  //   }
  //   String output = fileContent("/index.htm");
  //   server.send(200, "text/html", output);
  // }
