#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Servo.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"

// WiFi credentials
const char* ssid = "nama_wifi";
const char* password = "password_wifi";

// Pin configuration
const int servoPin = 2;
const int pirPin = 13;
const int buzzerPin = 12;
Servo myservo;

// MQTT settings
const char* mqttServer = "mqtt_server_address";
const int mqttPort = 1883;
const char* mqttUser = "mqtt_username";
const char* mqttPassword = "mqtt_password";
const char* mqttTopic = "motion_detection";

// Telegram settings
const char* telegramBotToken = "YOUR_TELEGRAM_BOT_TOKEN";
const char* telegramChatId = "YOUR_TELEGRAM_CHAT_ID";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

volatile bool motionDetected = false;

AsyncWebServer server(80);

void startCameraServer();

void setup() {
  Serial.begin(115200);
  pinMode(pirPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected");

  myservo.attach(servoPin);

  // MQTT setup
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);

  // Camera setup
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA; // SVGA (800x600)
  config.jpeg_quality = 10; // Compression quality (0-63)
  config.fb_count = 2;

  esp_err_t res = esp_camera_init(&config);
  if (res != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", res);
    return;
  }

  // Web server setup
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", "<html><body><h1>ESP32-CAM Control</h1><a href='/stream'>View Stream</a></body></html>");
  });

  server.on("/rotate", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("angle")) {
      int angle = request->getParam("angle")->value().toInt();
      myservo.write(angle);
      request->send(200, "text/plain", "Servo rotated");
    } else {
      request->send(400, "text/plain", "Missing parameters");
    }
  });

  server.on("/camera_settings", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("resolution") && request->hasParam("quality") && request->hasParam("brightness") && request->hasParam("contrast") && request->hasParam("saturation")) {
      String resolution = request->getParam("resolution")->value();
      int quality = request->getParam("quality")->value().toInt();
      int brightness = request->getParam("brightness")->value().toInt();
      int contrast = request->getParam("contrast")->value().toInt();
      int saturation = request->getParam("saturation")->value().toInt();

      sensor_t *s = esp_camera_sensor_get();
      if (s != NULL) {
        // Apply camera settings
        if (resolution == "QVGA") s->set_framesize(s, FRAMESIZE_QVGA);
        else if (resolution == "VGA") s->set_framesize(s, FRAMESIZE_VGA);
        else if (resolution == "SVGA") s->set_framesize(s, FRAMESIZE_SVGA);
        else if (resolution == "XGA") s->set_framesize(s, FRAMESIZE_XGA);
        else if (resolution == "SXGA") s->set_framesize(s, FRAMESIZE_SXGA);
        else if (resolution == "UXGA") s->set_framesize(s, FRAMESIZE_UXGA);

        s->set_quality(s, quality);
        s->set_brightness(s, brightness);
        s->set_contrast(s, contrast);
        s->set_saturation(s, saturation);

        request->send(200, "text/plain", "Camera settings updated");
      } else {
        request->send(500, "text/plain", "Failed to get sensor");
      }
    } else {
      request->send(400, "text/plain", "Missing parameters");
    }
  });

  // Start server
  server.begin();

  // Attach interrupt for motion detection
  attachInterrupt(digitalPinToInterrupt(pirPin), motionDetected_ISR, CHANGE);

  // Start camera stream
  startCameraServer();
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  if (motionDetected) {
    captureAndSendImage();
    sendTelegramMessage("Motion detected!");
    mqttClient.publish(mqttTopic, "Motion detected!");
    motionDetected = false;
  }
}

void motionDetected_ISR() {
  motionDetected = true;
}

void captureAndSendImage() {
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  digitalWrite(buzzerPin, HIGH); // Bunyikan buzzer
  delay(1000); // Tahan bunyi buzzer selama 1 detik
  digitalWrite(buzzerPin, LOW); // Matikan buzzer

  esp_camera_fb_return(fb);
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (mqttClient.connect("ESP32-CAM", mqttUser, mqttPassword)) {
      Serial.println("Connected to MQTT broker");
      mqttClient.subscribe(mqttTopic);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Retrying in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Callback function untuk menangani pesan MQTT yang diterima
  // Anda dapat menambahkan logika di sini sesuai kebutuhan Anda
}

void sendTelegramMessage(const char* message) {
  // Kirim pesan ke bot Telegram
  String url = "https://api.telegram.org/bot" + String(telegramBotToken) + "/sendMessage?chat_id=" + String(telegramChatId) + "&text=" + String(message);
  HTTPClient http;
  http.begin(url);
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
  } else {
    Serial.print("Error on sending message. Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

void startCameraServer() {
  server.on("/stream", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "multipart/x-mixed-replace; boundary=frame", streamProcessor, NULL);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
}

void streamProcessor(AsyncWebServerRequest *request, uint8_t *buffer, size_t maxLen, size_t index) {
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  size_t jpg_buf_len = 0;
  uint8_t * jpg_buf = NULL;
  bool ok = frame2jpg(fb, 80, &jpg_buf, &jpg_buf_len);
  esp_camera_fb_return(fb);
  if (!ok) {
    Serial.println("JPEG compression failed");
    return;
  }

  char part_buf[64];
  size_t hlen = snprintf(part_buf, 64, "\r\n--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", jpg_buf_len);
  if (index == 0) {
    memcpy(buffer, part_buf, hlen);
    buffer += hlen;
    index += hlen;
  }

  memcpy(buffer + index, jpg_buf, jpg_buf_len);
  index += jpg_buf_len;
  free(jpg_buf);
}
