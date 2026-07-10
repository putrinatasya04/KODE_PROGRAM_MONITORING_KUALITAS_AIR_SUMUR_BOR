#include <EEPROM.h>
#include "GravityTDS.h"

#define TdsSensorPin 35
GravityTDS gravityTds;

float temperature = 25;
float tdsValue = 0;

// 🔥 HASIL KALIBRASI - dari interpolasi linier 2 titik
// Titik 1: tdsRaw saat larutan 500 ppm  → rata-rata dari Serial Monitor
// Titik 2: tdsRaw saat larutan 1000 ppm → rata-rata dari Serial Monitor
float tdsRaw1 = 485.0;   // ← tdsRaw saat larutan 500 ppm
float tdsRaw2 = 984.0;   // ← tdsRaw saat larutan 1000 ppm
float tds1    = 500.0;
float tds2    = 1000.0;
float a = (tds2 - tds1) / (tdsRaw2 - tdsRaw1);  // otomatis terhitung
float b = tds1 - (a * tdsRaw1);                  // otomatis terhitung

void setup() {
  Serial.begin(115200);

  EEPROM.begin(512);

  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(3.3);
  gravityTds.setAdcRange(4095);
  gravityTds.begin();

  // Cek nilai a dan b di Serial Monitor
  Serial.print("Nilai a = "); Serial.println(a);
  Serial.print("Nilai b = "); Serial.println(b);
}

void loop() {
  int adcValue = analogRead(TdsSensorPin);

  gravityTds.setTemperature(temperature);
  gravityTds.update();

  float tdsRaw = gravityTds.getTdsValue();

  // 🔥 kalibrasi 2 titik (interpolasi linier)
  tdsValue = (a * tdsRaw) + b;
  if (tdsValue < 0) tdsValue = 0;

  Serial.print("ADC: ");
  Serial.print(adcValue);
  Serial.print(" | Raw TDS: ");
  Serial.print(tdsRaw);
  Serial.print(" | TDS Fix: ");
  Serial.print(tdsValue);
  Serial.println(" ppm");

  delay(1000);
}