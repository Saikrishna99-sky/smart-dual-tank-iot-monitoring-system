// Simplified version — NO FreeRTOS tasks, single loop()
// Fixes: MQTT disconnect + SD card open failure
// MODIFIED: SD card, Modbus, FOTA functions commented out

#define TINY_GSM_MODEM_SIM7600
//#define TINY_GSM_MODEM_A7672X
#include <otadrive_esp.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>
// #include <ModbusMaster.h>        // ── MODBUS COMMENTED ──
#include <Preferences.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <Arduino.h>
// #include <SD.h>                  // ── SD COMMENTED ──
// #include <SPI.h>                 // ── SD COMMENTED ──
#include <TimeLib.h>
#include <SimpleTimer.h>
#include <TinyGsmClient.h>
#include <ESP32Time.h>
#include <time.h>

#include "cpp1.h"
#include "cpp2.h"
#include "cpp3.h"
#include "cpp4.h"
#include "cpp5.h"
#include "cpp6.h"

ESP32Time rtc(0);

// define APIKEY "e49c9ab1-e86b-4814-80e8-9e91c2749a40"
// #define FW_VER "#v@1.2"

int DF;
int SDtime;
String modbusData;

SimpleTimer SecondTimer(60000);   // will be updated after loading DF
SimpleTimer sd_timer(60000 * 7);  // SD post timer
String modifiedData;
unsigned long lastDisconnectTime = 0;

bool sd_flag        = false;
bool newfile_flag   = false;
bool sd_mqtt_flag   = false;
bool use_4g_network = true;

// ── IO Board buffer ──
String ioBuffer = "";
bool   ioBuffer_ready = false;
String  _rxLine    = "";
String  _rxJson    = "";
bool    _rxStarted = false;
WiFiClient wifi_client;
PubSubClient mqtt_wifi_client(wifi_client);

// const int chipSelect = 5;        // ── SD COMMENTED ──
String Device_Id;
String Data_freq;
String TimeSD;
String receivedMessage;

const int BUFFER_SIZE = 2048;
char buf[BUFFER_SIZE];

#define RXD2 26
#define TXD2 25

const char* ntpServer         = "pool.ntp.org";
const long  gmtOffset_sec     = 0;
const int   daylightOffset_sec = 0;

#define MQTT_STATUS_LED 33

HardwareSerial& sim7600Serial = Serial1;
TinyGsm         modem(sim7600Serial);
TinyGsmClient   gsm_otadrive_client(modem, 1);
TinyGsmClient   gsm_mqtt_client(modem, 0);
PubSubClient    mqtt_4g_client(gsm_mqtt_client);
String          gprs_ip;

SoftwareSerial mySerial(13, 14);

String       input_value;
const char*  broker;
const char*  sub_topic;
const char*  pub_topic;
const char*  pub_topic1;
const char*  username;
const char*  pswd;
int          port;

bool is_reconnecting = false;

// ModbusMaster myModbus;           // ── MODBUS COMMENTED ──
// uint16_t AI[400];                // ── MODBUS COMMENTED ──
// String   DO[4];                  // ── MODBUS COMMENTED ──
// bool     bitValue;               // ── MODBUS COMMENTED ──
// int      slaveAddress;           // ── MODBUS COMMENTED ──
// int      functionCode;           // ── MODBUS COMMENTED ──
// int      slaveAddresses[32];     // ── MODBUS COMMENTED ──
// int      currentDataType = 0;    // ── MODBUS COMMENTED ──
// int      currentStartReg = 0;    // ── MODBUS COMMENTED ──

const char* ssid     = "UPPER_TANK";
const char* password = "123456789";

String postData;
String currentTime;
String io_data;
String submitBuffer = "";

int DI1 = 0, DI2 = 0, DI3 = 0, DI4 = 0, DI5 = 0, DI6 = 0, DI7 = 0, DI8 = 0;
int DI9 = 0, DI10 = 0, DI11 = 0, DI12 = 0, DI13 = 0, DI14 = 0, DI15 = 0, DI16 = 0;
int DO1 = 0, DO2 = 0, DO3 = 0, DO4 = 0;
int AI1 = 0, AI2 = 0, AI3 = 0, AI4 = 0;
int AI5 = 0, AI6 = 0, AI7 = 0, AI8 = 0;

IPAddress local_ip(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

AsyncWebServer server(80);
WiFiClient     espClient;
PubSubClient   client(espClient);
Preferences    prefs;
int            current_hrs;

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
  int  epmqtt_port;
 // String mqtt_server;
  bool use_4g;
};
MqttConfig mqttConfig;

// ── SlaveConfig struct kept for handleFormSubmit/load endpoint compatibility ──
struct SlaveConfig {
  char epName[20];
  int  epslaveid;
  int  epfuncode;
  int  epdatatype;
  int  eprow;
  int  epReg;
};
SlaveConfig retrievslave[100];

struct rs485Config {
  String baudrate, databits, parity, stopBits;
};
rs485Config retrieveRS485;

struct ionameconfig {
  String epdi1name, epdi2name, epdi3name, epdi4name,
         epdi5name, epdi6name, epdi7name, epdi8name,
         epdi9name, epdi10name, epdi11name, epdi12name,
         epdi13name, epdi14name, epdi15name, epdi16name;
};
ionameconfig retrieveioname;

// struct WriteCmd {                // ── MODBUS COMMENTED ──
//   bool     pending      = false; // ── MODBUS COMMENTED ──
//   int      slaveAddress = 0;     // ── MODBUS COMMENTED ──
//   uint16_t reg          = 0;     // ── MODBUS COMMENTED ──
//   uint16_t value        = 0;     // ── MODBUS COMMENTED ──
//   int      functionCode = 5;     // ── MODBUS COMMENTED ──
// };                               // ── MODBUS COMMENTED ──
// volatile WriteCmd pendingWrite;  // ── MODBUS COMMENTED ──

struct dodata {
  String epdodata;
};
dodata retrievdo;

// String fileName;                 // ── SD COMMENTED ──
// String currentfile;              // ── SD COMMENTED ──
// unsigned long maxFileSize = 50000; // ── SD COMMENTED ──
// File   dataFile;                 // ── SD COMMENTED ──

// ─────────────────── Forward declarations ───────────────────
// void put_slaveconfi(int, String, int, int, int, int, int);  // ── MODBUS COMMENTED ──
// void executePendingWrite();      // ── MODBUS COMMENTED ──
void get_slaveconfi(int);
void clear_slaveconfi(int);
void get_wifiCred();
void get_mqttconfi();
void get_IOnames();
void get_rs485Config();
// void initializeRS485();          // ── MODBUS COMMENTED ──
void sim_mqttconnect();
void re_mqttconnect();
void wifi_mqttconnect();
void connectToWiFi();
void getTimeFromNTP();
void get_time();
// void modbusmater();              // ── MODBUS COMMENTED ──
void receive_data();
void publishMessage(String, String);
// void writeToSDCard(const String&); // ── SD COMMENTED ──
// bool initSDCard();               // ── SD COMMENTED ──
// String generateFileName();       // ── SD COMMENTED ──
// void SendSD_data();              // ── SD COMMENTED ──
// bool sendDataToCloud(String);    // ── SD COMMENTED ──
// void deleteFile(fs::FS&, const char*); // ── SD COMMENTED ──
void handleFormSubmit(AsyncWebServerRequest*, uint8_t*, size_t, size_t, size_t);
// void handleWriteRegister(String); // ── MODBUS COMMENTED ──
void wifi_mqtt_callback(char*, byte*, unsigned int);
void mqtt_4g_callback(char*, byte*, unsigned int);
void sim7600ATCommand(const char*);
void parseTimeInfo(String);
void sendATCommands(const char*);
String sendATCommand(String, String);
void put_wifiCred(String, String, String, String, String);
void put_slaveconfi(int, String, int, int, int, int, int);
void put_IOnames(String, String, String, String, String, String, String, String,
                 String, String, String, String, String, String, String, String);
void put_rs485Config(String, String, String, String);
bool isSlaveConfigValid(int);
// float registersToFloat(uint16_t, uint16_t, int); // ── MODBUS COMMENTED ──
// uint32_t getSerialConfig(String, String, String); // ── MODBUS COMMENTED ──
String mergeJsonStrings(const String&, const String&);
// void update_prgs(size_t, size_t); // ── FOTA COMMENTED ──
// void printInfo();                 // ── FOTA COMMENTED ──
// void FOTA();                      // ── FOTA COMMENTED ──

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);

  pinMode(27, OUTPUT);
  pinMode(MQTT_STATUS_LED, OUTPUT);
  digitalWrite(MQTT_STATUS_LED, LOW);

  for (int i = 0; i < 100; i++) get_slaveconfi(i);

  get_wifiCred();
  if (DF <= 0) DF = 1;
  SecondTimer.setInterval(60000 * DF);

  get_mqttconfi();
  get_IOnames();
  get_rs485Config();

  // mySerial.println(retrievdo.epdodata);
  // initializeRS485();               // ── MODBUS COMMENTED ──

  pinMode(2, OUTPUT); digitalWrite(2, HIGH);
  pinMode(4, OUTPUT); digitalWrite(4, HIGH);

  delay(3000);
  sim7600Serial.begin(115200, SERIAL_8N1, RXD2, TXD2);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.println("AP started: " + String(ssid));
  delay(200);

  server.on("/cpp1", HTTP_GET, [](AsyncWebServerRequest * r) {
    r->send(200, "text/html", HTML);
  });
  server.on("/cpp2", HTTP_GET, [](AsyncWebServerRequest * r) {
    r->send(200, "text/html", HTML1);
  });
  server.on("/cpp3", HTTP_GET, [](AsyncWebServerRequest * r) {
    r->send(200, "text/html", HTML2);
  });
  server.on("/cpp4", HTTP_GET, [](AsyncWebServerRequest * r) {
    r->send(200, "text/html", HTML3);
  });
  server.on("/cpp5", HTTP_GET, [](AsyncWebServerRequest * r) {
    r->send(200, "text/html", HTML4);
  });
  server.on("/cpp6", HTTP_GET, [](AsyncWebServerRequest * r) {
    r->send(200, "text/html", HTML5);
  });

  server.on("/save", HTTP_POST, [](AsyncWebServerRequest * request) {
    String ssidsta     = request->arg("ssid");
    String passwordsta = request->arg("password");
    Device_Id  = request->arg("DeviceID");
    Data_freq  = request->arg("data_freq");
    TimeSD     = request->arg("quantity");
    put_wifiCred(ssidsta, passwordsta, Device_Id, Data_freq, TimeSD);
    get_wifiCred();
    SecondTimer.setInterval(60000 * DF);
    request->send(200, "text/html", HTML1);
  });

  server.on("/takeName", HTTP_POST, [](AsyncWebServerRequest * request) {
    put_IOnames(request->arg("nameInput0"), request->arg("nameInput1"),
                request->arg("nameInput2"), request->arg("nameInput3"),
                request->arg("nameInput4"), request->arg("nameInput5"),
                request->arg("nameInput6"), request->arg("nameInput7"),
                request->arg("nameInput8"), request->arg("nameInput9"),
                request->arg("nameInput10"), request->arg("nameInput11"),
                request->arg("nameInput12"), request->arg("nameInput13"),
                request->arg("nameInput14"), request->arg("nameInput15"));
    get_IOnames();
    SecondTimer.setInterval(60000 * DF);
    request->send(200, "text/html", HTML1);
  });

  server.on("/rs485-config", HTTP_POST, [](AsyncWebServerRequest * request) {
    String baud = request->arg("Baud Rate");
    String db   = request->arg("Data bits");
    String par  = request->arg("Parity");
    String sb   = request->arg("Stop Bits");
    put_rs485Config(baud, db, par, sb);
    get_rs485Config();
    // initializeRS485();             // ── MODBUS COMMENTED ──
    request->send(200, "text/html", HTML2);
  });

  server.on("/cluod", HTTP_POST, [](AsyncWebServerRequest * request) {
    strcpy(mqttConfig.broker,    request->arg("Host IP or Domain").c_str());
    strcpy(mqttConfig.pubtopic,  request->arg("Publish Topic").c_str());
    strcpy(mqttConfig.pubtopic1, request->arg("Publish Topic_sd").c_str());
    strcpy(mqttConfig.subtopic,  request->arg("Subscribe Topic").c_str());
    strcpy(mqttConfig.username,  request->arg("User Name").c_str());
    strcpy(mqttConfig.password,  request->arg("Password").c_str());
    mqttConfig.epmqtt_port = request->arg("Port").toInt();
   // mqttConfig.mqtt_server = request->arg("Host IP or Domain");
    String nm = request->arg("network_mode");
    mqttConfig.use_4g  = (nm == "4g");
    use_4g_network     = mqttConfig.use_4g;
    prefs.begin("mqtt", false);
    prefs.putBytes("config", &mqttConfig, sizeof(mqttConfig));
    prefs.end();
    get_mqttconfi();
    request->send(200, "text/html", HTML3);
  });

  server.onRequestBody([](AsyncWebServerRequest * request, uint8_t* data,
  size_t len, size_t index, size_t total) {
    if (request->url() == "/submit")
      handleFormSubmit(request, data, len, index, total);
  });

  server.on("/load", HTTP_GET, [](AsyncWebServerRequest * request) {
    for (int i = 0; i < 100; i++) get_slaveconfi(i);
    StaticJsonDocument<12000> data;
    data["rowCount"] = retrievslave[0].eprow;
    for (int i = 0; i < 100; i++) {
      data["NI" + String(i + 1)] = retrievslave[i].epName;
      data["SA" + String(i + 1)] = retrievslave[i].epslaveid;
      data["RG" + String(i + 1)] = retrievslave[i].epReg;
      data["FC" + String(i + 1)] = retrievslave[i].epfuncode;
      data["DT" + String(i + 1)] = retrievslave[i].epdatatype;
    }
    String resp; serializeJson(data, resp);
    request->send(200, "application/json", resp);
  });

  server.on("/values", HTTP_GET, [](AsyncWebServerRequest * request) {
    StaticJsonDocument<1000> doc1;
    doc1["doValue0"] = DO1; doc1["doValue1"] = DO2;
    doc1["doValue2"] = DO3; doc1["doValue3"] = DO4;
    doc1["diValue0"] = DI1; doc1["diValue1"] = DI2;
    doc1["diValue2"] = DI3; doc1["diValue3"] = DI4;
    doc1["aiValue0"] = AI1; doc1["aiValue1"] = AI2;
    doc1["aiValue2"] = AI3; doc1["aiValue3"] = AI4;
    doc1["aiValue4"] = AI5; doc1["aiValue5"] = AI6;
    doc1["aiValue6"] = AI7; doc1["aiValue7"] = AI8;
    String resp; serializeJson(doc1, resp);
    request->send(200, "application/json", resp);
  });

  server.on("/docontrol", HTTP_POST, [](AsyncWebServerRequest * request) {
    if (request->hasArg("cmd")) {
      String cmd = request->arg("cmd");
      Serial.println("[DO] Sending to base board: " + cmd);
      mySerial.println(cmd);
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/confi", HTTP_GET, [](AsyncWebServerRequest * request) {
    StaticJsonDocument<1000> data;
    data["ssid"]         = retrieveWifi.epSsid;
    data["DeviceID"]     = retrieveWifi.epDID;
    data["data_freq"]    = retrieveWifi.epDF;
    data["quantity"]     = retrieveWifi.epTSD;
    data["Domain"]       = mqttConfig.broker;
    data["Po"]           = mqttConfig.epmqtt_port;
    data["PT1"]          = mqttConfig.pubtopic1;
    data["ST"]           = mqttConfig.subtopic;
    data["UN"]           = mqttConfig.username;
    data["Pas"]          = mqttConfig.password;
    data["BR"]           = retrieveRS485.baudrate;
    data["Db"]           = retrieveRS485.databits;
    data["PT"]           = retrieveRS485.parity;
    data["SB"]           = retrieveRS485.stopBits;
    data["PT0"]          = mqttConfig.pubtopic;
    data["network_mode"] = mqttConfig.use_4g ? "4g" : "wifi";
    data["ioName0"] = retrieveioname.epdi1name;
    data["ioName1"] = retrieveioname.epdi2name;
    data["ioName2"] = retrieveioname.epdi3name;
    data["ioName3"] = retrieveioname.epdi4name;
    data["ioName4"] = retrieveioname.epdi5name;
    data["ioName5"] = retrieveioname.epdi6name;
    data["ioName6"] = retrieveioname.epdi7name;
    data["ioName7"] = retrieveioname.epdi8name;
    String resp; serializeJson(data, resp);
    request->send(200, "application/json", resp);
  });

  server.begin();

  // ── SD card init commented ──────────────────────────────────
  // for (int i = 0; i < 3; i++) {
  //   if (initSDCard()) {
  //     Serial.println("SD Mounted");
  //     break;
  //   } else {
  //     Serial.println("SD Mount Failed");
  //     delay(500);
  //   }
  // }
  // fileName    = generateFileName();  // ── SD COMMENTED ──
  // currentfile = fileName;            // ── SD COMMENTED ──
  // ────────────────────────────────────────────────────────────

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
      is_reconnecting = true;       // ← ADD
      wifi_mqttconnect();
      is_reconnecting = false;      // ← ADD
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

    // ── IO read & publish ──────────────────────
    _rxStarted     = false;
    _rxJson        = "";
    _rxLine        = "";
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
      // else writeToSDCard(ioBuffer);  // SD not available
      ioBuffer_ready = false;
    } else {
      Serial.println("[TIMER] No IO data");
    }
    // ──────────────────────────────────────────
  }
}
    // modbusmater();                 // ── MODBUS COMMENTED ──


  // ── SD timer block commented ─────────────────────────────────
  // if (sd_timer.isReady()) {
  //   sd_timer.reset();
  //   current_hrs = rtc.getHour(true);
  //   if (current_hrs == SDtime) {
  //     bool ready = use_4g_network ? modem.isGprsConnected()
  //                  : (WiFi.status() == WL_CONNECTED);
  //     if (ready) SendSD_data();    // ── SD COMMENTED ──
  //   }
  // }
  // ─────────────────────────────────────────────────────────────

// ═══════════════════════════════════════════════════════════════
//                      FUNCTION DEFINITIONS
// ═══════════════════════════════════════════════════════════════

// ─── MQTT callbacks ─────────────────────────────────────────
void mqtt_4g_callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) message += (char)payload[i];
  Serial.println("4G MQTT msg: " + message);
  receivedMessage = message;
  // if (message.indexOf("\"register\"") != -1 && message.indexOf("\"value\"") != -1)
  //   handleWriteRegister(message); // ── MODBUS COMMENTED ──
}

void wifi_mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) message += (char)payload[i];
  Serial.println("WiFi MQTT msg: " + message);
  receivedMessage = message;
  // if (message.indexOf("\"register\"") != -1 && message.indexOf("\"value\"") != -1)
  //   handleWriteRegister(message); // ── MODBUS COMMENTED ──
}

// ─── Network connect ────────────────────────────────────────
void connectToWiFi() {
  get_wifiCred();

  if (strlen(retrieveWifi.epSsid) == 0) {
    Serial.println("SSID EMPTY");
    return;
  }

  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect(false);  // FIX: false = don't kill AP
  delay(1000);

  Serial.println("Connecting WiFi...");
  // Serial.println("SSID: " + retrieveWifi.epSsid);

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
  get_wifiCred(); get_mqttconfi();
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

  String clientId = retrieveWifi.epDID;
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
  broker         = mqttConfig.broker;
  sub_topic      = mqttConfig.subtopic;
  pub_topic      = mqttConfig.pubtopic;
  pub_topic1     = mqttConfig.pubtopic1;
  username       = mqttConfig.username;
  pswd           = mqttConfig.password;
  port           = mqttConfig.epmqtt_port;
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
  // getTimeFromNTP();  // ← DELETE THIS LINE
  
  mqtt_wifi_client.setServer(broker, port);
  mqtt_wifi_client.setCallback(wifi_mqtt_callback);
  if (mqtt_wifi_client.connect(retrieveWifi.epDID, mqttConfig.username, mqttConfig.password)) {
    Serial.println("WiFi MQTT Connected!");
    mqtt_wifi_client.subscribe(sub_topic);
    digitalWrite(MQTT_STATUS_LED, HIGH);
  } else {
    Serial.println("WiFi MQTT failed rc=" + String(mqtt_wifi_client.state()));
    digitalWrite(MQTT_STATUS_LED, LOW);
  }
}

void publishMessage(String topic, String payload) {
  Serial.println("[PUB] Topic: " + topic);
  Serial.println("[PUB] Payload: " + payload);

  if (use_4g_network) {
    if (!mqtt_4g_client.connected()) {
      Serial.println("[MQTT] Not connected — skipping publish");
      // writeToSDCard(payload);    // ── SD COMMENTED ──
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
      // writeToSDCard(payload);    // ── SD COMMENTED ──
    }
  } else {
    if (!mqtt_wifi_client.connected()) wifi_mqttconnect();
    if (mqtt_wifi_client.connected()) {
      mqtt_wifi_client.loop();
      mqtt_wifi_client.publish(topic.c_str(), payload.c_str());
      mqtt_wifi_client.loop();
      Serial.println("WiFi Published");
    }
    // else { writeToSDCard(payload); } // ── SD COMMENTED ──
  }
}

// ── modbusmater() COMMENTED OUT ─────────────────────────────
// void modbusmater() { ... }         // ── MODBUS COMMENTED ──
// ────────────────────────────────────────────────────────────

// ── executePendingWrite() COMMENTED OUT ─────────────────────
// void executePendingWrite() { ... } // ── MODBUS COMMENTED ──
// ────────────────────────────────────────────────────────────

// ── handleWriteRegister() COMMENTED OUT ─────────────────────
// void handleWriteRegister(String payload) { ... } // ── MODBUS COMMENTED ──
// ────────────────────────────────────────────────────────────

void receive_data() {
  while (mySerial.available()) {
    char c = mySerial.read();
    if (c == '\n') {
      _rxLine.trim();

      if (_rxLine == "<JSON_START>") {
        _rxStarted = true;
        _rxJson    = "";
        _rxLine    = "";
        continue;
      }

      if (_rxLine == "<JSON_END>" && _rxStarted) {
        _rxStarted = false;

        if (_rxJson.length() == 0) {
          _rxLine = "";
          continue;
        }

        StaticJsonDocument<256> doc;
        if (deserializeJson(doc, _rxJson)) {
          Serial.println("[RX] JSON parse error!");
          _rxLine = "";
          return;
        }

        int   MOTOR = doc["MOTOR"] | 0;
        int   RAW   = doc["RAW"]   | 0;
        float LEVEL = doc["LEVEL"] | 0.0;

        StaticJsonDocument<256> doc1;
        doc1["DeviceId"]     = retrieveWifi.epDID;
        doc1["slaveAddress"] = 0;
        doc1["time"]         = currentTime;
        // doc1["MOTOR"]        = MOTOR;
        doc1["RAW_1"]          = RAW;
        doc1["LEVEL_1"]        = LEVEL;

        ioBuffer = "";
        serializeJson(doc1, ioBuffer);
        ioBuffer_ready = true;
        Serial.println("[RX] Buffered OK: " + ioBuffer);
// ──── UPPER LEVEL CHECK → LOWER TANK COMMAND ────
static bool          motorCmdSent   = false;
static unsigned long motorCmdTime   = 0;

const unsigned long MOTOR_RETRY_INTERVAL = 1UL * 60UL * 1000UL; // 2 minutes

if (RAW <= 250) {
  // First time OR retry if motor still not running after timeout
  if (!motorCmdSent || (millis() - motorCmdTime >= MOTOR_RETRY_INTERVAL)) {
    Serial.println("[UPPER] RAW LOW → MOTOR:1 (retry if needed)");
    publishMessage(sub_topic, "MOTOR:1");
    motorCmdSent = true;
    motorCmdTime = millis();   // reset retry timer
  }

} else if (RAW >= 620 && motorCmdSent) {
  Serial.println("[UPPER] RAW FULL → MOTOR:0");
  publishMessage(sub_topic, "MOTOR:0");
  motorCmdSent = false;
  motorCmdTime = 0;
}
// ─────────────────────────────────────────────────

        _rxLine = "";
        return;
      }

      if (_rxStarted) _rxJson += _rxLine;
      _rxLine = "";

    } else {
      _rxLine += c;
    }
  }
}

// ─── SD Card functions COMMENTED OUT ────────────────────────
// bool initSDCard() {
//   return SD.begin(chipSelect);      // ── SD COMMENTED ──
// }
//
// String generateFileName() {
//   static int fc = 0;
//   return "/data" + String(fc++) + ".txt"; // ── SD COMMENTED ──
// }
//
// void writeToSDCard(const String& data) {
//   ...                               // ── SD COMMENTED ──
// }
//
// void SendSD_data() {
//   ...                               // ── SD COMMENTED ──
// }
//
// bool sendDataToCloud(String data) {
//   publishMessage(pub_topic1, data);  // ── SD COMMENTED ──
//   return true;
// }
//
// void deleteFile(fs::FS& fs, const char* path) {
//   ...                               // ── SD COMMENTED ──
// }
// ────────────────────────────────────────────────────────────

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
    else                mqtt_wifi_client.loop();

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
  if (pi != -1) tzSec =  ts.substring(pi + 1).toInt() * 15 * 60;
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
  String data; unsigned long t = millis();
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
  DF    = retrieveWifi.epDF;
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

  // ← ADD defaults
  if (mqttConfig.epmqtt_port == 0) mqttConfig.epmqtt_port = 1883;
  if (strlen(mqttConfig.broker) == 0) strcpy(mqttConfig.broker, "not-set");

  broker    = mqttConfig.broker;
  sub_topic = mqttConfig.subtopic;
  pub_topic = mqttConfig.pubtopic;
  port      = mqttConfig.epmqtt_port;
  // ← ADD topic debug prints
  Serial.println("Broker:   " + String(broker));
  Serial.println("PubTopic: " + String(pub_topic));
  Serial.println("SubTopic: " + String(sub_topic));
  Serial.println("Port:     " + String(port));
}
void get_slaveconfi(int i) {
  if (i < 0 || i >= 100) return;
  String nsName = "sc" + String(i / 10);
  String key    = "s"  + String(i % 10);
  prefs.begin(nsName.c_str(), true);
  prefs.getBytes(key.c_str(), &retrievslave[i], sizeof(retrievslave[i]));
  prefs.end();
}

void clear_slaveconfi(int i) {
  if (i < 0 || i >= 100) return;
  String nsName = "sc" + String(i / 10);
  String key    = "s"  + String(i % 10);
  prefs.begin(nsName.c_str(), false);
  prefs.remove(key.c_str());
  prefs.end();
  memset(&retrievslave[i], 0, sizeof(SlaveConfig));
}

void put_slaveconfi(int idx, String name, int sid, int reg, int fc, int row, int dt) {
  if (idx < 0 || idx >= 100) return;
  SlaveConfig s; memset(&s, 0, sizeof(s));
  strncpy(s.epName, name.c_str(), 19); s.epName[19] = '\0';
  s.epslaveid = sid; s.epReg = reg; s.epfuncode = fc; s.epdatatype = dt; s.eprow = row;
  String nsName = "sc" + String(idx / 10);
  String key    = "s"  + String(idx % 10);
  prefs.begin(nsName.c_str(), false);
  prefs.putBytes(key.c_str(), &s, sizeof(s));
  prefs.end();
}

void get_rs485Config() {
  prefs.begin("Settings"); prefs.getBytes("rs485Config", &retrieveRS485, sizeof(retrieveRS485)); prefs.end();
}

void put_rs485Config(String b, String d, String p, String s) {
  rs485Config e = {b, d, p, s};
  prefs.begin("Settings"); prefs.putBytes("rs485Config", &e, sizeof(e)); prefs.end();
}

void get_IOnames() {
  prefs.begin("takename"); prefs.getBytes("takename", &retrieveioname, sizeof(retrieveioname)); prefs.end();
}

void put_IOnames(String a, String b, String c, String d, String e, String f, String g, String h,
                 String i, String j, String k, String l, String m, String n, String o, String p) {
  ionameconfig x = {a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p};
  prefs.begin("takename"); prefs.putBytes("takename", &x, sizeof(x)); prefs.end();
}

// ─── RS485 config (kept for /rs485-config endpoint) ─────────
// uint32_t getSerialConfig() and initializeRS485() COMMENTED OUT
// void initializeRS485() { ... }     // ── MODBUS COMMENTED ──
// uint32_t getSerialConfig(...) { ... } // ── MODBUS COMMENTED ──

bool isSlaveConfigValid(int i) {
  if (i < 0 || i >= 100) return false;
  SlaveConfig& s = retrievslave[i];
  if (s.epslaveid < 1 || s.epslaveid > 247) return false;
  if (s.epfuncode < 1 || s.epfuncode > 4)   return false;
  if (s.epReg < 0 || s.epReg > 65535)       return false;
  if (s.epdatatype < 0 || s.epdatatype > 4) s.epdatatype = 0;
  return true;
}

// ── registersToFloat() COMMENTED OUT ────────────────────────
// float registersToFloat(uint16_t r1, uint16_t r2, int order) {
//   ...                               // ── MODBUS COMMENTED ──
// }
// ────────────────────────────────────────────────────────────

void handleFormSubmit(AsyncWebServerRequest* request, uint8_t* data,
                      size_t len, size_t index, size_t total) {
  if (index == 0) submitBuffer = "";
  for (size_t i = 0; i < len; i++) submitBuffer += (char)data[i];
  if (index + len < total) return;

  if (submitBuffer.length() == 0) {
    request->send(400, "text/plain", "Invalid");
    return;
  }

  DynamicJsonDocument doc(32768);
  if (deserializeJson(doc, submitBuffer)) {
    request->send(400, "text/plain", "Bad JSON");
    submitBuffer = "";
    return;
  }

  int rowCount = doc["rowCount"] | 0;
  for (int i = 1; i <= rowCount && i <= 100; i++) {
    const char* name = doc[("name"         + String(i)).c_str()];
    const char* sa   = doc[("slaveAddress" + String(i)).c_str()];
    const char* rg   = doc[("register"     + String(i)).c_str()];
    const char* fc   = doc[("functionCode" + String(i)).c_str()];
    const char* dt   = doc[("datatype"     + String(i)).c_str()];
    if (!name || !sa || !rg || !fc) continue;

    SlaveConfig slave; memset(&slave, 0, sizeof(slave));
    strncpy(slave.epName, name, 19); slave.epName[19] = '\0';
    slave.epslaveid  = String(sa).toInt();
    slave.epReg      = String(rg).toInt();
    slave.epfuncode  = String(fc).toInt();
    slave.epdatatype = dt ? String(dt).toInt() : 0;
    slave.eprow      = rowCount;

    int    idx    = i - 1;
    String nsName = "sc" + String(idx / 10);
    String key    = "s"  + String(idx % 10);
    prefs.begin(nsName.c_str(), false);
    prefs.putBytes(key.c_str(), &slave, sizeof(slave));
    prefs.end();
  }

  for (int i = rowCount; i < 100; i++) clear_slaveconfi(i);
  for (int i = 0; i < 100; i++) get_slaveconfi(i);
  submitBuffer = "";
  request->send(200, "text/plain", "Saved");
}

String mergeJsonStrings(const String& j1, const String& j2) {
  if (j2.isEmpty() || j2 == "{}") return j1;
  if (j1.isEmpty() || j1 == "{}") return j2;
  String m = j1; int lb = m.lastIndexOf('}'); if (lb == -1) return j1;
  int fb = j2.indexOf('{'); if (fb == -1) return j1;
  String tail = j2.substring(fb + 1); tail.trim();
  if (tail == "}") return j1;
  return m.substring(0, lb) + ", " + tail;
}

// ── FOTA functions COMMENTED OUT ────────────────────────────
// void printInfo() { ... }           // ── FOTA COMMENTED ──
// void update_prgs(size_t i, size_t total) { ... } // ── FOTA COMMENTED ──
// void FOTA() { ... }                // ── FOTA COMMENTED ──
// ────────────────────────────────────────────────────────────
