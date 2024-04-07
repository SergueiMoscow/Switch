#include <Arduino.h>
#include "S_Web.h"

ESP8266WebServer server(80);
File fsUploadFile;
#define WIFI_JSON "/wifi.json"

void replyOK() {
  server.send(200, "text/plain", "");
}

void handleFileUpload() {
  Serial.println("handleFileUpload ");
    String filename;
    HTTPUpload& upload = server.upload();
    Serial.print("Upload status: ");
    Serial.println(upload.status);
    if (upload.status == UPLOAD_FILE_START) {
      filename = upload.filename;
      Serial.print("Upload file start ");
      Serial.println(filename);
      if (!filename.startsWith("/")) filename = "/" + filename;
      fsUploadFile = LittleFS.open(filename, "w");
      if (!fsUploadFile) { Serial.print("Error creating file"); Serial.println(filename); }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
      if (fsUploadFile)
      {
        fsUploadFile.write(upload.buf, upload.currentSize);
        Serial.println("Uploading: ");
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      Serial.println("Upload file end");
      if (fsUploadFile)
        fsUploadFile.close();
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location", "/spiffs");
      server.send(303);
    }
}


//-------------------------------------------
void webServerSetup()
//-------------------------------------------
{
  // void S_Web::setup()
  server.on("/", test);
  server.on("/settings", webSettings);
  server.on("/reset", webReset);
  server.on("/wifi", webWifi);
  server.on("/spiffs", webFSBrowser);
  server.on("/style.css", webStyle);
  server.on("/update", webUpdate);
  server.on("/upload", HTTP_POST, replyOK, handleFileUpload);

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
  // S_FS fs = S_FS();
  S_Settings settings = S_Settings(); // JSON.parse(fs.readFile(WIFI_JSON)); // fs.listDir("/");
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
    // fs.writeFile(WIFI_JSON, JSON.stringify(settings).c_str());
    // writeSettings();
  }
  // web page
  //output += fs.readFile("header.htm");
  output += S_FS::fileContent("header.htm");
  Serial.println(output);
  output += webMenu("/settings");
//  output += settings.stringReplace(fs.readFile("settings.htm"));
  output += settings.stringReplace(S_FS::fileContent("settings.htm"));
  //output += fs.readFile("footer.htm");
  output += S_FS::fileContent("footer.htm");
  // String output2 = settings.stringReplace(output);
  server.send(200, "text/html", output);
}

//------------------------------------ webFSBrowser -------
void webFSBrowser()
//-------------------------------------------
{
  String page_content = "";
  String fileName;
  // int fileSize;
  LittleFS.begin();
  // download
  // https://esp32.com/viewtopic.php?t=11307
  // AsyncWebServerResponse *response = server.beginResponse(LittleFS, server.arg("filename"), String(), true);
  Serial.println("Вход: webFSBrowser");
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
  Serial.println("Test filename" + server.arg("filename"));
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
  page_content += "<form method =\"POST\" action=\"/upload\" enctype=\"multipart/form-data\" content-type=\"multipart/form-data\">";
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
    // fileSize = file.size();
#endif
#ifdef ESP8266
    Dir file = LittleFS.openDir("/");
    while (root.next())
    {
      File file = root.openFile("r");
      fileName = root.fileName();
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
#endif
#ifdef ESP8266
  }
#endif
  page_content += "</table>";
  if (server.hasArg("view"))
  {
    File file = LittleFS.open(server.arg("view"), "r");
    if (!file)
    {
      Serial.println("Failed to open file for reading");
      page_content += "<b>Failed to open file for reading</b>";
    }
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
  server.send(200, "text/html", page_content);
}

//-------------------- webWifi --------------
void webWifi()
//-------------------------------------------
{
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
  //output += fs.readFile("header.htm");
  output += S_FS::fileContent("header.htm");
  Serial.println(output);
  output += webMenu("/wifi");
  //output += settings.stringReplace(fs.readFile("wifi.htm"));
  output += settings.stringReplace(S_FS::fileContent("wifi.htm"));
  //output += fs.readFile("footer.htm");
  output += S_FS::fileContent("footer.htm");
  // String output2 = settings.stringReplace(output);
  server.send(200, "text/html", output);
}

//------------------- webStyle --------------
void webStyle()
//-------------------------------------------
{
  //S_FS fs = S_FS();
  String output = S_FS::fileContent("style.css");
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

void webReset()
{
  ESP.restart();
}

void webUpdate() {

  Serial.println("WebUpdate");
  static bool varAutoUpdate = false;
  String html = S_FS::fileContent("header.htm");
  JSONVar otaSettings = JSON.parse(S_FS::fileContent("ota.json"));
  String module_type = S_Settings::delQuotes(otaSettings["type"]);
  String url_update = S_Settings::delQuotes(otaSettings["url_update"]);
  String version = S_OTA::getBuildVersion();
  html += "<title>";
  html += module_type;
  html += "</title></head>";
  html += webMenu("/update");
  html += "<br><br><a href='/update?reboot=true'>Reboot</a>";
  html += "<br><br><a href='/update?serialcheck=true'>Serialcheck</a>";

  html += "<br>";

  html += "<br><a href='/update?otaupdateinfo=true'>OTAWEB firmware check</a>";

  if (ESP.getFlashChipSize() > 900000)
  {
    html += "<br><font color=\"red\">Check your new firmware compile time size! must be 1mbyte+</font>";
  }
  else
  {
    html += "<br><font color=\"red\">Your flash ";
    html += String(ESP.getFlashChipSize());
    html += " bytes only, it's too small for OTA WEB</font>";
  }
  //s += "<br><a href='/update&otaupdate'>OTAWEB firmware update</a>";

  html += "<br>";
  html += "module type: ";
  html += module_type;
  html += "<br>";
  html += "module fw: ";
  html += version;
  WiFiClient espClient;

  if (server.hasArg("otaupdateinfo"))
  {
    html += "<br>";
    html += "server firmware: ";
    html += url_update + "?info=true&module=" + module_type;

    HTTPClient http;
    http.begin(espClient, url_update + "?info=true&module=" + module_type);
    int httpCode = http.GET();
    if (httpCode > 0) {
      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        if (payload.length() > 0 && payload != "firmware type error")
        {
          html += payload;
          if (payload != version)
            html += "<br><a href='/update?otaupdate=true'>OTAWEB firmware update</a>";
        }
        else
          html += "server error";
      }
    } else {
      html += "[HTTP] GET... failed";
    }

    http.end();
  }
  else if (server.hasArg("otaupdate"))
  {
    Serial.println("OTAWEB update request");

    WiFiClient espClient;
    if (varAutoUpdate) {
      html = "HTTP/1.1 307 Temporary Redirect";
      html += "\r\nLocation: /update";
      html += "\r\n\r\n";
      server.sendContent(html);
      return;
    } 
    
    t_httpUpdate_return ret;
    varAutoUpdate = true;

    ret = HTTP_UPDATE_OK;
    html += "<br>";
    html += "<br>";

    switch (ret) {
      case HTTP_UPDATE_FAILED:
        html += url_update + "?module=" + module_type;
        html += "<br>HTTP_UPDATE_FAILD Error (";
#ifdef ESP32
        html += httpUpdate.getLastError();
        html += "): ";
        html += httpUpdate.getLastErrorString().c_str();
#endif
#ifdef ESP8266
        html += ESPhttpUpdate.getLastError();
        html += "): ";
        html += ESPhttpUpdate.getLastErrorString().c_str();
#endif
        break;

      case HTTP_UPDATE_NO_UPDATES:
        html += "HTTP_UPDATE_NO_UPDATES";
        break;

      case HTTP_UPDATE_OK:
        html += "HTTP_UPDATE_OK";
        break;
    }
  }
  else if (   server.hasArg("serialcheck"))
  {
    Serial.println("serial check");
    html = "HTTP/1.1 307 Temporary Redirect";
    html += "\r\nLocation: /update";
    html += "\r\n\r\n";
    server.sendContent(html);
    return;
  }
  else if (   server.hasArg("reboot"))
  {
    html = "<head>";
    html += "<meta http-equiv=\"refresh\" content=\"20;url=/\">";
    html += "</head>";
    html += "REDIRECTING in 20S";



    html += "</html>\r\n\r\n";
    /// TODO: Так не катит, надо как-то контролировать
    // rebootReq = true;
  }
  html += "</html>\r\n\r\n";
  server.send(200, "text/html", html);
}
