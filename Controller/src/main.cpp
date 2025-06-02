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

struct SensorPacket {
  char name[4];
  float temperature;
  float humidity;
};
