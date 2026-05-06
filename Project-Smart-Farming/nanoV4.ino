#define SOIL_MOISTURE_PIN_1 A0
#define SOIL_MOISTURE_PIN_2 A1
#define SOIL_MOISTURE_PIN_3 A4

#define SOIL_PH_PIN A2
#define SOIL_PH_ENABLE_PIN 7

#define HUMIDITY_THRESHOLD 40
#define PH_LOW_THRESHOLD 6.5
#define PH_HIGH_THRESHOLD 7.0

float pH, lastReading;
int soilMoisture[3];

// Hasil kalibrasi kelembaban tanah (Arduino Nano)
int dryValue = 478;
int wetValue = 233;

void setup() {
  Serial.begin(9600);

  pinMode(SOIL_PH_ENABLE_PIN, OUTPUT);
  digitalWrite(SOIL_PH_ENABLE_PIN, HIGH);  // Default OFF
}

void loop() {
  // Baca Soil Moisture Sensor
  int moistureRaw[3];
  moistureRaw[0] = analogRead(SOIL_MOISTURE_PIN_1);
  moistureRaw[1] = analogRead(SOIL_MOISTURE_PIN_2);
  moistureRaw[2] = analogRead(SOIL_MOISTURE_PIN_3);

  for (int i = 0; i < 3; i++) {
    // Mapping manual biar lebih aman
    soilMoisture[i] = (dryValue - moistureRaw[i]) * 100 / (dryValue - wetValue);
    soilMoisture[i] = constrain(soilMoisture[i], 0, 100);

    Serial.print("Kelembaban Tanah ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(soilMoisture[i]);
    Serial.println("%");
  }

  // Baca Soil pH Sensor
  digitalWrite(SOIL_PH_ENABLE_PIN, LOW);  // Enable sensor
  delay(1000);  // Tunggu pembacaan stabil

  int adc = analogRead(SOIL_PH_PIN);
  digitalWrite(SOIL_PH_ENABLE_PIN, HIGH);  // Disable sensor

  pH  = (-0.0333 * adc) + 9.8340;
  if (pH != lastReading) lastReading = pH;

  Serial.print("pH Tanah: ");
  Serial.println(lastReading, 1); // 2 angka di belakang koma

  // Cek kondisi pH
  if (pH < PH_LOW_THRESHOLD) {
    Serial.println("⚠ pH terlalu rendah. Perlu pemupukan.");
  } else if (pH > PH_HIGH_THRESHOLD) {
    Serial.println("⚠ pH terlalu tinggi. Perlu penanganan.");
  } else {
    Serial.println("✅ pH Tanah optimal.");
  }

  Serial.println("----------------------------");
  delay(5000);
}