#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); 

volatile byte half_revolutions;
unsigned int rpmku;
unsigned long timeold;

int kalibrasi;

char customKey;
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'D', 'C', 'B', 'A'},
  {'#', '9', '6', '3'},
  {'0', '8', '5', '2'},
  {'*', '7', '4', '1'}
};
byte rowPins[ROWS] = {4, 5, 6, 7};
byte colPins[COLS] = {8, 9, 10, 11};

int x = 0;

Keypad customKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

int speedx = 3;

float kp;
float nilaikp;
float ki;
float nilaiki;
float kd;
float nilaikd;
float sp;
float nilaisp;
float error, errorx, sumerr;
float p, i, d, pid, rpmfix;
float selisih;

unsigned long startMillis;

void setup() {
  Serial.begin(9600); 
  Serial.println("Starting...");
  Serial.println("Time RPM KP KI KD"); 

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.noCursor();

  attachInterrupt(0, rpm_fun, RISING);
  half_revolutions = 0;
  rpmku = 0;
  timeold = 0;
  kalibrasi = 0;

  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(speedx, OUTPUT);
  pinMode(2, INPUT);

  digitalWrite(A0, HIGH);
  digitalWrite(A1, LOW);
}

void loop() {
  customKey = customKeypad.getKey();

  if (x == 0) {
    lcd.setCursor(0, 0);
    lcd.print("1.SET PID       ");
  }

  if (x == 1) {
    lcd.setCursor(0, 0);
    lcd.print("2.MULAI        ");
  }

  if (x == 2) {
    lcd.setCursor(0, 0);
    lcd.print("3.CEK PID     ");
  }

  switch (customKey) {
    case '0' ... '9':
      break;

    case '#':
      break;

    case '*':
      break;

    case 'A':
      x++;
      break;

    case 'B':
      x--;
      break;

    case 'C':
      break;

    case 'D':
      if (x == 0) {
        lcd.clear();
        setkp();
        setki();
        setkd();
        setsp();
        cekpid();
      }
      if (x == 1) {
        lcd.clear();
        mulai();
      }
      if (x == 2) {
        lcd.clear();
        cekpid();
      }
      break;
  }

  if (x > 2) {
    x = 0;
  }

  if (x < 0) {
    x = 2;
  }
}

void setkp() {
  lcd.setCursor(0, 0);
  lcd.print("SET KP     ");

  customKey = customKeypad.getKey();

  if (customKey >= '0' && customKey <= '9') {
    kp = kp * 10 + (customKey - '0');
    lcd.setCursor(0, 1);
    lcd.print(kp);
  }

  if (customKey == 'A') {
    lcd.clear();
    delay(1000);
    kp = kp / 100;
    nilaikp = kp;
    return;
  }

  if (customKey == 'B') {
    lcd.clear();
    delay(1000);
    kp = kp / 1000;
    nilaikp = kp;
    return;
  }

  if (customKey == '*') {
    lcd.clear();
    delay(1000);
    nilaikp = kp;
    return;
  }

  setkp();
}

void setki() {
  lcd.setCursor(0, 0);
  lcd.print("SET KI     ");

  customKey = customKeypad.getKey();

  if (customKey >= '0' && customKey <= '9') {
    ki = ki * 10 + (customKey - '0');
    lcd.setCursor(0, 1);
    lcd.print(ki);
  }

  if (customKey == 'A') {
    lcd.clear();
    delay(1000);
    ki = ki / 100;
    nilaiki = ki;
    return;
  }

  if (customKey == 'B') {
    lcd.clear();
    delay(1000);
    ki = ki / 1000;
    nilaiki = ki;
    return;
  }

  if (customKey == '*') {
    lcd.clear();
    delay(1000);
    nilaiki = ki;
    return;
  }

  setki();
}

void setkd() {
  lcd.setCursor(0, 0);
  lcd.print("SET KD     ");

  customKey = customKeypad.getKey();

  if (customKey >= '0' && customKey <= '9') {
    kd = kd * 10 + (customKey - '0');
    lcd.setCursor(0, 1);
    lcd.print(kd);
  }

  if (customKey == 'A') {
    lcd.clear();
    delay(1000);
    kd = kd / 100;
    nilaikd = kd;
    return;
  }

  if (customKey == 'B') {
    lcd.clear();
    delay(1000);
    kd = kd / 1000;
    nilaikd = kd;
    return;
  }

  if (customKey == '*') {
    lcd.clear();
    delay(1000);
    nilaikd = kd;
    return;
  }

  setkd();
}

void setsp() {
  lcd.setCursor(0, 0);
  lcd.print("SET SP     ");

  customKey = customKeypad.getKey();

  if (customKey >= '0' && customKey <= '9') {
    sp = sp * 10 + (customKey - '0');
    lcd.setCursor(0, 1);
    lcd.print(sp);
  }

  if (customKey == '*') {
    lcd.clear();
    delay(1000);
    nilaisp = sp;
    return;
  }

  setsp();
}

void cekpid() {
  lcd.setCursor(0, 0);
  lcd.print("KP");
  lcd.print(nilaikp, 2);
  lcd.print("-KI");
  lcd.print(nilaiki, 2);

  lcd.setCursor(0, 1);
  lcd.print("KD");
  lcd.print(nilaikd, 2);
  lcd.print("-SP");
  lcd.print(nilaisp, 2);

  customKey = customKeypad.getKey();

  if (customKey == '*') {
    lcd.clear();
    delay(1000);
    return;
  }

  cekpid();
}

void mulai() {
  static bool firstRun = true;
  if (firstRun) {
    startMillis = millis();
    firstRun = false;
  }
  error = nilaisp - rpmfix;
  p = error * nilaikp;
  sumerr = error + errorx;
  i = nilaiki * sumerr;
  selisih = error - errorx;
  d = nilaikd * selisih;
  pid = p + i + d;

  if (pid < 0) {
    pid = 0;
  }

  if (pid > 255) {
    pid = 255; // Limit the PID to max value for PWM
  }

  analogWrite(speedx, pid);

  rpmku = 30 * 1000 / (millis() - timeold) * half_revolutions;
  timeold = millis();
  half_revolutions = 0;

  kalibrasi = (rpmku - 150) / 109;
  rpmfix = kalibrasi * 10;

  if (rpmfix > 2000) {
    rpmfix = 0;
  }

  lcd.setCursor(0, 0);
  lcd.print("RPM= ");
  lcd.print(rpmfix);
  lcd.print("       ");
  lcd.setCursor(0, 1);
  lcd.print("PID= ");
  lcd.print(pid);
  lcd.print("       ");

  // Print data for Serial Plotter with timestamp
  unsigned long currentMillis = millis();
  unsigned long elapsedMillis = currentMillis - startMillis;
  Serial.print(elapsedMillis / 1000.0);  // Mengirim waktu dalam detik
  Serial.print(" ");
  Serial.println(rpmfix);

  delay(200);
  customKey = customKeypad.getKey();

  if (customKey == '*') {
    lcd.clear();
    kp = 0;
    ki = 0;
    kd = 0;
    sp = 0;
    analogWrite(speedx, 0);
    delay(1000);
    return;
  }

  errorx = error;
  mulai();
}

void rpm_fun() {
  half_revolutions++;
}
