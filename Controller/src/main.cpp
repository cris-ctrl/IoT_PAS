#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

// Peer MAC addresses
uint8_t macA4[] = {0xA4, 0xE5, 0x7C, 0xBB, 0xE9, 0xFC};
uint8_t mac2C[] = {0x2C, 0xF4, 0x32, 0x8C, 0x09, 0xBF};  // Corrected to match node's controller MAC

// network config
char NN[] = "Fiodor";
char PW[] = "dotoievski";

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

void setup(){
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init failed!");
    return;
  }

  // Registra função para tratar pacotes recebidos
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE); // ESP8266
  esp_now_register_recv_cb(onReceive);
  Serial.println("workingig");


}
void loop(){

}
