#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>

const char* ssid = "..."  // "nama wifi";
const char* password = "..."  // "passwordwifi";

#define BOT_TOKEN "..." // "(isikodebottoken)"
#define CHAT_ID   "..." // "(isichatid)"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

#define TRIG_IN 5
#define ECHO_IN 18
#define TRIG_OUT 17
#define ECHO_OUT 16
#define LED_RED 26
#define LED_GREEN 27
#define BUZZER 25
#define SERVO_PIN 13
#define SS_PIN 4
#define RST_PIN 2

#define DETECT_DISTANCE 20
#define VEHICLE_TIMEOUT 10000
#define TELEGRAM_INTERVAL 2000

String allowedUID = "6185FC17";

Servo gateServo;
MFRC522 rfid(SS_PIN, RST_PIN);

bool gateOpen = false;
bool isProcessing = false;
unsigned long lastTelegramCheck = 0;

void openGate() {
  gateServo.write(90);
  gateOpen = true;
  Serial.println("[SERVO] Gate OPEN");
}

void closeGate() {
  gateServo.write(0);
  gateOpen = false;
  Serial.println("[SERVO] Gate CLOSE");
}

void setLED(bool red, bool green) {
  digitalWrite(LED_RED, red ? HIGH : LOW);
  digitalWrite(LED_GREEN, green ? HIGH : LOW);
}

void beep(int count, int durationMs) {
  for (int i = 0; i < count; i++) {
    tone(BUZZER, 1000);
    delay(durationMs);
    noTone(BUZZER);

    if (i < count - 1) {
      delay(100);
    }
  }
}

float readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) {
    return 999;
  }

  return duration * 0.034 / 2;
}

void vehicleEntry() {

  // Cek dulu ada kendaraan tidak
  if (readDistance(TRIG_IN, ECHO_IN) > DETECT_DISTANCE) {
    Serial.println("Tidak ada kendaraan!");
    beep(1, 500);
    return;
  }

  isProcessing = true;

  openGate();
  setLED(false, true);
  beep(1, 200);

  Serial.println("Gate terbuka, menunggu kendaraan lewat...");
  bot.sendMessage(CHAT_ID, " Kendaraan Terdeteksi!\nGate Dibuka.", "");

  // Tunggu kendaraan maju — kalau timeout berarti tidak maju-maju
  unsigned long startTime = millis();

  while (readDistance(TRIG_IN, ECHO_IN) <= DETECT_DISTANCE) {

    // Cek penerobos saat gate terbuka
    if (readDistance(TRIG_OUT, ECHO_OUT) <= DETECT_DISTANCE) {

      Serial.println(">> PENEROBOS SAAT GATE TERBUKA!");
      bot.sendMessage(CHAT_ID, " Ada Penerobos!\nGate Ditutup Paksa!", "");

      closeGate();
      setLED(true, false);

      while (readDistance(TRIG_OUT, ECHO_OUT) <= DETECT_DISTANCE) {
        tone(BUZZER, 1000);
        delay(200);
        noTone(BUZZER);
        delay(200);
      }

      Serial.println(">> Penerobos pergi.");
      bot.sendMessage(CHAT_ID, " Penerobos Sudah Pergi.", "");

      isProcessing = false;
      return;
    }

    // Timeout — mobil tidak maju
    if (millis() - startTime > VEHICLE_TIMEOUT) {

      Serial.println("Timeout! Mobil tidak maju. Menutup gate.");
      bot.sendMessage(CHAT_ID, " Kendaraan Tidak Maju!\nGate Ditutup Otomatis.", "");

      closeGate();
      setLED(true, false);
      beep(2, 100);

      isProcessing = false;
      return;
    }

    delay(200);
  }

  // Tunggu kendaraan sampai sensor keluar
  startTime = millis();

  while (readDistance(TRIG_OUT, ECHO_OUT) > DETECT_DISTANCE) {
    if (millis() - startTime > VEHICLE_TIMEOUT) {
      Serial.println("Timeout menunggu sensor keluar.");
      break;
    }

    delay(200);
  }

  // Tunggu kendaraan selesai melewati sensor keluar
  startTime = millis();

  while (readDistance(TRIG_OUT, ECHO_OUT) <= DETECT_DISTANCE) {
    if (millis() - startTime > VEHICLE_TIMEOUT) {
      break;
    }

    delay(200);
  }

  delay(500);

  closeGate();
  setLED(true, false);
  beep(2, 100);

  Serial.println("Gate tertutup, sistem siap.");
  bot.sendMessage(CHAT_ID, " Kendaraan Sudah Lewat.\nGate Ditutup.", "");

  isProcessing = false;
}

void setup() {

  Serial.begin(115200);

  pinMode(TRIG_IN, OUTPUT);
  pinMode(ECHO_IN, INPUT);

  pinMode(TRIG_OUT, OUTPUT);
  pinMode(ECHO_OUT, INPUT);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  pinMode(BUZZER, OUTPUT);

  gateServo.attach(SERVO_PIN);

  closeGate();

  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, LOW);

  SPI.begin(14, 19, 23, 4);
  rfid.PCD_Init();

  byte v = rfid.PCD_ReadRegister(rfid.VersionReg);

  if (v == 0x00 || v == 0xFF) {
    Serial.println("RFID TIDAK TERDETEKSI!");
  } else {
    Serial.println("RFID OK!");
  }

  WiFi.begin(ssid, password);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");

  client.setInsecure();

  bot.sendMessage(
    CHAT_ID,
    " Smart Parking Online!\n\n"
    "Perintah:\n"
    "/buka - Buka gate manual\n"
    "/tutup - Tutup gate manual\n"
    "/status - Cek status gate",
    ""
  );

  Serial.println("=== SMART PARKING GATE READY ===");
}

void loop() {

  // Cek RFID
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {

    String uid = "";

    for (byte i = 0; i < rfid.uid.size; i++) {
      uid += String(rfid.uid.uidByte[i], HEX);
    }

    uid.toUpperCase();

    Serial.print("UID: ");
    Serial.println(uid);

    if (uid == allowedUID) {

      Serial.println(">> Kartu Valid!");
      vehicleEntry();

    } else {

      Serial.println(">> Kartu Tidak Valid!");

      bot.sendMessage(
        CHAT_ID,
        " Akses Ditolak!\nUID: " + uid,
        ""
      );

      beep(3, 100);
      setLED(true, false);
    }

    rfid.PICC_HaltA();
  }

  // Cek penerobos saat gate tertutup
  // (sensor masuk DAN sensor keluar terbaca bersamaan)
  if (!isProcessing &&
      readDistance(TRIG_OUT, ECHO_OUT) <= DETECT_DISTANCE &&
      readDistance(TRIG_IN, ECHO_IN) <= DETECT_DISTANCE) {

    Serial.println(">> PENEROBOS TERDETEKSI DI DEPAN GATE!");

    bot.sendMessage(
      CHAT_ID,
      " Ada Penerobos di Depan Gate!",
      ""
    );

    setLED(true, false);

    while (readDistance(TRIG_OUT, ECHO_OUT) <= DETECT_DISTANCE) {
      tone(BUZZER, 1000);
      delay(200);
      noTone(BUZZER);
      delay(200);
    }

    Serial.println(">> Penerobos pergi.");
    bot.sendMessage(CHAT_ID, " Penerobos Sudah Pergi.", "");
  }

  // Cek pesan Telegram setiap 2 detik
  if (millis() - lastTelegramCheck > TELEGRAM_INTERVAL) {

    lastTelegramCheck = millis();

    int msgCount = bot.getUpdates(bot.last_message_received + 1);

    while (msgCount) {

      for (int i = 0; i < msgCount; i++) {

        String text = bot.messages[i].text;
        String chat = bot.messages[i].chat_id;

        Serial.print("Pesan masuk: ");
        Serial.println(text);

        if (text == "/buka") {

          openGate();
          setLED(false, true);
          beep(1, 200);

          bot.sendMessage(chat, " Gate dibuka secara manual.", "");

        } else if (text == "/tutup") {

          closeGate();
          setLED(true, false);
          beep(2, 100);

          bot.sendMessage(chat, " Gate ditutup secara manual.", "");

        } else if (text == "/status") {

          String status = gateOpen
                            ? " Gate sedang TERBUKA"
                            : " Gate sedang TERTUTUP";

          bot.sendMessage(chat, status, "");

        } else {

          bot.sendMessage(
            chat,
            "Perintah tidak dikenal.\n\n"
            "/buka - Buka gate manual\n"
            "/tutup - Tutup gate manual\n"
            "/status - Cek status gate",
            ""
          );
        }
      }

      msgCount = bot.getUpdates(bot.last_message_received + 1);
    }
  }

  delay(200);
}