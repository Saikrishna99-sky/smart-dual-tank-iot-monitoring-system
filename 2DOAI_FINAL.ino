#include <ArduinoJson.h>

// -------- CALIBRATION --------
int EMPTY_RAW = 200;
int FULL_RAW  = 900;

#define CUTOFF_LEVEL    250   // Motor auto-OFF at/below this RAW value
#define RECOVERY_LEVEL  300   // Motor auto-ON at/above this RAW value
// -----------------------------

#define MOTOR_PIN   16   // DO1
#define BUZZER_PIN  17   // DO2
#define SENSOR_PIN  15   // AI4

static float lvl = 0.0;
static int raw = 0;
static int lastraw = 0;
int do1 = 0;

// -------- MOTOR STATE --------
bool motorRunning = false;
bool autoCutoffActive = false;
// -----------------------------

// -------- BUZZER CONTROL --------
bool buzzerActive = false;
bool buzzerState  = false;

unsigned long buzzerTimer = 0;

const unsigned long BUZZER_ON_TIME  = 5000; // 5 sec ON
const unsigned long BUZZER_OFF_TIME = 2000; // 2 sec OFF

int buzzerCycles = 0;
const int MAX_BUZZER_CYCLES = 3;
// --------------------------------

void setup() {

  Serial.begin(9600);
  Serial.setTimeout(200);

  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);

  digitalWrite(MOTOR_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("LOWER TANK READY");
}

void loop() {

  read_sensor();

  // AUTO CUTOFF:
  // If motor is running and water RAW value is at/below cutoff,
  // motor turns OFF and buzzer starts 3-cycle alert.
  if (motorRunning && lastraw <= CUTOFF_LEVEL) {

    Serial.printf("[LOWER] Level dropped to %.1f%% -> MOTOR OFF + BUZZER ON\n", lvl);

    digitalWrite(MOTOR_PIN, LOW);
    do1 = 0;
    motorRunning = false;

    autoCutoffActive = true;

    start_buzzer_3_cycles();

    Serial.println("[LOWER] AUTO CUTOFF - LOW WATER");

    send_ack("AUTO_CUTOFF", "MOTOR_OFF_BUZZER_ON", lvl);
  }

  // Handle buzzer ON/OFF pattern
  handle_buzzer();

  // Auto recovery: if water comes back above recovery level, motor turns ON
  auto_motor_recovery();

  // Read serial commands
  check_input();

  delay(100);
}

// -------- SENSOR READ --------
void read_sensor() {

  raw = 0;

  for (int i = 0; i < 10; i++) {
    raw += analogRead(SENSOR_PIN);
    delay(2);
  }

  raw /= 10;
  lastraw = raw;

  lvl = (float)(raw - EMPTY_RAW) * 100.0 / (float)(FULL_RAW - EMPTY_RAW);

  if (lvl < 0.0)   lvl = 0.0;
  if (lvl > 100.0) lvl = 100.0;

  do1 = digitalRead(MOTOR_PIN);
}

// -------- START BUZZER 3 CYCLES --------
void start_buzzer_3_cycles() {

  buzzerActive = true;
  buzzerState  = true;
  buzzerCycles = 0;
  buzzerTimer  = millis();

  digitalWrite(BUZZER_PIN, HIGH);

  Serial.println("[BUZZER] STARTED: 5 SEC ON / 2 SEC OFF / 3 TIMES");
}

// -------- BUZZER PATTERN --------
// 5 sec ON -> 2 sec OFF -> repeat 3 times -> complete OFF
void handle_buzzer() {

  if (!buzzerActive) return;

  unsigned long now = millis();

  // Buzzer currently ON
  if (buzzerState) {

    if (now - buzzerTimer >= BUZZER_ON_TIME) {

      digitalWrite(BUZZER_PIN, LOW);
      buzzerState = false;
      buzzerTimer = now;

      buzzerCycles++;

      Serial.printf("[BUZZER] OFF 2 SEC | Cycle %d/%d completed\n",
                    buzzerCycles, MAX_BUZZER_CYCLES);
    }
  }

  // Buzzer currently OFF
  else {

    // After 3 ON cycles, stop completely
    if (buzzerCycles >= MAX_BUZZER_CYCLES) {

      buzzerActive = false;
      buzzerState  = false;
      digitalWrite(BUZZER_PIN, LOW);

      Serial.println("[BUZZER] COMPLETED 3 CYCLES -> STOPPED");
      return;
    }

    if (now - buzzerTimer >= BUZZER_OFF_TIME) {

      digitalWrite(BUZZER_PIN, HIGH);
      buzzerState = true;
      buzzerTimer = now;

      Serial.println("[BUZZER] ON 5 SEC");
    }
  }
}

// -------- AUTO RECOVERY --------
void auto_motor_recovery() {

  if (autoCutoffActive && lastraw >= RECOVERY_LEVEL) {

    Serial.printf("[LOWER] Water recovered RAW=%d -> CUTOFF CLEARED, WAITING FOR UPPER MOTOR:1\n",
                  lastraw);

    // Stop buzzer
    buzzerActive = false;
    buzzerState  = false;
    buzzerCycles = 0;
    digitalWrite(BUZZER_PIN, LOW);

    // Clear cutoff block
    autoCutoffActive = false;

    // Keep motor OFF for safety
    digitalWrite(MOTOR_PIN, LOW);
    do1 = 0;
    motorRunning = false;

    send_ack("AUTO_RECOVERY", "CUTOFF_CLEARED_WAITING_COMMAND", lvl);
  }
}

// -------- SEND DATA --------
void send_data() {

  StaticJsonDocument<256> doc;

  doc["MOTOR"]         = do1;
  doc["RAW"]           = raw;
  doc["LEVEL"]         = lvl;
  doc["CUTOFF_ACTIVE"] = autoCutoffActive;

  Serial.println("<JSON_START>");
  serializeJson(doc, Serial);
  Serial.println();
  Serial.println("<JSON_END>");
  Serial.flush();

  delay(50);
}

// -------- COMMAND HANDLER --------
void check_input() {

  if (!Serial.available()) return;

  String input = Serial.readStringUntil('\n');
  input.trim();

  if (input == "READ") {
    send_data();
    return;
  }

  else if (input == "MOTOR:1") {

    // If motor already ON, ignore repeated MOTOR:1 command
    if (motorRunning) {
      send_ack("MOTOR_ON", "ALREADY_ON", lvl);
      return;
    }

    handle_motor_on();
    return;
  }

  else if (input == "MOTOR:0") {
    handle_motor_off();
    return;
  }

  else if (input == "BUZZER:1") {
    start_buzzer_3_cycles();
    return;
  }

  else if (input == "BUZZER:0") {
    buzzerActive = false;
    buzzerState  = false;
    buzzerCycles = 0;
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("[BUZZER] STOPPED");
    return;
  }
}

// -------- MOTOR ON --------
void handle_motor_on() {

  Serial.printf("[LOWER] raw=%d  level=%.1f%%\n", raw, lvl);

  // During auto cutoff, block MOTOR:1 command until water recovers
  if (autoCutoffActive) {
    Serial.printf("[LOWER] BLOCKED - AUTO CUTOFF ACTIVE (level=%.1f%%)\n", lvl);
    send_ack("MOTOR_ON", "BLOCKED_CUTOFF", lvl);
    return;
  }

  // Water is above cutoff -> allow motor ON
  if (lastraw > CUTOFF_LEVEL) {

    // Stop any buzzer
    buzzerActive = false;
    buzzerState  = false;
    buzzerCycles = 0;
    digitalWrite(BUZZER_PIN, LOW);

    digitalWrite(MOTOR_PIN, HIGH);
    do1 = 1;
    motorRunning = true;

    Serial.println("[LOWER] MOTOR ON");
    send_ack("MOTOR_ON", "OK", lvl);
  }

  // Water is low -> block motor and start buzzer
  else {

    digitalWrite(MOTOR_PIN, LOW);
    do1 = 0;
    motorRunning = false;
    autoCutoffActive = true;

    if (!buzzerActive) {
      start_buzzer_3_cycles();
    }

    Serial.println("[LOWER] BLOCKED - LOW WATER");
    send_ack("MOTOR_ON", "BLOCKED_LOW_WATER", lvl);
  }
}

// -------- MOTOR OFF --------
void handle_motor_off() {

  digitalWrite(MOTOR_PIN, LOW);
  do1 = 0;
  motorRunning = false;

  // Manual OFF clears cutoff and buzzer
  autoCutoffActive = false;

  buzzerActive = false;
  buzzerState  = false;
  buzzerCycles = 0;
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("[LOWER] MOTOR OFF");
  send_ack("MOTOR_OFF", "OK", lvl);
}

// -------- ACK --------
void send_ack(String cmd, String sts, float value) {

  StaticJsonDocument<256> doc;

  doc["ACK"]         = cmd;
  doc["STATUS"]      = sts;
  doc["LOWER_LEVEL"] = value;
  doc["RAW"]         = raw;
  doc["MOTOR"]       = do1;

  serializeJson(doc, Serial);
  Serial.println();
}
