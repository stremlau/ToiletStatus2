#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <ArduinoJson.h>

#include <SPI.h>
#include "RF24.h"

bool mdns_started = false;

ESP8266WebServer server(80);
MDNSResponder mdns;
RF24 radio(4, 15);

byte address[6] = {"CjTS2"}; // Campusjaeger Toilet-Sensor 2

byte locked = false;

String ssid;
String pass;
String host;

void readSettings() {
  // TODO close file
  ssid = SPIFFS.open("ssid", "r").readString();
  pass = SPIFFS.open("pass", "r").readString();
  host = SPIFFS.open("host", "r").readString();

  Serial.println("Einstellungen:");
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(host);
}

void writeToFile(String path, String s) {
  File f = SPIFFS.open(path, "w");
  f.print(s);
  f.close();
}

void writeSettings() {
  writeToFile("ssid", ssid);
  writeToFile("pass", pass);
  writeToFile("host", host);

  WiFi.begin (ssid.c_str(), pass.c_str());
}

String ipToString(IPAddress addr) {
  char ip[24];
  sprintf(ip, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
  return String(ip);
}

void handleSettings() {
  if (server.hasArg("ssid") && server.hasArg("pass") && server.hasArg("host"))  {
    ssid = server.arg("ssid");
    pass = server.arg("pass");
    host = server.arg("host");

    writeSettings();
    server.send(200, "text/html", "OK");
  }
  else {
    String content;
    
    DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(5));
    JsonObject& root = jsonBuffer.createObject();
    root["ssid"] = ssid;
    root["pass"] = pass;
    root["host"] = host;
    root.printTo(content);

    server.send(200, "application/json", content);
  }
}

void handleInfo() {
  // TODO use JsonBuffer
  String str = "{";
  str += "\"ip_ap\":\"" + ipToString(WiFi.softAPIP()) + "\",";
  str += "\"ip_local\":\"" + ipToString(WiFi.localIP()) + "\",";
  str += "\"status\":\"" + String(WiFi.status()) + "\",";
  str += "\"channel\":\"" + String(WiFi.channel()) + "\",";
  str += "\"signal\":\"" + String(WiFi.RSSI()) + "\"";
  str += "}";
  server.send(200, "application/json", str);
}

void handleStatus() {
  server.send(200, "text/html", String(locked));
}

void handleRoot() {
  File f = SPIFFS.open("/index.html", "r");
  server.send(200, "text/html", f.readString());

  // disable config wifi
  if ( WiFi.status() == WL_CONNECTED ) {
    WiFi.mode(WIFI_STA);
  }
}

void handleNotFound() {
  File f = SPIFFS.open(server.uri(), "r");
  if (!f) {
    server.send(404, "text/html", "Not found. Upload Sketch data folder.");
    return;
  }

  String dataType = "text/html";

  if(server.uri().endsWith(".png")) dataType = "image/png";
  else if(server.uri().endsWith(".js")) dataType = "application/javascript";
  
  server.send(200, dataType, f.readString());
}

void setup() {
  delay(1000);
  Serial.begin(115200);

  pinMode(2, INPUT);

  SPIFFS.begin();
  readSettings();
  
  WiFi.mode(WIFI_AP_STA);

  WiFi.begin (ssid.c_str(), pass.c_str());
  Serial.println ( "" );

  Serial.println();
  Serial.print("Configuring access point...");
  WiFi.softAP("configMe");

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.on("/settings", handleSettings);
  server.on("/info", handleInfo);
  server.on("/status", handleStatus);

  server.serveStatic("/", SPIFFS, "/","max-age=86400");
  
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

  radio.begin();

  radio.setPALevel(RF24_PA_LOW);
  
  radio.openReadingPipe(1, address);
  radio.startListening();
}

void loop() {
  if ( WiFi.status() != WL_CONNECTED ) {
    mdns_started = false;
  }
  else {
    if (!mdns_started) {
      if (host.length() > 0) {
        mdns_started = mdns.begin (host.c_str());
        if (mdns_started) Serial.println("mDNS started");
      }
    }
    else {
      mdns.update();
    }
  }

  byte msg;
  
  if (radio.available()) {
    while (radio.available()) {
      radio.read(&msg, sizeof(byte));
    }

    locked = msg % 2;
    byte nodeId = msg / 2;

    Serial.print("Node ");
    Serial.print(nodeId);
    Serial.print(": ");
    Serial.println(locked);
  }
  
  server.handleClient();
}
