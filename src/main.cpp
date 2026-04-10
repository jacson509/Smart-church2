#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "SPIFFS.h"
#include <ArduinoJson.h>

// WIFI
const char* ssid = "Triplej509";
const char* password = "12345678";

// SERVER.
WebServer server(80);

// VARIABLES (propres)
float temperature = 0;
int humidity = 0;
bool motion = false;
int light = 0;

// API MOCKAROO
String api_url = "https://my.api.mockaroo.com/jacson.json?key=5883b5e0";

// ===================== API =====================
void updateData() {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, api_url);

  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();

    Serial.println("----- API RESPONSE -----");
    Serial.println(payload);
    Serial.println("------------------------");

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

  if (!error) {

  JsonObject obj = doc.is<JsonArray>() ? doc[0] : doc.as<JsonObject>();

  temperature = obj["temperature"] | obj["Temperature"] | 0;
  humidity    = obj["humidity"]    | obj["Humidity"]    | 0;
  motion      = obj["motion"]      | obj["Motion"]      | false;
  light       = obj["light"]       | obj["Light"]       | 0;

  Serial.println(" DATA OK");
    } else {
      Serial.println("Erreur JSON");
    }
  } else {
    Serial.println("Erreur HTTP");
  }

  http.end();
}

// ===================== API LOCAL =====================
void handleData() {
  String json = "{";
  json += "\"temperature\":" + String(temperature) + ",";
  json += "\"humidity\":" + String(humidity) + ",";
  json += "\"motion\":" + String(motion ? "true" : "false") + ",";
  json += "\"light\":" + String(light);
  json += "}";

  server.send(200, "application/json", json);
}

// ===================== PAGE WEB =====================
void handleRoot() {
  File file = SPIFFS.open("/index.html", "r");
  if (!file) {
    server.send(500, "text/plain", "Erreur fichier HTML");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connexion WiFi...");
  }

  Serial.println("Connecté !");
  Serial.println(WiFi.localIP());

  if (!SPIFFS.begin(true)) {
    Serial.println("Erreur SPIFFS");
    return;
  }

  server.on("/", handleRoot);
  server.on("/data", handleData);

  server.begin();
}

// ===================== LOOP =====================
void loop() {
  server.handleClient();

  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate > 7000) {
    updateData();
    lastUpdate = millis();
  }
}
