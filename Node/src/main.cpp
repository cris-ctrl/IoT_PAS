#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <espnow.h>
#include <DHT.h>

// ============== CONFIGURATION ==============
const char* selfName = "2C"; 
uint8_t controllerMAC[] = {0x2C, 0xF4, 0x32, 0x8C, 0x09, 0xBF}; // ESP-09's MAC
uint8_t nextNodeMAC[] = {}; // MAC of next device in chain

#define DHTPIN 2     // GPIO2 for DHT11
#define DHTTYPE DHT11
// ===========================================

DHT dht(DHTPIN, DHTTYPE);

struct ControlPacket {
  char target[4];
  bool led;
};

struct AckPacket {
  char name[4];
};

struct SensorPacket {
  char name[4];
  float temperature;
  float humidity;
};

unsigned long lastAck = 0;
const long ackInterval = 2000;
unsigned long lastSensorRead = 0;
const long sensorInterval = 5000;

void sendAck() {
  AckPacket ack;
  strncpy(ack.name, selfName, 3);
  ack.name[3] = '\0';
  esp_now_send(controllerMAC, (uint_t *)&ack, sizeof(ack));
}

void sendSensorData() {
  SensorPacket sensorData;
  strncpy(sensorData.name, selfName, 3);
  sensorData.name[3] = '\0';
  
  sensorData.temperature = dht.readTemperature();
  sensorData.humidity = dht.readHumidity();
  
  if (!isnan(sensorData.temperature) && !isnan(sensorData.humidity)) {
    esp_now_send(controllerMAC, (uint8_t *)&sensorData, sizeof(sensorData));
  }
}

void onReceive(uint8_t *mac, uint8_t *data, uint8_t len) {
  if (len == sizeof(ControlPacket)) {
    ControlPacket* pkt = (ControlPacket*)data;
    
    // Handle local command
    if (strcmp(pkt->target, selfName) == 0) {
      digitalWrite(LED_BUILTIN, pkt->led ? LOW : HIGH);
    }
    // Forward if we have a next node and packet isn't for us
    else if (sizeof(nextNodeMAC) == 6) {
      esp_now_send(nextNodeMAC, data, len);
    }
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  dht.begin();
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0) return;
  
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  
  // Always connect to controller
  esp_now_add_peer(controllerMAC, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  
  // Only connect to next node if MAC is specified
  if (sizeof(nextNodeMAC) == 6) {
    esp_now_add_peer(nextNodeMAC, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  }

  esp_now_register_recv_cb(onReceive);
}

void loop() {
  if (millis() - lastAck >= ackInterval) {
    sendAck();
    lastAck = millis();
  }
  
  if (millis() - lastSensorRead >= sensorInterval) {
    sendSensorData();
    lastSensorRead = millis();
  }
}