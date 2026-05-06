// (Kode lengkap dengan logika penyiraman dan pemupukan terpisah untuk ESP32 dan Nano)

#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
//#include <sstream>


#define INFLUXDB_URL "http://your-url"
#define INFLUXDB_TOKEN "your-token"
#define INFLUXDB_ORG "your-org"
#define INFLUXDB_BUCKET "your-bucket"
#define DEVICE "esp32_kebun"

const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";
const char* ap_ssid = "your_ap_ssid";
const char* ap_password = "your_ap_password";

WebServer server(80);
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
Point sensor("sensor_data");

#define HUMIDITY_LOW_THRESHOLD 40
#define HUMIDITY_HIGH_THRESHOLD 80
#define PH_LOW_THRESHOLD 6.0
#define PH_HIGH_THRESHOLD 7.0

#define DHT_PIN 4
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

#define SOIL_MOISTURE_PIN_1 32
#define SOIL_MOISTURE_PIN_2 33
#define SOIL_PH_PIN 34
#define SOIL_PH_ENABLE_PIN 13

#define FLOW_SENSOR_PIN 19
#define RELAY_WATER_PUMP 14
#define RELAY_WATER_VALVE 27
#define RELAY_FERT_PUMP 25 
#define RELAY_FERT_VALVE 26

#define RXD2 16
#define TXD2 17
HardwareSerial RS485Serial(1);

BH1750 lightMeter;
float pH, lastReading;
int soilMoisture[2];
int dryValue = 2545;
int wetValue = 1150;

volatile int pulseCount = 0;
float flowRate = 0.0;
float flowVolume = 0.0;
unsigned long oldTime = 0;

void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

int moisture1, moisture2, moisture3;
float pHnano;
bool isWatering = false;
bool isPHnotOptimal = false;

String wateringMode = "auto";
String fertilizingMode = "auto";

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 1200000;
//const unsigned long sendInterval = 300000;

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>ESP32 Control</title></head><body>";
  html += "<h1>Kontrol Manual Relay</h1>";
  html += "<h2>Penyiraman</h2>";
  html += "<p>Mode Saat Ini: <b>" + wateringMode + "</b></p>";
  html += "<p><a href=\"/watering/on\">Hidupkan Penyiraman</a></p>";
  html += "<p><a href=\"/watering/off\">Matikan Penyiraman</a></p>";
  html += "<p><a href=\"/watering/auto\">Mode Otomatis Penyiraman</a></p>";
  html += "<h2>Pemupukan</h2>";
  html += "<p>Mode Saat Ini: <b>" + fertilizingMode + "</b></p>";
  html += "<p><a href=\"/fertilizing/on\">Hidupkan Pemupukan</a></p>";
  html += "<p><a href=\"/fertilizing/off\">Matikan Pemupukan</a></p>";
  html += "<p><a href=\"/fertilizing/auto\">Mode Otomatis Pemupukan</a></p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleWateringOn() { wateringMode = "on"; server.send(200, "text/plain", "Penyiraman HIDUP"); }
void handleWateringOff() { wateringMode = "off"; server.send(200, "text/plain", "Penyiraman MATI"); }
void handleWateringAuto() { wateringMode = "auto"; server.send(200, "text/plain", "Mode Penyiraman OTOMATIS"); }
void handleFertilizingOn() { fertilizingMode = "on"; server.send(200, "text/plain", "Pemupukan HIDUP"); }
void handleFertilizingOff() { fertilizingMode = "off"; server.send(200, "text/plain", "Pemupukan MATI"); }
void handleFertilizingAuto() { fertilizingMode = "auto"; server.send(200, "text/plain", "Mode Pemupukan OTOMATIS"); }

size_t estimateUploadSize(Point &point) {
  String line = point.toLineProtocol();  // Dapatkan string Line Protocol
  return line.length();                  // Panjang string dalam byte
}


void setup() {
  Serial.begin(115200);
  RS485Serial.begin(9600, SERIAL_8N1, RXD2, TXD2);
  dht.begin();
  delay(5000);
  Wire.begin();
  lightMeter.begin();

  pinMode(DHT_PIN, INPUT_PULLUP);
  pinMode(SOIL_PH_ENABLE_PIN, OUTPUT);
  pinMode(RELAY_WATER_PUMP, OUTPUT);
  pinMode(RELAY_WATER_VALVE, OUTPUT);
  pinMode(RELAY_FERT_PUMP, OUTPUT);
  pinMode(RELAY_FERT_VALVE, OUTPUT);
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);

  digitalWrite(RELAY_WATER_PUMP, LOW);
  digitalWrite(RELAY_WATER_VALVE, LOW);
  digitalWrite(RELAY_FERT_PUMP, LOW);
  digitalWrite(RELAY_FERT_VALVE, LOW);
  
  digitalWrite(SOIL_PH_ENABLE_PIN, HIGH);

  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING);

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  Serial.println("Mencoba konek ke WiFi...");
  Serial.println(WiFi.status());

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nSTA connected. IP: ");
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.status());
  } else {
    Serial.println("\nGagal konek ke WiFi STA.");
  }

  bool ap_result = WiFi.softAP(ap_ssid, ap_password);
  if (ap_result) {
    Serial.print("Access Point aktif. IP: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("Gagal buat Access Point.");
  }

  server.on("/", handleRoot);
  server.on("/watering/on", handleWateringOn);
  server.on("/watering/off", handleWateringOff);
  server.on("/watering/auto", handleWateringAuto);
  server.on("/fertilizing/on", handleFertilizingOn);
  server.on("/fertilizing/off", handleFertilizingOff);
  server.on("/fertilizing/auto", handleFertilizingAuto);
  server.begin();

  sensor.addTag("device", DEVICE);
  
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.validateConnection()) {
      Serial.print("InfluxDB connection failed: ");
      Serial.println(client.getLastErrorMessage());
    } else {
      Serial.println("Connected to InfluxDB");
    }
  } else {
    Serial.println("STA WiFi tidak konek, skip InfluxDB setup.");
  }
}

void bacaDataNano() {
  static String buffer = "";

  while (RS485Serial.available()) {
    char c = RS485Serial.read();
    if (c == '\n') {
      // Parsing setelah newline
      Serial.print("Data Nano: ");
      Serial.println(buffer);

      int idx1 = buffer.indexOf(',');
      int idx2 = buffer.indexOf(',', idx1 + 1);
      int idx3 = buffer.indexOf(',', idx2 + 1);

      if (idx1 > 0 && idx2 > idx1 && idx3 > idx2) {
        moisture1 = buffer.substring(0, idx1).toInt();
        moisture2 = buffer.substring(idx1 + 1, idx2).toInt();
        moisture3 = buffer.substring(idx2 + 1, idx3).toInt();
        pHnano    = buffer.substring(idx3 + 1).toFloat();
      }

      buffer = "";  // Reset buffer setelah parsing
    } else {
      buffer += c;
    }
  }
}

void loop() {
  server.handleClient();

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  float lux = lightMeter.readLightLevel();

  int moistureRaw[2];
  moistureRaw[0] = analogRead(SOIL_MOISTURE_PIN_1);
  moistureRaw[1] = analogRead(SOIL_MOISTURE_PIN_2);
  for (int i = 0; i < 2; i++) {
    soilMoisture[i] = map(moistureRaw[i], dryValue, wetValue, 0, 100);
    soilMoisture[i] = constrain(soilMoisture[i], 0, 100);
  }

  digitalWrite(SOIL_PH_ENABLE_PIN, LOW);
  delay(1000);
  int ADC = analogRead(SOIL_PH_PIN);
  float ADC_scaled = map(ADC, 0, 4095, 0, 1023);
  pH = (-0.0233 * ADC_scaled) + 12.698;
  if (pH != lastReading) {
    lastReading = pH;
  }  

  if (millis() - oldTime >= 1000) {
    oldTime = millis();
    flowRate = pulseCount / 7.5;
    flowVolume += flowRate / 60.0;
    pulseCount = 0;
  }
  
  bacaDataNano();

  Serial.println("=== Data Lokal ===");
  for (int i = 0; i < 2; i++) {
    Serial.print("Kelembaban Tanah "); Serial.print(i+1); Serial.print(": ");
    Serial.print(soilMoisture[i]); Serial.println("%");
  }
  Serial.print("pH Lokal: "); Serial.println(lastReading, 1);
  Serial.print("Suhu: "); Serial.print(t); Serial.println("°C");
  Serial.print("Kelembaban: "); Serial.print(h); Serial.println("%");
  Serial.print("Cahaya: "); Serial.print(lux); Serial.println(" lux");
  Serial.print("ADC: "); Serial.println(ADC);
  Serial.print("Debit (L/Min): "); Serial.print(flowRate, 2); Serial.println(" L/Min");
  Serial.print("Total Volume: "); Serial.print(flowVolume, 2); Serial.println(" L");

  Serial.println("=== Data Nano ===");
  Serial.print("Nano Moisture 1: "); Serial.print(moisture1); Serial.println("%");
  Serial.print("Nano Moisture 2: "); Serial.print(moisture2); Serial.println("%");
  Serial.print("Nano Moisture 3: "); Serial.print(moisture3); Serial.println("%");
  Serial.print("Nano pH: "); Serial.println(pHnano, 1);

  bool isWateringLocal = false;
  //bool isWateringNano = false;
  bool isFertilizingLocal = false;
  //bool isFertilizingNano = false;

  if (wateringMode == "auto") {
    if (soilMoisture[0] < HUMIDITY_LOW_THRESHOLD || soilMoisture[1] < HUMIDITY_LOW_THRESHOLD) {
      isWateringLocal = true;
    } else if (soilMoisture[0] >= HUMIDITY_HIGH_THRESHOLD && soilMoisture[1] >= HUMIDITY_HIGH_THRESHOLD) {
      isWateringLocal = false;
    }

    /*if (moisture1 < HUMIDITY_LOW_THRESHOLD || moisture2 < HUMIDITY_LOW_THRESHOLD || moisture3 < HUMIDITY_LOW_THRESHOLD) {
      isWateringNano = true;
    } else if (moisture1 >= HUMIDITY_HIGH_THRESHOLD && moisture2 >= HUMIDITY_HIGH_THRESHOLD && moisture3 >= HUMIDITY_HIGH_THRESHOLD) {
      isWateringNano = false;
    }*/
  }

  if (fertilizingMode == "auto") {
    isFertilizingLocal = (lastReading < PH_LOW_THRESHOLD); //|| lastReading > PH_HIGH_THRESHOLD);
    //isFertilizingNano = (pHnano < PH_LOW_THRESHOLD || pHnano > PH_HIGH_THRESHOLD);
  }

  if (wateringMode == "auto") {
    isWatering = isWateringLocal; //|| isWateringNano;
    digitalWrite(RELAY_WATER_PUMP, isWatering ? HIGH : LOW);
    digitalWrite(RELAY_WATER_VALVE, isWatering ? HIGH : LOW);
  } else if (wateringMode == "on") {
    digitalWrite(RELAY_WATER_PUMP, HIGH);
    digitalWrite(RELAY_WATER_VALVE, HIGH);
  } else {
    digitalWrite(RELAY_WATER_PUMP, LOW);
    digitalWrite(RELAY_WATER_VALVE, LOW);
  }

  if (fertilizingMode == "auto") {
    isPHnotOptimal = isFertilizingLocal; //|| isFertilizingNano;
    digitalWrite(RELAY_FERT_PUMP, isPHnotOptimal ? HIGH : LOW);
    digitalWrite(RELAY_FERT_VALVE, isPHnotOptimal ? HIGH : LOW);
  } else if (fertilizingMode == "on") {
    digitalWrite(RELAY_FERT_PUMP, HIGH);
    digitalWrite(RELAY_FERT_VALVE, HIGH);
  } else {
    digitalWrite(RELAY_FERT_PUMP, LOW);
    digitalWrite(RELAY_FERT_VALVE, LOW);
  }

  if (millis() - lastSendTime >= sendInterval) {
    sensor.clearFields();
    sensor.addField("temperature", t);
    sensor.addField("humidity", h);
    sensor.addField("soil_moisture_1", soilMoisture[0]);
    sensor.addField("soil_moisture_2", soilMoisture[1]);
    sensor.addField("nano_moisture_1", moisture1);
    sensor.addField("nano_moisture_2", moisture2);
    sensor.addField("nano_moisture_3", moisture3);
    sensor.addField("pH_sensor", lastReading);
    sensor.addField("pH_nano", pHnano);
    sensor.addField("lux", lux);
    sensor.addField("flow_rate", flowRate);
    //sensor.addField("is_watering_local", isWateringLocal);
    //sensor.addField("is_watering_nano", isWateringNano);
    //sensor.addField("is_fertilizing_local", isFertilizingLocal);
    //sensor.addField("is_fertilizing_nano", isFertilizingNano);
    //sensor.addField("is_watering", isWatering);
    //sensor.addField("is_fertilizing", isPHnotOptimal);

    if (WiFi.status() == WL_CONNECTED) {
      size_t uploadBytes = estimateUploadSize(sensor);
      float uploadKB = uploadBytes / 1024.0;
      Serial.print("Ukuran data upload: ");
      Serial.print(uploadKB, 3);
      Serial.println(" KB");
      
      // Tambahkan ke database jika ingin log ukuran upload juga
      sensor.addField("upload_kb", uploadKB); // tambahkan ke InfluxDB juga jika diinginkan

      client.writePoint(sensor);
    }

    lastSendTime = millis();
  }

  delay(5000);
}
