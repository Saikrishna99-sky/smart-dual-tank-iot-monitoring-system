#include <ArduinoJson.h>

uint16_t ai_state[20];

// -------- CALIBRATION --------
int EMPTY_RAW = 205;
int FULL_RAW  = 630;
// -----------------------------

void setup() {
  Serial.begin(9600);
  Serial.println("begin");
  Serial.setTimeout(200);
  pinMode(A6, INPUT);
   Serial.println("UPPER TANK SYSTEM READY");
}
void loop() {
  io_poll();
  check_input();
  delay(100);
}

// -------- SENSOR READ --------
void io_poll() {
  int raw = 0;
  for (int i = 0; i < 10; i++) {
    raw += analogRead(A6);   // ✅ A6 pin
    delay(2);
  }
  raw /= 10;

  float level = (raw - EMPTY_RAW) * 100.0 / (FULL_RAW - EMPTY_RAW);
  if (level < 0)   level = 0;
  if (level > 100) level = 100;

  ai_state[5] = raw;
  ai_state[6] = level * 10;
}

// -------- SEND JSON --------
void send_data() {
  StaticJsonDocument<256> doc;
  doc["RAW"]   = ai_state[5];
  doc["LEVEL"] = ai_state[6] / 10.0;

  Serial.println("<JSON_START>");
  serializeJson(doc, Serial);
  Serial.println();
  Serial.println("<JSON_END>");
  Serial.flush();
  delay(50);
}

// -------- COMMAND --------
void check_input() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input == "READ") send_data();
  }
}