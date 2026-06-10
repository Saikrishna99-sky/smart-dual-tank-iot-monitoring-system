
#define TINY_GSM_MODEM_SIM7600
#include <otadrive_esp.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiAP.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <Arduino.h>
#include <TimeLib.h>
#include <SimpleTimer.h>
#include <TinyGsmClient.h>
#include <ESP32Time.h>
#include <time.h>
#include "cpp1.h"
#include "cpp3.h"
float lastLowerLevel = 0.0;
int lastLowerRaw = 0;
bool lowerCutoffActive = false;

String alertEmail = "";
String senderEmail = "";
String senderPassword = "";
bool emailAlertEnable = false;
bool emailAlreadySent = false;

String base64Encode(String input) {
  const char* base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

  String encoded = "";
  int i = 0;
  unsigned char arr3[3];
  unsigned char arr4[4];

  for (int pos = 0; pos < input.length(); pos++) {
    arr3[i++] = input[pos];

    if (i == 3) {
      arr4[0] = (arr3[0] & 0xfc) >> 2;
      arr4[1] = ((arr3[0] & 0x03) << 4) + ((arr3[1] & 0xf0) >> 4);
      arr4[2] = ((arr3[1] & 0x0f) << 2) + ((arr3[2] & 0xc0) >> 6);
      arr4[3] = arr3[2] & 0x3f;

      for (i = 0; i < 4; i++) encoded += base64_chars[arr4[i]];
      i = 0;
    }
  }

  if (i) {
    for (int j = i; j < 3; j++) arr3[j] = '\0';

    arr4[0] = (arr3[0] & 0xfc) >> 2;
    arr4[1] = ((arr3[0] & 0x03) << 4) + ((arr3[1] & 0xf0) >> 4);
    arr4[2] = ((arr3[1] & 0x0f) << 2) + ((arr3[2] & 0xc0) >> 6);

    for (int j = 0; j < i + 1; j++) encoded += base64_chars[arr4[j]];
    while (i++ < 3) encoded += '=';
  }

  return encoded;
}

void sendEmailAlert() {
  if (!emailAlertEnable) {
    Serial.println("EMAIL DISABLED");
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("EMAIL FAIL: WiFi not connected");
    return;
  }

  if (alertEmail.length() == 0 || senderEmail.length() == 0 || senderPassword.length() == 0) {
    Serial.println("EMAIL FAIL: Missing email/app password");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();

  Serial.println("Connecting to Gmail SMTP...");

  if (!client.connect("smtp.gmail.com", 465)) {
    Serial.println("SMTP connect fail");
    return;
  }

  auto waitSMTP = [&client]() {
    unsigned long timeout = millis();
    while (!client.available() && millis() - timeout < 5000) {
      delay(10);
    }

    while (client.available()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);
    }
  };

  waitSMTP();

  client.println("EHLO esp32");
  waitSMTP();

  client.println("AUTH LOGIN");
  waitSMTP();

  client.println(base64Encode(senderEmail));
  waitSMTP();

  client.println(base64Encode(senderPassword));
  waitSMTP();

  client.println("MAIL FROM:<" + senderEmail + ">");
  waitSMTP();

  client.println("RCPT TO:<" + alertEmail + ">");
  waitSMTP();

  client.println("DATA");
  waitSMTP();

  client.println("From: Lower Tank <" + senderEmail + ">");
  client.println("To: " + alertEmail);
  client.println("Subject: LOWER TANK WATER_LEVEL ALERT");
  client.println("Content-Type: text/plain; charset=UTF-8");
  client.println();

  client.println("ALERT: Lower tank LEVEL dropped below 250.");
  client.println();
  client.println("ACTION:NEED TO CALL WATER TANK TO FILL .");
  client.print("Lower Level: ");
  client.print(lastLowerLevel);
  client.println("%");
  client.print("RAW Value: ");
  client.println(lastLowerRaw);
  client.print("Cutoff Active: ");
  client.println(lowerCutoffActive ? "YES" : "NO");
  client.println();
  client.println("Device: Lower Tank Controller");

  client.println(".");
  waitSMTP();

  client.println("QUIT");
  waitSMTP();

  client.stop();

  Serial.println("EMAIL SENT SUCCESSFULLY");
}

ESP32Time rtc(0);

int DF;
int SDtime;

SimpleTimer SecondTimer(60000);
SimpleTimer sd_timer(60000 * 7);
String modifiedData;
unsigned long lastDisconnectTime = 0;

bool sd_flag = false;
bool newfile_flag = false;
bool sd_mqtt_flag = false;
bool use_4g_network = true;

// ── IO Board buffer ──
String ioBuffer = "";
bool ioBuffer_ready = false;
String _rxLine = "";
String _rxJson = "";
bool _rxStarted = false;
WiFiClient wifi_client;
PubSubClient mqtt_wifi_client(wifi_client);

String Device_Id;
String Data_freq;
String TimeSD;
String receivedMessage;

const int BUFFER_SIZE = 2048;
char buf[BUFFER_SIZE];

#define RXD2 26
#define TXD2 25

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 0;

#define MQTT_STATUS_LED 33

HardwareSerial& sim7600Serial = Serial1;
TinyGsm modem(sim7600Serial);
TinyGsmClient gsm_otadrive_client(modem, 1);
TinyGsmClient gsm_mqtt_client(modem, 0);
PubSubClient mqtt_4g_client(gsm_mqtt_client);
String gprs_ip;

SoftwareSerial mySerial(13, 14);

String input_value;
const char* broker;
const char* sub_topic;
const char* pub_topic;
const char* pub_topic1;
const char* username;
const char* pswd;
int port;

bool is_reconnecting = false;

const char* ssid = "LOWER_TANK";
const char* password = "123456789";

String postData;
String currentTime;
String io_data;
int DI1 = 0, DI2 = 0, DI3 = 0, DI4 = 0, DI5 = 0, DI6 = 0, DI7 = 0, DI8 = 0;
int DI9 = 0, DI10 = 0, DI11 = 0, DI12 = 0, DI13 = 0, DI14 = 0, DI15 = 0, DI16 = 0;
int DO1 = 0, DO2 = 0, DO3 = 0, DO4 = 0;
int AI1 = 0, AI2 = 0, AI3 = 0, AI4 = 0;
int AI5 = 0, AI6 = 0, AI7 = 0, AI8 = 0;

IPAddress local_ip(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

AsyncWebServer server(80);
Preferences prefs;
int current_hrs;

struct wifiCred {
  char epSsid[50];
  char epPassword[50];
  char epDID[50];
  int epDF;
  int epTSD;
};
wifiCred retrieveWifi = { "", "" };

struct MqttConfig {
  char broker[100];
  char pubtopic[100];
  char pubtopic1[100];
  char subtopic[100];
  char username[100];
  char password[100];
  int epmqtt_port;
  String mqtt_server;
  bool use_4g;
};
MqttConfig mqttConfig;
// ─────────────────── Forward declarations ───────────────────
void get_wifiCred();
void get_mqttconfi();
void sim_mqttconnect();
void re_mqttconnect();
void wifi_mqttconnect();
void connectToWiFi();
void getTimeFromNTP();
void get_time();
void receive_data();
void publishMessage(String, String);
void handleTankCommand(String);
void wifi_mqtt_callback(char*, byte*, unsigned int);
void mqtt_4g_callback(char*, byte*, unsigned int);
void sim7600ATCommand(const char*);
void parseTimeInfo(String);
void sendATCommands(const char*);
String sendATCommand(String, String);
void put_wifiCred(String, String, String, String, String);
String mergeJsonStrings(const String&, const String&);

void save_email_config();
void load_email_config();


void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);

  pinMode(27, OUTPUT);
  pinMode(MQTT_STATUS_LED, OUTPUT);
  digitalWrite(MQTT_STATUS_LED, LOW);

  get_wifiCred();
  if (DF <= 0) DF = 1;
  SecondTimer.setInterval(60000 * DF);

  get_mqttconfi();
  load_email_config();
pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  delay(3000); 
  sim7600Serial.begin(115200, SERIAL_8N1, RXD2, TXD2);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.println("AP started: " + String(ssid));
  delay(200);


  server.on("/", HTTP_GET, [](AsyncWebServerRequest* r) {
    r->send(200, "text/html", HTML);
  });

  server.on("/cpp1", HTTP_GET, [](AsyncWebServerRequest* r) {
    r->send(200, "text/html", HTML);
  });

  server.on("/cpp3", HTTP_GET, [](AsyncWebServerRequest* r) {
    r->send(200, "text/html", HTML2);
  });


  server.on("/cloud", HTTP_GET, [](AsyncWebServerRequest* r) {
    r->send(200, "text/html", HTML2);
  });

  server.on("/cluod", HTTP_GET, [](AsyncWebServerRequest* r) {
    r->send(200, "text/html", HTML2);
  });

  server.on("/save", HTTP_POST, [](AsyncWebServerRequest* request) {
    String ssidsta = request->arg("ssid");
    String passwordsta = request->arg("password");
    Device_Id = request->arg("DeviceID");
    Data_freq = request->arg("data_freq");
    TimeSD = request->arg("quantity");
    put_wifiCred(ssidsta, passwordsta, Device_Id, Data_freq, TimeSD);
    get_wifiCred();
    SecondTimer.setInterval(60000 * DF);
    request->redirect("/cpp3");
  });

  server.on("/cluod", HTTP_POST, [](AsyncWebServerRequest* request) {
    strcpy(mqttConfig.broker, request->arg("Host IP or Domain").c_str());
    strcpy(mqttConfig.pubtopic, request->arg("Publish Topic").c_str());
    strcpy(mqttConfig.pubtopic1, request->arg("Publish Topic_sd").c_str());
    strcpy(mqttConfig.subtopic, request->arg("Subscribe Topic").c_str());
    strcpy(mqttConfig.username, request->arg("User Name").c_str());
    strcpy(mqttConfig.password, request->arg("Password").c_str());
    mqttConfig.epmqtt_port = request->arg("Port").toInt();
    mqttConfig.mqtt_server = request->arg("Host IP or Domain");
    String nm = request->arg("network_mode");
    mqttConfig.use_4g = (nm == "4g");
    use_4g_network = mqttConfig.use_4g;
    prefs.begin("mqtt", false);
    prefs.putBytes("config", &mqttConfig, sizeof(mqttConfig));
    prefs.end();
    get_mqttconfi();
  load_email_config();
    alertEmail = request->arg("Alert Email");
    senderEmail = request->arg("Sender Gmail");
    senderPassword = request->arg("App Password");
    emailAlertEnable = request->hasArg("Enable Alert");
    save_email_config();
    request->send(200, "text/html", "<html><body><h3>Configuration Saved Successfully</h3><a href=\"/cpp3\">Back to Cloud Page</a></body></html>");
  });

  server.on("/confi", HTTP_GET, [](AsyncWebServerRequest* request) {
    StaticJsonDocument<1000> data;
    data["ssid"] = retrieveWifi.epSsid;
    data["DeviceID"] = retrieveWifi.epDID;
    data["data_freq"] = retrieveWifi.epDF;
    data["quantity"] = retrieveWifi.epTSD;
    data["Domain"] = mqttConfig.broker;
    data["Po"] = mqttConfig.epmqtt_port;
    data["PT1"] = mqttConfig.pubtopic1;
    data["ST"] = mqttConfig.subtopic;
    data["UN"] = mqttConfig.username;
    data["Pas"] = mqttConfig.password;
    data["PT0"] = mqttConfig.pubtopic;
    data["network_mode"] = mqttConfig.use_4g ? "4g" : "wifi";
    data["alertEmail"] = alertEmail;
    data["senderEmail"] = senderEmail;
    data["emailEnable"] = emailAlertEnable;
    String resp;
    serializeJson(data, resp);
    request->send(200, "application/json", resp);
  });

  server.begin();

  if (use_4g_network) {
    sim_mqttconnect();
  } else {
    connectToWiFi();
    wifi_mqttconnect();
  }

  get_time();
}

void loop() {
  if (use_4g_network) {
    mqtt_4g_client.loop();
    if (!mqtt_4g_client.connected() && !is_reconnecting) {
      digitalWrite(MQTT_STATUS_LED, LOW);
      static unsigned long lastLoopRecon = 0;
      if (millis() - lastLoopRecon > 5000) {
        lastLoopRecon = millis();
        re_mqttconnect();
      }
    } else {
      digitalWrite(MQTT_STATUS_LED, HIGH);
    }
  } else {
    if (!mqtt_wifi_client.connected() && !is_reconnecting) {
      digitalWrite(MQTT_STATUS_LED, LOW);
      is_reconnecting = true;
      wifi_mqttconnect();
      is_reconnecting = false;
    } else {
      digitalWrite(MQTT_STATUS_LED, HIGH);
    }
    mqtt_wifi_client.loop();
  }

  // executePendingWrite();           // ── MODBUS COMMENTED ──
  receive_data();

  if (SecondTimer.isReady()) {
    SecondTimer.reset();
    if (use_4g_network) {
      sd_flag = !mqtt_4g_client.connected();
    } else {
      if (WiFi.status() != WL_CONNECTED) connectToWiFi();
      sd_flag = (WiFi.status() != WL_CONNECTED);
      if (!sd_flag && !mqtt_wifi_client.connected()) {
        is_reconnecting = true;
        wifi_mqttconnect();
        is_reconnecting = false;
      }
    }
    get_time();
    delay(50);

    // ── IO read & publish ──────────────────────────────────────
    _rxStarted = false;
    _rxJson = "";
    _rxLine = "";
    ioBuffer_ready = false;
    mySerial.println("READ");
    Serial.println("[TIMER] READ sent to IO board");

    unsigned long ioWait = millis();
    while (!ioBuffer_ready && millis() - ioWait < 600) {
      receive_data();
      delay(5);
    }

    if (ioBuffer_ready && ioBuffer.length() > 0) {
      Serial.println("[TIMER] Publishing: " + ioBuffer);
      if (!sd_flag) publishMessage(pub_topic, ioBuffer);
      ioBuffer_ready = false;
    } else {
      Serial.println("[TIMER] No IO data");
    }
  }
}

// ═══════════════════════════════════════════════════════════════
//                      FUNCTION DEFINITIONS
// ═══════════════════════════════════════════════════════════════

void handleTankCommand(String cmd) {

  Serial.println("[LOWER CMD] Received: " + cmd);

  if (cmd == "MOTOR:1") {

    Serial.printf("[LOWER] Level = %.1f%%\n", lastLowerLevel);
    Serial.printf("[LOWER] Cutoff State = %s\n",
                  lowerCutoffActive ? "ACTIVE" : "NORMAL");

   
    if (!lowerCutoffActive) {

      // SAFE → MOTOR ON
      mySerial.println("MOTOR:1");

      Serial.println("[LOWER] SAFE → MOTOR ON sent");

      StaticJsonDocument<256> ack;

      ack["DeviceId"] = retrieveWifi.epDID;
      ack["event"] = "MOTOR_ON";
      ack["status"] = "OK";
      ack["lower_level"] = lastLowerLevel;
      ack["time"] = currentTime;

      String ackStr;
      serializeJson(ack, ackStr);

      publishMessage(pub_topic, ackStr);

    } else {

      // BLOCKED → LOWER TANK EMPTY

      Serial.println("[LOWER] BLOCKED - AUTO CUTOFF ACTIVE");

      StaticJsonDocument<128> alert;

      alert["DeviceId"] = retrieveWifi.epDID;
      alert["event"] = "MOTOR_BLOCKED";
      alert["reason"] = "AUTO_CUTOFF_ACTIVE";
      alert["lower_level"] = lastLowerLevel;
      alert["time"] = currentTime;

      String alertStr;
      serializeJson(alert, alertStr);

      publishMessage(pub_topic, alertStr);
    }
  }

  else if (cmd == "MOTOR:0") {

    mySerial.println("MOTOR:0");

    Serial.println("[LOWER] MOTOR OFF");

    StaticJsonDocument<128> ack;

    ack["DeviceId"] = retrieveWifi.epDID;
    ack["event"] = "MOTOR_OFF";
   ack["lower_level"] = serialized(String(lastLowerLevel, 2));
    ack["time"] = currentTime;

    String ackStr;
    serializeJson(ack, ackStr);

    publishMessage(pub_topic, ackStr);
  }
}

// ─── MQTT callbacks ─────────────────────────────────────────
void mqtt_4g_callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) message += (char)payload[i];
  Serial.println("4G MQTT msg: " + message);
  receivedMessage = message;

  if (String(topic) == String(sub_topic)) {
    handleTankCommand(message);
    return;
  }
}

void wifi_mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) message += (char)payload[i];
  Serial.println("WiFi MQTT msg: " + message);
  receivedMessage = message;

  if (String(topic) == String(sub_topic)) {
    handleTankCommand(message);
    return;
  }
}

// ─── Network connect ────────────────────────────────────────
void connectToWiFi() {
  get_wifiCred();

  if (strlen(retrieveWifi.epSsid) == 0) {
    Serial.println("SSID EMPTY");
    return;
  }

  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect(false);  
  delay(1000);

  Serial.println("Connecting WiFi...");

  WiFi.begin(retrieveWifi.epSsid, retrieveWifi.epPassword);

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected");
    Serial.println(WiFi.localIP());
    getTimeFromNTP();
  } else {
    Serial.println("\nWiFi Failed");
    Serial.println(WiFi.status());
  }
}

void sim_mqttconnect() {
  is_reconnecting = true;
  get_wifiCred();
  get_mqttconfi();
  load_email_config();

  // FIX: default port if not set
  if (port <= 0) port = 1883;

  Serial.println("Connecting 4G MQTT...");

  while (sim7600Serial.available()) sim7600Serial.read();
  sim7600ATCommand("AT");
  sim7600ATCommand("AT+CFUN=1");
  sim7600ATCommand("AT+CSQ");
  sim7600ATCommand("AT+CPIN?");
  sim7600ATCommand("AT+CPSI?");
  sim7600ATCommand("AT+CGPADDR");
  sim7600ATCommand("AT+CGACT?");
  sim7600ATCommand("AT+CTZU=1");
  sim7600ATCommand("AT+CREG=0");
  sim7600ATCommand("AT+CGREG=0");
  sim7600ATCommand("AT+CTZR=0");
  sim7600Serial.println("AT+NETOPEN");
  delay(3000);
  while (sim7600Serial.available()) sim7600Serial.read();

  String clientId = String(retrieveWifi.epDID);
  if (clientId.isEmpty())
    clientId = "ESP32_" + String((uint32_t)ESP.getEfuseMac(), HEX);
  Serial.println(">>> SIM_CONNECT ClientID: " + clientId);

  mqtt_4g_client.setServer(mqttConfig.broker, port);
  mqtt_4g_client.setCallback(mqtt_4g_callback);
  mqtt_4g_client.setKeepAlive(120);
  mqtt_4g_client.setSocketTimeout(15);
  mqtt_4g_client.setBufferSize(2048);

  for (int r = 0; r < 5 && !mqtt_4g_client.connected(); r++) {
    Serial.printf("MQTT attempt %d\n", r + 1);
    bool ok = (strlen(mqttConfig.username) > 0)
                ? mqtt_4g_client.connect(clientId.c_str(),
                                         mqttConfig.username, mqttConfig.password)
                : mqtt_4g_client.connect(clientId.c_str());
    if (ok) {
      Serial.println("✅ 4G MQTT Connected!");
      mqtt_4g_client.subscribe(sub_topic);
      digitalWrite(MQTT_STATUS_LED, HIGH);
      break;
    }
    Serial.printf("❌ state=%d retry\n", mqtt_4g_client.state());
    delay(1500);
  }
  get_time();
  is_reconnecting = false;
}

void re_mqttconnect() {
  if (is_reconnecting) return;
  is_reconnecting = true;

  prefs.begin("mqtt");
  prefs.getBytes("config", &mqttConfig, sizeof(mqttConfig));
  prefs.end();
  broker = mqttConfig.broker;
  sub_topic = mqttConfig.subtopic;
  pub_topic = mqttConfig.pubtopic;
  pub_topic1 = mqttConfig.pubtopic1;
  username = mqttConfig.username;
  pswd = mqttConfig.password;
  port = mqttConfig.epmqtt_port;
  if (port <= 0) port = 1883;  // FIX: default port
  use_4g_network = mqttConfig.use_4g;

  Serial.println("Reconnecting MQTT...");
  digitalWrite(MQTT_STATUS_LED, LOW);

  String clientId = retrieveWifi.epDID;
  if (clientId.isEmpty())
    clientId = "ESP32_" + String((uint32_t)ESP.getEfuseMac(), HEX);
  Serial.println(">>> RE_CONNECT ClientID: " + clientId);

  mqtt_4g_client.setServer(mqttConfig.broker, port);
  mqtt_4g_client.setCallback(mqtt_4g_callback);
  mqtt_4g_client.setKeepAlive(120);
  mqtt_4g_client.setSocketTimeout(15);

  for (int r = 0; r < 3; r++) {
    bool ok = (strlen(mqttConfig.username) > 0)
                ? mqtt_4g_client.connect(clientId.c_str(),
                                         mqttConfig.username, mqttConfig.password)
                : mqtt_4g_client.connect(clientId.c_str());
    if (ok) {
      Serial.println("Reconnected!");
      mqtt_4g_client.subscribe(sub_topic);
     
      digitalWrite(MQTT_STATUS_LED, HIGH);
      is_reconnecting = false;
      return;
    }
    Serial.printf("[MQTT] attempt %d fail state=%d\n", r + 1, mqtt_4g_client.state());
    delay(500);
  }
  Serial.println("[MQTT] All retries failed");
  is_reconnecting = false;
}

void wifi_mqttconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
    delay(2000);
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return;
  }
  // FIX: direct struct use, not stale const char* pointers
  mqtt_wifi_client.setServer(mqttConfig.broker, mqttConfig.epmqtt_port);
  mqtt_wifi_client.setCallback(wifi_mqtt_callback);
  if (mqtt_wifi_client.connect(retrieveWifi.epDID,
                               mqttConfig.username, mqttConfig.password)) {
    Serial.println("WiFi MQTT Connected!");
    mqtt_wifi_client.subscribe(sub_topic);
    digitalWrite(MQTT_STATUS_LED, HIGH);
  } else {
    Serial.println("WiFi MQTT failed rc=" + String(mqtt_wifi_client.state()));
    digitalWrite(MQTT_STATUS_LED, LOW);
  }
}

// FIX: if(use_4g_network) restored — was broken with dangling else
void publishMessage(String topic, String payload) {
  Serial.println("[PUB] Topic: " + topic);
  Serial.println("[PUB] Payload: " + payload);

  if (use_4g_network) {
    if (!mqtt_4g_client.connected()) {
      Serial.println("[MQTT] Not connected — skipping publish");
      // writeToSDCard(payload);      // ── SD COMMENTED ──
      return;
    }
    mqtt_4g_client.loop();
    bool ok = mqtt_4g_client.publish(topic.c_str(), payload.c_str());

    unsigned long pt = millis();
    while (millis() - pt < 500) {
      mqtt_4g_client.loop();
      delay(10);
    }

    if (ok) {
      Serial.println("✅ Published 4G");
    } else {
      Serial.println("❌ Publish failed");
    }
  } else {
    if (!mqtt_wifi_client.connected()) wifi_mqttconnect();
    if (mqtt_wifi_client.connected()) {
      mqtt_wifi_client.loop();
      mqtt_wifi_client.publish(topic.c_str(), payload.c_str());
      mqtt_wifi_client.loop();
      Serial.println("WiFi Published");
    }
  }
}
void receive_data() {

  while (mySerial.available()) {

    char c = mySerial.read();

    if (c == '\n') {

      _rxLine.trim();

      if (_rxLine == "<JSON_START>") {

        _rxStarted = true;
        _rxJson = "";
        _rxLine = "";
        continue;
      }

      if (_rxLine == "<JSON_END>" && _rxStarted) {

        _rxStarted = false;

        if (_rxJson.length() == 0) {

          _rxLine = "";
          return;
        }

        StaticJsonDocument<256> doc;

        if (deserializeJson(doc, _rxJson)) {

          Serial.println("[RX] JSON parse error!");
          _rxLine = "";
          return;
        }

        // ───── DATA FROM ATMEGA ─────

        int MOTOR = doc["MOTOR"] | 0;
        int RAW = doc["RAW"] | 0;
        float LEVEL = doc["LEVEL"] | 0.0;
        bool CUTOFF_ACTIVE =
          doc["CUTOFF_ACTIVE"] | false;

        // SAVE GLOBALLY
        lastLowerLevel = LEVEL;
        lastLowerRaw = RAW;

        // ✅ SAVE CUTOFF STATE
        lowerCutoffActive = CUTOFF_ACTIVE;

        if (RAW < 250 && !emailAlreadySent) {
          Serial.println("[EMAIL] RAW below 250 -> EMAIL ALERT TRIGGERED");
          sendEmailAlert();
          emailAlreadySent = true;
        }

        if (RAW >= 250) {
          emailAlreadySent = false;
        }

        Serial.printf(
          "[RX] LEVEL=%.1f  CUTOFF=%s\n",
          LEVEL,
          lowerCutoffActive ? "YES" : "NO"
        );

        // ───── BUILD MQTT JSON ─────

        StaticJsonDocument<256> doc1;

        doc1["DeviceId"] = retrieveWifi.epDID;
        doc1["slaveAddress"] = 0;
        doc1["time"] = currentTime;

        doc1["MOTOR"] = MOTOR;
        doc1["RAW"] = RAW;
        doc1["LEVEL"] =serialized(String(LEVEL,2));

        // ✅ EXTRA INFO
        doc1["LOWER_LEVEL"] = serialized(String(LEVEL,2));
        doc1["CUTOFF_ACTIVE"] =
          lowerCutoffActive;

        ioBuffer = "";

        serializeJson(doc1, ioBuffer);

        ioBuffer_ready = true;

        Serial.println("[RX] Buffered OK:");
        Serial.println(ioBuffer);

        _rxLine = "";
        return;
      }

      if (_rxStarted)
        _rxJson += _rxLine;

      _rxLine = "";

    } else {

      _rxLine += c;
    }
  }
}

// ─── Time ────────────────────────────────────────────────────
void getTimeFromNTP() {
  if (WiFi.status() != WL_CONNECTED) return;
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm ti;
  int retry = 0;
  while (!getLocalTime(&ti) && retry < 10) {
    delay(1000);
    retry++;
  }
  if (retry < 10) {
    rtc.setTime(ti.tm_sec, ti.tm_min, ti.tm_hour, ti.tm_mday, ti.tm_mon + 1, ti.tm_year + 1900);
    setTime(ti.tm_hour, ti.tm_min, ti.tm_sec, ti.tm_mday, ti.tm_mon + 1, ti.tm_year + 1900);
    currentTime = String(now());
    Serial.println("NTP time: " + currentTime);
  }
}

void sendATCommands(const char* command) {
  while (sim7600Serial.available()) sim7600Serial.read();
  sim7600Serial.println(command);
  Serial.println("[AT CMD] Sent: " + String(command));

  unsigned long t = millis();
  while (millis() - t < 3000) {
    if (use_4g_network) mqtt_4g_client.loop();
    else mqtt_wifi_client.loop();

    while (sim7600Serial.available()) {
      String resp = sim7600Serial.readStringUntil('\n');
      resp.trim();
      Serial.println("[AT RESP] " + resp);

      if (resp.indexOf("+CCLK:") >= 0) {
        parseTimeInfo(resp);
        return;
      }
    }
    delay(10);
  }
  Serial.println("[AT] timeout — no +CCLK response");
}

void parseTimeInfo(String data) {
  Serial.println("[PARSE] Input: " + data);

  int q = data.indexOf('\"');
  if (q == -1) {
    Serial.println("[PARSE] No quote found");
    return;
  }

  String ts = data.substring(q + 1, data.indexOf('\"', q + 1));
  Serial.println("[PARSE] Timestamp string: " + ts);

  int dy, mo, yr, hr, mn, sc;
  int firstSlash = ts.indexOf('/');

  if (firstSlash == 2) {
    yr = ts.substring(0, 2).toInt() + 2000;
    mo = ts.substring(3, 5).toInt();
    dy = ts.substring(6, 8).toInt();
    hr = ts.substring(9, 11).toInt();
    mn = ts.substring(12, 14).toInt();
    sc = ts.substring(15, 17).toInt();
  } else if (firstSlash == 4) {
    yr = ts.substring(0, 4).toInt();
    mo = ts.substring(5, 7).toInt();
    dy = ts.substring(8, 10).toInt();
    hr = ts.substring(11, 13).toInt();
    mn = ts.substring(14, 16).toInt();
    sc = ts.substring(17, 19).toInt();
  } else {
    Serial.println("[PARSE] Unknown format — firstSlash at: " + String(firstSlash));
    return;
  }

  Serial.printf("[PARSE] yr=%d mo=%d dy=%d hr=%d mn=%d sc=%d\n",
                yr, mo, dy, hr, mn, sc);

  int tzSec = 0;
  int pi = ts.indexOf('+', 9);
  int mi = ts.lastIndexOf('-');
  if (pi != -1) tzSec = ts.substring(pi + 1).toInt() * 15 * 60;
  else if (mi > 8) tzSec = -ts.substring(mi + 1).toInt() * 15 * 60;

  setTime(hr, mn, sc, dy, mo, yr);
  adjustTime(-tzSec);
  time_t u = now();
  rtc.setTime(::second(u), ::minute(u), ::hour(u),
              ::day(u), ::month(u), ::year(u));
  currentTime = String(u);
  Serial.println("[PARSE] UTC Unix: " + currentTime);
}

void get_time() {
  if (use_4g_network) {
    String dt = modem.getGSMDateTime(DATE_FULL);
    Serial.println("[MODEM TIME RAW] " + dt);

    if (dt.length() > 5) {
      String fakeClk = "+CCLK: \"" + dt + "\"";
      Serial.println("[FAKE CCLK] " + fakeClk);
      parseTimeInfo(fakeClk);
    }

    if (currentTime.length() == 0 || currentTime == "0") {
      Serial.println("[TIME] TinyGSM failed — trying AT+CCLK?");
      sendATCommands("AT+CCLK?");
    }

    Serial.println("4G time final: " + currentTime);
  } else {
    getTimeFromNTP();
  }
}

String sendATCommand(String command, String expectedResponse) {
  sim7600Serial.println(command);
  String data;
  unsigned long t = millis();
  while (millis() - t < 1000) {
    while (sim7600Serial.available()) data += (char)sim7600Serial.read();
    if (data.indexOf(expectedResponse) != -1) break;
  }
  return data;
}

void sim7600ATCommand(const char* command) {
  while (sim7600Serial.available()) sim7600Serial.read();
  sim7600Serial.println(command);
  unsigned long t = millis();
  while (millis() - t < 3000) {
    while (sim7600Serial.available()) Serial.write(sim7600Serial.read());
    delay(10);
  }
  Serial.println();
}

// ─── Preferences (get/put) ──────────────────────────────────
void get_wifiCred() {
  prefs.begin("Settings");
  prefs.getBytes("Settings", &retrieveWifi, sizeof(retrieveWifi));
  DF = retrieveWifi.epDF;
  SDtime = retrieveWifi.epTSD;
  prefs.end();
}

void put_wifiCred(String s, String p, String d, String f, String t) {
  wifiCred e;
  memset(&e, 0, sizeof(e));
  strncpy(e.epSsid, s.c_str(), 49);
  strncpy(e.epPassword, p.c_str(), 49);
  strncpy(e.epDID, d.c_str(), 49);
  e.epDF = f.toInt();
  e.epTSD = t.toInt();
  prefs.begin("Settings");
  prefs.putBytes("Settings", &e, sizeof(e));
  prefs.end();
}
void get_mqttconfi() {
  prefs.begin("mqtt");
  prefs.getBytes("config", &mqttConfig, sizeof(mqttConfig));
  prefs.end();
  broker = mqttConfig.broker;
  sub_topic = mqttConfig.subtopic;
  pub_topic = mqttConfig.pubtopic;
  pub_topic1 = mqttConfig.pubtopic1;
  username = mqttConfig.username;
  pswd = mqttConfig.password;
  port = mqttConfig.epmqtt_port;
  if (port <= 0) port = 1883;  // FIX: default port
  use_4g_network = mqttConfig.use_4g;
  Serial.println("Broker:" + String(broker) + " Mode:" + (use_4g_network ? "4G" : "WiFi"));
}


// ─── EMAIL CONFIG SAVE/LOAD ─────────────────────
void save_email_config() {

  prefs.begin("email", false);

  prefs.putString("alert", alertEmail);
  prefs.putString("sender", senderEmail);
  prefs.putString("pass", senderPassword);
  prefs.putBool("enable", emailAlertEnable);

  prefs.end();

  Serial.println("[EMAIL CONFIG] SAVED");
}

void load_email_config() {

  prefs.begin("email", true);

  alertEmail = prefs.getString("alert", "");
  senderEmail = prefs.getString("sender", "");
  senderPassword = prefs.getString("pass", "");
  emailAlertEnable = prefs.getBool("enable", false);

  prefs.end();

  Serial.println("[EMAIL CONFIG] ENABLE = " +
                 String(emailAlertEnable ? "YES" : "NO"));

  Serial.println("[EMAIL CONFIG] ALERT = " + alertEmail);
}


String mergeJsonStrings(const String& j1, const String& j2) {
  if (j2.isEmpty() || j2 == "{}") return j1;
  if (j1.isEmpty() || j1 == "{}") return j2;
  String m = j1;
  int lb = m.lastIndexOf('}');
  if (lb == -1) return j1;
  int fb = j2.indexOf('{');
  if (fb == -1) return j1;
  String tail = j2.substring(fb + 1);
  tail.trim();
  if (tail == "}") return j1;
  return m.substring(0, lb) + ", " + tail;
}