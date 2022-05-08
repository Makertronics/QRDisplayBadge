#define TFT_DC    D1     // TFT DC  pin is connected to NodeMCU pin D1 (GPIO5)
#define TFT_RST   D2     // TFT RST pin is connected to NodeMCU pin D2 (GPIO4)
#define TFT_CS    D8     // TFT CS  pin is connected to NodeMCU pin D8 (GPIO15)
#define TFT_BLK   D4     // TFT BLK pin is connected to NodeMCU Pin D4 (GPIO2)

#define TFT_MOSI 19
#define TFT_SCLK 18  

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_GFX.h>      
#include <Adafruit_ST7789.h>   
#include <SPI.h>
#include <FS.h>
#include "qrcode_st7789.h"
#include <ArduinoJson.h>

//edit the SSID and Password 
#define APSSID "ESP_AP"
#define APPSK  "esppassword"

struct Config {
  char Default[256];
  char Instructables[256];
  char Youtube[256];
  char Instagram[256];
  char LinkedIn[256];
};

//config file for buttons on webpage
const char *configFilename = "/config.json";  
Config config;  

ESP8266WebServer server(80);
Adafruit_ST7789 display = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
QRcode_ST7789 qrcode (&display);


void loadConfiguration(const char *filename, Config &config) {
  File file = SPIFFS.open(filename, "r");
  
  StaticJsonDocument<1280> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  strlcpy(config.Default,doc["default"],sizeof(config.Default));  
  strlcpy(config.Instructables,doc["Instructables"],sizeof(config.Instructables));   
  strlcpy(config.Youtube,doc["Youtube"],sizeof(config.Youtube));   
  strlcpy(config.Instagram,doc["Instagram"],sizeof(config.Instagram));   
  strlcpy(config.LinkedIn,doc["LinkedIn"],sizeof(config.LinkedIn));          
  
  Serial.println(config.Default);
  Serial.println(config.Instructables);
  Serial.println(config.Youtube);
  Serial.println(config.Instagram);
  Serial.println(config.LinkedIn);
  
  // Close the file
  file.close();
}

void saveConfiguration(const char *filename, const Config &config) {
  // Delete existing file, otherwise the configuration is appended to the file
  SPIFFS.remove(filename);

  File file = SPIFFS.open(filename, "w");
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  StaticJsonDocument<1280> doc;

  // Set the values in the document
  doc["default"] = config.Default;
  doc["Instructables"] = config.Instructables ;
  doc["Youtube"] = config.Youtube;
  doc["Instagram"] = config.Instagram;
  doc["LinkedIn"] = config.LinkedIn;

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  file.close();
}

void handleRoot() {
  File htmlFile = SPIFFS.open("/index.html","r");
  server.streamFile(htmlFile,"text/html");
  htmlFile.close();
}

void QR_DataHandler(){
    String message = "";
    if (server.arg("Data")== ""){     //Parameter not found
      message = "Data Argument not found";
    }else{     //Parameter found
      message = "Data Argument = ";
      message += server.arg("Data");     //Gets the value of the query parameter
      qrcode.create(server.arg("Data"));
      Serial.print("New QR Created\n\rData:");
      Serial.println(server.arg("Data"));

      server.arg("Data").toCharArray(config.Default,server.arg("Data").length()+1);
      saveConfiguration(configFilename, config);
    }
    
    File htmlFile = SPIFFS.open("/index.html","r");
    server.streamFile(htmlFile,"text/html");
    htmlFile.close();
    
}

void JSON_Handler(){
  File JSON_File = SPIFFS.open("/config.json","r");
  if(JSON_File) Serial.println("sending JSON File");
  server.streamFile(JSON_File,"application/json");
  JSON_File.close();
}

void EditPage_Handler(){
  if(server.args() != 0){   //when any parameters are passed  
      server.arg("Instructables").toCharArray(config.Instructables,server.arg("Instructables").length()+1);
      server.arg("Youtube").toCharArray(config.Youtube,server.arg("Youtube").length()+1);
      server.arg("Instagram").toCharArray(config.Instagram,server.arg("Instagram").length()+1);
      server.arg("LinkedIn").toCharArray(config.LinkedIn,server.arg("LinkedIn").length()+1);

      saveConfiguration(configFilename, config);
      server.send(200, "text/plain", "parameters fetched");
    }  
  else{
    File htmlFile = SPIFFS.open("/EditPage.html","r");
    server.streamFile(htmlFile,"text/html");
    htmlFile.close();
  }
}

void setup() {
    Serial.begin(115200);
    Serial.println("");
    Serial.println("Starting...");

    // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
    loadConfiguration(configFilename, config);
    
    WiFi.softAP(APSSID, APPSK);
    
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.on("/", handleRoot);
    server.on("/QR", QR_DataHandler);
    server.on("/config.json", JSON_Handler);
    server.on("/edit", EditPage_Handler);
    server.begin();
    Serial.println("HTTP server started");
    
    // enable debug qrcode
    qrcode.debug();
    //SPI.begin(TFT_SCLK,TFT_RST,TFT_MOSI,TFT_CS);
    display.init(240,240,SPI_MODE2);
    pinMode(TFT_BLK, OUTPUT);
    digitalWrite(TFT_BLK, HIGH);

    // Initialize QRcode display using library
    qrcode.init();
    // create qrcode
    //qrcode.create("https://www.instructables.com/member/Makertronics/instructables/");
    qrcode.create(config.Default);
}

void loop() { 
  //to server the client conneced to the esp8266 in AP Mode
  server.handleClient();

  /*
  if(Serial.available()){
      String message = Serial.readString();
      Serial.print("Message: ");
      Serial.println(message);
      qrcode.create(message);
    
    }
   */
  }
