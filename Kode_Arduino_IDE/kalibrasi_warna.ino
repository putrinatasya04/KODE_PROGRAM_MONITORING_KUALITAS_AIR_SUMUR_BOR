#define S0 4
#define S1 5
#define S2 18
#define S3 19
#define OUT 21

#define SAMPLE 15  // biar stabil

// ================= FUNGSI =================
int readColor(bool s2, bool s3) {
  long total = 0;

  digitalWrite(S2, s2);
  digitalWrite(S3, s3);
  delay(50);

  for (int i = 0; i < SAMPLE; i++) {
    total += pulseIn(OUT, LOW);
  }

  return total / SAMPLE;
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT, INPUT);

  // scaling 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  Serial.println("=== KALIBRASI SENSOR RGB ===");
}

// ================= LOOP =================
void loop() {

  int r = readColor(LOW, LOW);
  int g = readColor(HIGH, HIGH);
  int b = readColor(LOW, HIGH);

  // tentukan warna dominan
  String warna;

  if (r < g && r < b) {
    warna = "MERAH";
  } else if (g < r && g < b) {
    warna = "HIJAU";
  } else if (b < r && b < g) {
    warna = "BIRU";
  } else {
    warna = "TIDAK STABIL";
  }

  Serial.println("\n===== DATA SENSOR =====");
  Serial.print("R : "); Serial.println(r);
  Serial.print("G : "); Serial.println(g);
  Serial.print("B : "); Serial.println(b);
  Serial.print("Warna Dominan : "); Serial.println(warna);
  Serial.println("=======================");

  delay(2000);
}