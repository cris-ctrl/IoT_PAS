#include <DHT.h>

// Configuration
#define DHTPIN 2     // GPIO2 on ESP-01 (physical pin 5)
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9800);
  dht.begin();
  Serial.println("\nESP-01 DHT11 Sensor Test");
}

void loop() {
  delay(2000);  // Wait between measurements (DHT11 max rate is 1Hz)
  
  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();  // Celsius
  
  // Check if readings failed
  if (isnan(humidity) || isnan(tempC)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Print results
  Serial.print("Temperature: ");
  Serial.print(tempC);
  Serial.print("°C\t");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
}