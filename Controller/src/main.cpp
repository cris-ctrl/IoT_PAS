#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <ESP8266WebServer.h>


// network config
char NN[] = "Fiodor";
char PW[] = "dotoievski";
ESP8266WebServer server(80);

struct SensorPacket {
  char name[4];
  float temperature;
  float humidity;
};

SensorPacket recievedData;

void onReceive(uint8_t *mac, uint8_t *data, uint8_t len){
  if(len == sizeof(SensorPacket)){
    memcpy(&recievedData, data, sizeof(SensorPacket));

    Serial.println("Recieved from: ");
    Serial.println(recievedData.name);
    Serial.println("moisty level: ");
    Serial.println(recievedData.humidity);
    Serial.println("Temperature: ");
    Serial.println(recievedData.temperature);

  }else{
    Serial.println("packet size error, trying to print anyway");
    Serial.println(recievedData.humidity);
  }
}

void handleRoot() {
  String page = "<html><head><meta charset='utf-8'><meta http-equiv='refresh' content='2'><title>ESP-SAP</title></head><body>";
  page += "<h1>Sensor Data</h1>";
  page += "<p><b>Device:</b> " + String(recievedData.name) + "</p>";
  page += "<p><b>Humidity:</b> " + String(recievedData.humidity) + "%</p>";
  page += "<p><b>Temperature:</b> " + String(recievedData.temperature) + "°C</p>";
  page += "</body></html>";

  server.send(200, "text/html", page);
}



void setup(){
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(NN, PW);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init failed!");
    return;
  }

  // Registra função para tratar pacotes recebidos
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE); // ESP8266
  esp_now_register_recv_cb(onReceive);
  Serial.println("workingig");


  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web server started!");


}
void loop(){
  server.handleClient();
}


/*

// Serve data on web page


void setup() {
  Serial.begin(9600);

  // Connect to home Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Start ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init failed!");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onReceive);

  // Setup web server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web server started!");
}

void loop() {
  server.handleClient();
}
*/