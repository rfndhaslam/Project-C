#include <HardwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>

// UART untuk UHF RFID Reader
#define RXD2 16
#define TXD2 17
HardwareSerial RFID(2);

// Sensor Proximity Ruangan A
const int sensorDalamA = 27;
const int sensorLuarA = 26;

// Sensor Proximity Ruangan B
const int sensorDalamB = 25;
const int sensorLuarB = 33;

// Status sebelumnya
bool lastStatusDalamA = HIGH;
bool lastStatusLuarA = HIGH;
bool lastStatusDalamB = HIGH;
bool lastStatusLuarB = HIGH;

// WiFi credentials
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

String ruangan = "";

// API endpoints
const char* apiMasukEndpoint = "https://your-api-endpoint.com/api/items/masuk";
const char* apiKeluarEndpoint = "https://your-api-endpoint.com/api/items/keluar";

// Waktu pengukuran
unsigned long waktuDeteksi, waktuPengiriman, waktuTotal;

void setup() {
  pinMode(sensorDalamA, INPUT);
  pinMode(sensorLuarA, INPUT);
  pinMode(sensorDalamB, INPUT);
  pinMode(sensorLuarB, INPUT);

  Serial.begin(9600);
  RFID.begin(57600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Sistem Monitoring Barang Aktif");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  bool currentDalamA = digitalRead(sensorDalamA);
  bool currentLuarA = digitalRead(sensorLuarA);
  bool currentDalamB = digitalRead(sensorDalamB);
  bool currentLuarB = digitalRead(sensorLuarB);

  // Barang Masuk Ruangan A
  if (currentLuarA == LOW && lastStatusLuarA == HIGH) {
    ruangan = "1";
    waktuDeteksi = millis();
    if (bacaTagRFID(apiMasukEndpoint)) {
      waktuPengiriman = millis();
      waktuTotal = waktuPengiriman - waktuDeteksi;
      Serial.println("Barang masuk ke Ruangan A!");
      logWaktu();
    }
  }

  // Barang Keluar Ruangan A
  if (currentDalamA == LOW && lastStatusDalamA == HIGH) {
    ruangan = "1";
    waktuDeteksi = millis();
    if (bacaTagRFID(apiKeluarEndpoint)) {
      waktuPengiriman = millis();
      waktuTotal = waktuPengiriman - waktuDeteksi;
      Serial.println("Barang keluar dari Ruangan A!");
      logWaktu();
    }
  }

  // Barang Masuk Ruangan B
  if (currentLuarB == LOW && lastStatusLuarB == HIGH) {
    ruangan = "2";
    waktuDeteksi = millis();
    if (bacaTagRFID(apiMasukEndpoint)) {
      waktuPengiriman = millis();
      waktuTotal = waktuPengiriman - waktuDeteksi;
      Serial.println("Barang masuk ke Ruangan B!");
      logWaktu();
    }
  }

  // Barang Keluar Ruangan B
  if (currentDalamB == LOW && lastStatusDalamB == HIGH) {
    ruangan = "2";
    waktuDeteksi = millis();
    if (bacaTagRFID(apiKeluarEndpoint)) {
      waktuPengiriman = millis();
      waktuTotal = waktuPengiriman - waktuDeteksi;
      Serial.println("Barang keluar dari Ruangan B!");
      logWaktu();
    }
  }

  lastStatusDalamA = currentDalamA;
  lastStatusLuarA = currentLuarA;
  lastStatusDalamB = currentDalamB;
  lastStatusLuarB = currentLuarB;

  delay(100);
}

// Pembacaan RFID multi-tag unik
bool bacaTagRFID(const char* apiEndpoint) {
  unsigned long startMillis = millis();
  const unsigned long bacaTimeout = 3000;
  const unsigned long jedaTag = 100;

  String detectedTags[10];
  int tagCount = 0;

  while (millis() - startMillis < bacaTimeout) {
    if (RFID.available()) {
      String uid = "";
      while (RFID.available()) {
        uint8_t data = RFID.read();
        if (data != '\r' && data != '\n') {
          if (data < 0x10) uid += "0";
          uid += String(data, HEX);
        }
        delay(2);
      }

      uid.toUpperCase();
      uid.replace(" ", "");

      if (uid.length() >= 8) {
        bool alreadyExists = false;
        for (int i = 0; i < tagCount; i++) {
          if (detectedTags[i] == uid) {
            alreadyExists = true;
            break;
          }
        }

        if (!alreadyExists && tagCount < 10) {
          detectedTags[tagCount++] = uid;
          Serial.print("Tag Baru Terdeteksi: ");
          Serial.println(uid);
        }
      }
    }
    delay(jedaTag);
  }

  if (tagCount > 0) {
    for (int i = 0; i < tagCount; i++) {
      String totalStr = "\"" + String(waktuTotal) + "\"";
      String deteksiStr = "\"" + String(waktuDeteksi) + "\"";
      String pengirimanStr = "\"" + String(waktuPengiriman) + "\"";

      if (!sendToAPI(detectedTags[i], apiEndpoint, ruangan, totalStr, deteksiStr, pengirimanStr)) {
        Serial.println("Gagal kirim tag ke API: " + detectedTags[i]);
      }
    }
    return true;
  }

  Serial.println("Tidak ada tag terdeteksi dalam waktu pembacaan.");
  return false;
}

bool sendToAPI(String uid, const char* apiEndpoint, String Ruangan, String Total, String Deteksi, String Pengiriman) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(apiEndpoint);
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{\"kode_barang\":\"" + uid + 
                         "\", \"ruangan\":\"" + Ruangan + 
                         "\", \"total\":" + Total + 
                         ", \"deteksi\":" + Deteksi + 
                         ", \"pengiriman\":" + Pengiriman + "}";

    Serial.println("Sending JSON payload: " + jsonPayload);

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      Serial.println(httpResponseCode);
      Serial.println(http.getString());
      http.end();
      return true;
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
      Serial.println(http.getString());
      http.end();
      return false;
    }
  } else {
    Serial.println("WiFi Disconnected");
    return false;
  }
}

void logWaktu() {
  Serial.print("Waktu deteksi: ");
  Serial.println(waktuDeteksi);
  Serial.print("Waktu pengiriman: ");
  Serial.println(waktuPengiriman);
  Serial.print("Durasi total (ms): ");
  Serial.println(waktuTotal);
}
