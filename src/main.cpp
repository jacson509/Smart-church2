#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include "SPIFFS.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// WIFI
const char* ssid = "Triplej509";
const char* password = "your password";

WebServer server(80);

// ADAFRUIT IO
#define IO_USERNAME  "jacson509"
#define IO_KEY "YOUR_IO_KEY"


// ================= GET DATA =================
String getAdafruitData(String feed){

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;

  String url = "https://io.adafruit.com/api/v2/" + String(IO_USERNAME) + "/feeds/" + feed + "/data/last";

  http.begin(client, url);
  http.addHeader("X-AIO-Key", IO_KEY);

  int httpCode = http.GET();

  Serial.print("HTTP CODE: ");
  Serial.println(httpCode);

  String payload = "{}";

  if(httpCode == 200){
    payload = http.getString();

    Serial.println("==== DATA RECEIVED ====");
    Serial.println(payload);
  } else {
    Serial.println("Erreur Adafruit IO");
  }

  http.end();

  return payload;
}

// ================= PARSE =================
String extractValue(String data){

  int start = data.indexOf("\"value\":\"") + 9;
  int end = data.indexOf("\"", start);

  String value = data.substring(start, end);

  value.trim();

  if(value == "" || value == "null"){
    value = "0";
  }

  return value;
}

// ================= ROUTES =================
void handleRoot(){

  if(!SPIFFS.exists("/index.html")){
    server.send(404, "text/plain", "Fichier HTML non trouvé");
    return;
  }

  File file = SPIFFS.open("/index.html", "r");
  server.streamFile(file, "text/html");
  file.close();
}

// 🌡️
void handleTemperature(){

  String data = getAdafruitData("temperature");
  String value = extractValue(data);

  server.send(200, "application/json",
    "{\"value\":" + value + "}");
}

// 💧
void handleHumidity(){

  String data = getAdafruitData("humidity");
  String value = extractValue(data);

  server.send(200, "application/json",
    "{\"value\":" + value + "}");
}

// 🚶
void handleMotion(){

  String data = getAdafruitData("motion");
  String value = extractValue(data);

  server.send(200, "application/json",
    "{\"value\":" + value + "}");
}

// 💡
void handleLight(){

  String data = getAdafruitData("light");
  String value = extractValue(data);

  server.send(200, "application/json",
    "{\"value\":" + value + "}");
}

// ================= SETUP =================
void setup(){

  Serial.begin(115200);

  // SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("Erreur SPIFFS");
    return;
  }

  // WIFI
  WiFi.begin(ssid,password);
  Serial.print("Connexion...");

  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnecté !");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // ROUTES
  server.on("/", handleRoot);
  server.on("/temperature", handleTemperature);
  server.on("/humidity", handleHumidity);
  server.on("/motion", handleMotion);
  server.on("/light", handleLight);

  server.begin();

  // OTA
  ArduinoOTA.setHostname("SmartChurchESP32");
  ArduinoOTA.begin();

  Serial.println("OTA prêt !");
}

// ================= LOOP =================
void loop(){
  server.handleClient();
  ArduinoOTA.handle();
}