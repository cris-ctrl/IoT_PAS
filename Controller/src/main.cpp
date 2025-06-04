#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <ESP8266WebServer.h>
#include <FS.h>  // Include SPIFFS library

// Network config
char NN[] = "Fiodor";
char PW[] = "dotoievski";
ESP8266WebServer server(80);

struct SensorPacket {
  char name[4];
  float temperature;
  float humidity;
};

SensorPacket recievedData;

void onReceive(uint8_t *mac, uint8_t *data, uint8_t len) {
  if (len == sizeof(SensorPacket)) {
    memcpy(&recievedData, data, sizeof(SensorPacket));
    Serial.println("Recebido de: " + String(recievedData.name));
    Serial.println("Humidade: " + String(recievedData.humidity) + "%");
    Serial.println("Temperatura: " + String(recievedData.temperature) + "°C");
  } else {
    Serial.println("Erro de tamanho de pacote");
  }
}

void handleRoot() {
  File file = SPIFFS.open("/dashboard.html", "r");
  if (!file) {
    server.send(500, "text/plain", "Erro carregando arquivo");
    return;
  }
  
  String html = file.readString();
  file.close();
  
  // Replace placeholders with live data
  html.replace("{DEVICE}", String(recievedData.name));
  html.replace("{HUMIDITY}", String(recievedData.humidity));
  html.replace("{TEMP}", String(recievedData.temperature));
  
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(9600);
  
  // Initialize SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS falhou!");
    while(1) yield(); // Halt if filesystem fails
  }

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(NN, PW);
  
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConectado! IP: " + WiFi.localIP().toString());

  // Initialize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init falhou!");
    return;
  }
  
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onReceive);

  // Configure web server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web server iniciou");
}

void loop() {
  server.handleClient();
}