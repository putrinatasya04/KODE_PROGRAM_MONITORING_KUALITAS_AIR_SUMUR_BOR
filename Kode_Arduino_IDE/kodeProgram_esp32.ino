// ================================================
//  MONITORING KUALITAS AIR SUMUR BOR
//  Putri Natasya Lisandra (2240304010)
// ================================================

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <EEPROM.h>
#include "GravityTDS.h"

// ================= WIFI =================
#define WIFI_SSID       "Galaxy A06 2802"
#define WIFI_PASSWORD   "putri1809"

// ================= FIREBASE =================
#define API_KEY         "AIzaSyBjrGnsdNgZYsW9G3wzxS7jdeIO0BDL52Q"
#define DATABASE_URL    "https://monitoringairsumurbor-default-rtdb.firebaseio.com/"
#define DATABASE_SECRET "ISI_SECRET_KEY_DISINI"

FirebaseData   fbdo;
FirebaseAuth   auth;
FirebaseConfig config;

// ================= PIN SENSOR =================
#define PH_PIN   34
#define NTU_PIN  32
#define TDS_PIN  35
// TCS3200
#define S0      4
#define S1      5
#define S2      18
#define S3      19
#define OUT_PIN 21

// ================= KALIBRASI pH =================
// Dari file kalibrasi: buffer 6.86, slope=-4.0, intercept=19.286 (R²=0.989)
#define PH_SLOPE     -4.0
#define PH_INTERCEPT 19.286

// ================= KALIBRASI TDS =================
// Dari file kalibrasi: slope=225.71, intercept=14.33 (R²=0.952)
GravityTDS gravityTds;
#define TDS_SLOPE     225.71
#define TDS_INTERCEPT 14.33

// ================= VARIABEL =================
float phValue, ntuValue, tdsValue, warnaValue;
String kondisiAir, statusAir;

// ================= TIMER =================
unsigned long sendDataPrevMillis = 0;
const unsigned long interval = 10000;
// const unsigned long interval = 5400000; // 1.5 jam 
// ================================================
float bacaPH() {
  int buf[10], tmp;
  unsigned long avg = 0;

  for (int i = 0; i < 10; i++) {
    buf[i] = analogRead(PH_PIN);
    delay(30);
  }
  // Bubble sort
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buf[i] > buf[j]) {
        tmp = buf[i]; buf[i] = buf[j]; buf[j] = tmp;
      }
    }
  }
  for (int i = 2; i < 8; i++) avg += buf[i];
  float adcVal = avg / 6.0;
  float volt   = adcVal * 3.3 / 4095.0;
  float ph     = PH_SLOPE * volt + PH_INTERCEPT;

  Serial.println("  pH ADC=" + String(adcVal,1) +
                 " Volt=" + String(volt,4) +
                 " pH=" + String(ph,2));
  if (ph < 0)  ph = 0;
  if (ph > 14) ph = 14;
  return ph;
}
//  FUNGSI BACA NTU
// ================================================
float bacaNTU() {
  long total = 0;
  for (int i = 0; i < 10; i++) {
    total += analogRead(NTU_PIN);
    delay(20);
  }
  float adcVal = total / 10.0;
  float volt   = adcVal * 3.3 / 4095.0;
  float ntu    = (-73.53 * volt) + 164.0;

  Serial.println("  NTU ADC=" + String(adcVal,1) +
                 " Volt=" + String(volt,4) +
                 " NTU=" + String(ntu,2));

  if (ntu < 0) ntu = 0;
  return ntu;
}
// ================================================
//  FUNGSI BACA TCS3200
long readColorSamples(bool s2, bool s3) {
  long total = 0;
  digitalWrite(S2, s2);
  digitalWrite(S3, s3);
  delay(50);
  for (int i = 0; i < 15; i++) {
    total += pulseIn(OUT_PIN, LOW);
  }
  return total / 15;
}

float bacaTCU() {
  long r = readColorSamples(LOW,  LOW);
  long g = readColorSamples(HIGH, HIGH);
  long b = readColorSamples(LOW,  HIGH);

  Serial.println("  TCU R=" + String(r) +
                 " G=" + String(g) +
                 " B=" + String(b));

  float total = r + g + b;
  if (total == 0) return 0;

  // Normalisasi
  float rn = r / total;
  float gn = g / total;
  float bn = b / total;

  // Semakin merah/kuning → TCU naik
  float tcu = 0;
  if (rn > 0.45) {
    // Air kemerahan/kuning → TCU tinggi
    tcu = rn * 100.0;
    if (tcu > 50) tcu = 50;
  } else {
    // Air jernih/hijau → TCU rendah
    tcu = (1.0 - gn) * 30.0;
    if (tcu < 0) tcu = 0;
  }
  return tcu;
}
// ================================================
//  SETUP
// ================================================
void setup() {
  Serial.begin(115200);

  // ADC per pin
  analogReadResolution(12);
  analogSetPinAttenuation(PH_PIN,  ADC_11db);
  analogSetPinAttenuation(NTU_PIN, ADC_11db);

  // TCS3200
  pinMode(S0, OUTPUT); pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT); pinMode(S3, OUTPUT);
  pinMode(OUT_PIN, INPUT);
  digitalWrite(S0, HIGH); digitalWrite(S1, LOW);

  // GravityTDS
  EEPROM.begin(512);
  gravityTds.setPin(TDS_PIN);
  gravityTds.setAref(3.3);
  gravityTds.setAdcRange(4095);
  gravityTds.begin();

  // WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Menghubungkan WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("."); delay(500);
  }
  Serial.println("\nWiFi Terhubung! IP: " + WiFi.localIP().toString());

  // Firebase
  config.api_key      = API_KEY;
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase Siap!");
}

void loop() {
  unsigned long now = millis();

  if (Firebase.ready() && (now - sendDataPrevMillis >= interval)) {
    sendDataPrevMillis = now;
    Serial.println("\n=== BACA & KIRIM DATA ===");

    // ===== BACA SEMUA SENSOR =====
    phValue    = bacaPH();
    ntuValue   = bacaNTU();
    warnaValue = bacaTCU();

    gravityTds.setTemperature(25);
    gravityTds.update();
    float tdsRaw = gravityTds.getTdsValue();
    tdsValue = (TDS_SLOPE * tdsRaw / 1000.0) + TDS_INTERCEPT;
    
    tdsValue = tdsRaw;
    if (tdsValue < 0)    tdsValue = 0;
    if (tdsValue > 1000) tdsValue = 1000;

    Serial.println(" TDS raw=" + String(tdsRaw,2));

    // ===== RULE-BASED KONDISI (sesuai tabel logika) =====
    // P=0: pH 6.5-8.5, P=1: pH <6.5 atau >8.5
    // K=0: NTU ≤5,     K=1: NTU >5
    // T=0: TDS ≤500,   T=1: TDS >500
    // W=0: Warna ≤15,  W=1: Warna >15

    bool P = !(phValue >= 6.5 && phValue <= 8.5);  // P=1 jika pH diluar normal
    bool K = (ntuValue > 5);
    bool T = (tdsValue > 500);
    bool W = (warnaValue > 15);

    if (!P && !K && !T && !W) {
      kondisiAir = "LAYAK";               // Rule 0: 0000

    } else if (!P && !K && !T && W)  {
      kondisiAir = "CUKUP LAYAK";         // Rule 1: 0001

    } else if (!P && !K && T  && !W) {
      kondisiAir = "CUKUP LAYAK";         // Rule 2: 0010

    } else if (!P && !K && T  && W)  {
      kondisiAir = "CUKUP LAYAK";         // Rule 3: 0011

    } else {
      kondisiAir = "TIDAK LAYAK";         // Rule 4-15: semua lainnya
    }
    // ===== STATUS =====
    if (kondisiAir == "LAYAK")
      statusAir = "Air memenuhi standar kualitas";
    else if (kondisiAir == "CUKUP LAYAK")
      statusAir = "Air masih dapat digunakan dengan perhatian";
    else
      statusAir = "Air tidak memenuhi standar kualitas";
    // ===== KIRIM DATA REALTIME =====
    bool ok = true;
    ok &= Firebase.RTDB.setFloat(&fbdo,  "sensor/ph",      phValue);
    ok &= Firebase.RTDB.setFloat(&fbdo,  "sensor/ntu",     ntuValue);
    ok &= Firebase.RTDB.setFloat(&fbdo,  "sensor/tds",     tdsValue);
    ok &= Firebase.RTDB.setFloat(&fbdo,  "sensor/warna",   warnaValue);
    ok &= Firebase.RTDB.setString(&fbdo, "sensor/kondisi", kondisiAir);
    ok &= Firebase.RTDB.setString(&fbdo, "sensor/status",  statusAir);

    // ===== HISTORI EXCEL =====
    String path = "sensorData/" + String(millis());
    Firebase.RTDB.setFloat(&fbdo,  path + "/ph",      phValue);
    Firebase.RTDB.setFloat(&fbdo,  path + "/ntu",     ntuValue);
    Firebase.RTDB.setFloat(&fbdo,  path + "/tds",     tdsValue);
    Firebase.RTDB.setFloat(&fbdo,  path + "/warna",   warnaValue);
    Firebase.RTDB.setString(&fbdo, path + "/kondisi", kondisiAir);
    Firebase.RTDB.setString(&fbdo, path + "/status",  statusAir);
    Firebase.RTDB.setString(&fbdo, path + "/waktu",   String(millis()));

    // ===== DEBUG =====
    Serial.println("pH      : " + String(phValue, 2));
    Serial.println("NTU     : " + String(ntuValue, 2));
    Serial.println("TDS     : " + String(tdsValue, 2) + " ppm");
    Serial.println("TCU     : " + String(warnaValue, 2));
    Serial.println("P=" + String(P) + " K=" + String(K) +
                   " T=" + String(T) + " W=" + String(W));
    Serial.println("Kondisi : " + kondisiAir);
    Serial.println("Status  : " + statusAir);

    if (ok) Serial.println(">> BERHASIL KIRIM ✅");
    else    Serial.println(">> GAGAL ❌ : " + fbdo.errorReason());
    Serial.println("==========================");
  }
}