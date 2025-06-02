#include <DHT.h>
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

// Configuration
#define DHTPIN 0     // GPIO 0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
//###########
uint8_t mac09[] = {0x2C, 0xF4, 0x32, 0x8C, 0x09, 0xBF}; // mac of the controller
char Myname[4] = "e02";

//packet structures
struct SensorPacket {
  char name[4];
  float temperature;
  float humidity;
};



void setup() {
  dht.begin();
  WiFi.mode(WIFI_STA);  // ESP-NOW works only in STA or AP mode
  esp_now_init(); // do i really need to explain this to you? dumbass

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER); //controller only sends, slave only recieves, combo cant decide if hes a top or bottom (its both)
  esp_now_add_peer(mac09, ESP_NOW_ROLE_SLAVE, 1, NULL, 0); // adds esp09 as a slave peer, sending on channel 1 of 13, without encription

}

void loop() {
  delay(5000);  // Wait between measurements (DHT11 max rate is 1Hz)
  
  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();  // Celsius
  
  //look how cute :3 were wrapping our data into a neat lil packet (please help im going insane)
  SensorPacket Data;
  strcpy(Data.name, Myname);
  Data.temperature = tempC;
  Data.humidity = humidity;

  esp_now_send(mac09, (uint8_t *)&Data, sizeof(Data)); //(uint8_t *)&Data turns the packet struct into a byte array, and sends it to mac09

}