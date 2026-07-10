#define PH_SENSOR_PIN 34

int buffer_arr[10], temp;
unsigned long int avgval;

float volt;
float ph_act;

// HASIL KALIBRASI KAMU
float slope = -4.60;
float intercept = 18.37;

void setup() {
  Serial.begin(115200);

  // Setting ADC ESP32 (PENTING)
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  Serial.println("=== MONITORING pH SENSOR ===");
}

void loop() {

  // Ambil 10 data
  for (int i = 0; i < 10; i++) {
    buffer_arr[i] = analogRead(PH_SENSOR_PIN);
    delay(30);
  }

  // Sorting (filter noise)
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buffer_arr[i] > buffer_arr[j]) {
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }

  // Ambil rata-rata tengah
  avgval = 0;
  for (int i = 2; i < 8; i++) {
    avgval += buffer_arr[i];
  }

  float adcValue = avgval / 6.0;

  // Konversi ke volt
  volt = adcValue * 3.3 / 4095.0;

  // Hitung pH
  ph_act = slope * volt + intercept;

  // Tampilkan semua
  Serial.print("ADC   : ");
  Serial.print(adcValue);

  Serial.print(" | Volt: ");
  Serial.print(volt, 3);

  Serial.print(" V | pH: ");
  Serial.println(ph_act, 2);

  delay(3000);
}