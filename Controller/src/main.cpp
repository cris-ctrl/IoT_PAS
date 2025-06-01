#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <FS.h>

// Peer MAC addresses
uint8_t macA4[] = {0xA4, 0xE5, 0x7C, 0xBB, 0xE9, 0xFC};
uint8_t mac2C[] = {0x2C, 0xF4, 0x32, 0x8C, 0x09, 0xBF};  // Corrected to match node's controller MAC

// network config
char NN[] = "Fiodor";
char PW[] = "dotoievski";

WiFiServer server(80);

// Packet structures
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

// Last received ACK timestamps
unsigned long lastA4Ack = 0;
unsigned long last2CAck = 0;

// Sensor data
float temp2C = 0.0;
float hum2C = 0.0;
unsigned long lastSensor2C = 0;

// Debug print function for MAC addresses
void printMac(const char* label, const uint8_t* mac) {
  Serial.printf("%s: %02X:%02X:%02X:%02X:%02X:%02X\n", 
                label, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void sendToggle(String target, bool state) {
  ControlPacket pkt;
  strncpy(pkt.target, target.c_str(), 3);
  pkt.target[3] = '\0';
  pkt.led = state;
  
  Serial.printf("[SEND] Target: %s, State: %d\n", pkt.target, pkt.led);
  esp_now_send(target == "A4" ? macA4 : mac2C, (uint8_t *)&pkt, sizeof(pkt));
}

void onReceive(uint8_t *mac, uint8_t *data, uint8_t len) {
  // Prevent buffer overflow
  if (len > 50) {
    Serial.println("[ERROR] Packet too large, ignoring");
    return;
  }
  
  Serial.printf("\n[RECV] MAC: %02X:%02X:%02X:%02X:%02X:%02X | Len: %d\n", 
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], len);
  
  if (len == sizeof(AckPacket)) {
    AckPacket* ack = (AckPacket*)data;
    Serial.printf("[ACK] From: %s\n", ack->name);
    
    if (strcmp(ack->name, "A4") == 0) lastA4Ack = millis();
    else if (strcmp(ack->name, "2C") == 0) last2CAck = millis();
  }
  else if (len == sizeof(SensorPacket)) {
    SensorPacket* sensor = (SensorPacket*)data;
    
    // Convert floats to strings for safe printing
    char tempStr[8];
    char humStr[8];
    dtostrf(sensor->temperature, 5, 1, tempStr);
    dtostrf(sensor->humidity, 5, 1, humStr);
    
    Serial.printf("[SENSOR] From: %s, Temp: %s C, Hum: %s %%\n", 
                  sensor->name, tempStr, humStr);
    
    if (strcmp(sensor->name, "2C") == 0) {
      temp2C = sensor->temperature;
      hum2C = sensor->humidity;
      lastSensor2C = millis();
    }
  }
}

void handleClient(WiFiClient client) {
  String request = client.readStringUntil('\r');
  client.flush();

  // Handle API endpoints
  if (request.indexOf("/toggle") >= 0) {
    String tgt = request.indexOf("2C") >= 0 ? "2C" : "A4";
    bool state = request.indexOf("state=1") >= 0;
    sendToggle(tgt, state);
    
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close\r\n");
    client.println("OK");
    return;
  }
  else if (request.indexOf("/status") >= 0) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close\r\n");
    client.printf("{\"A4\":%lu,\"2C\":%lu}", 
                 millis() - lastA4Ack, millis() - last2CAck);
    return;
  }
  else if (request.indexOf("/sensor") >= 0) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close\r\n");
    client.printf("{\"temp\":%.1f,\"hum\":%.1f}", temp2C, hum2C);
    return;
  }

  // Serve HTML fallback if file missing
  File file = SPIFFS.open("/index.html", "r");
  if (!file) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close\r\n");
    client.println("<html><body><h1>ESP Controller Running</h1><p>SPIFFS not mounted</p></body></html>");
    return; 
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close\r\n");
  
  while(file.available()) {
    client.write(file.read());
  }
  file.close();
}

void setup() {
  Serial.begin(9600);
  Serial.println("\n\n[BOOT] Starting controller...");

  // Initialize SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("[SPIFFS] Failed to mount file system");
  } else {
    Serial.println("[SPIFFS] File system mounted");
  }

  // Get controller's MAC address
  uint8_t controllerMac[6];
  WiFi.macAddress(controllerMac);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(NN, PW);
  
  Serial.print("[WiFi] Connecting");
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - wifiStart > 30000) {
      Serial.println("\n[WiFi] Failed to connect - using ESP-NOW only");
      break;
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
  }

  // Print MAC addresses
  printMac("[MAC] Controller", controllerMac);
  printMac("[PEER] A4", macA4);
  printMac("[PEER] 2C", mac2C);

  // Initialize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("[ESP-NOW] Init failed! Restarting...");
    ESP.restart();
    delay(1000);
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  
  // Add peers with success checks
  if (esp_now_add_peer(macA4, ESP_NOW_ROLE_COMBO, 1, NULL, 0) == 0) {
    Serial.println("[PEER] A4 added successfully");
  } else {
    Serial.println("[PEER] Failed to add A4");
  }
  
  if (esp_now_add_peer(mac2C, ESP_NOW_ROLE_COMBO, 1, NULL, 0) == 0) {
    Serial.println("[PEER] 2C added successfully");
  } else {
    Serial.println("[PEER] Failed to add 2C");
  }

  esp_now_register_recv_cb(onReceive);
  server.begin();
  Serial.println("[HTTP] Server started");
}

void loop() {
  static unsigned long lastStatus = 0;
  
  // Periodic status updates with dtostrf conversions
  if (millis() - lastStatus > 5000) {
    char tempStr[8];
    char humStr[8];
    dtostrf(temp2C, 5, 1, tempStr);
    dtostrf(hum2C, 5, 1, humStr);
    
    Serial.printf("[STATUS] A4: %lums ago | 2C: %lums ago | Temp: %s C | Hum: %s %%\n", 
                 millis() - lastA4Ack, millis() - last2CAck, tempStr, humStr);
    lastStatus = millis();
  }
  
  WiFiClient client = server.available();
  if (client) handleClient(client);
}