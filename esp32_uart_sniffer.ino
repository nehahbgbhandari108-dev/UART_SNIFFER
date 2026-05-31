#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <time.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "FS.h"
#include "LittleFS.h"

// =====================================================
// UARTS
// =====================================================

HardwareSerial uart18(1);   // UART1 for GPIO18 receive
HardwareSerial uart16(2);   // UART2 for GPIO16 receive

const int RX_PIN_18 = 18;
const int RX_PIN_16 = 16;

// =====================================================
// WIFI
// =====================================================

const char* ssid     = "RND_TESTING";
const char* password = "White@4321";

// =====================================================
// SERVER
// =====================================================

// Public Render endpoint for cross-network uploads.
const String serverURL = "https://uart-sniffer.onrender.com/log";
// const String serverURL = "http://192.168.68.112:5000/log"; // Local test
const String deviceId = "ESP32S3_N1648";

// =====================================================
// WEB SERVER
// =====================================================

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// =====================================================
// FILE
// =====================================================

File logFile;

// =====================================================
// SETTINGS
// =====================================================

#define MAX_PACKET         256
#define PACKET_TIMEOUT_MS  100
#define UPLOAD_TIMEOUT_MS  30000
#define MAX_UPLOAD_LEN     600
#define DEDUP_WINDOW_MS    0

// =====================================================
// BUFFERS
// =====================================================

uint8_t buf18[MAX_PACKET];
int len18 = 0;
unsigned long last18 = 0;

uint8_t buf16[MAX_PACKET];
int len16 = 0;
unsigned long last16 = 0;

uint8_t lastBuf18[MAX_PACKET];
int lastLen18 = 0;
unsigned long lastPrint18 = 0;

uint8_t lastBuf16[MAX_PACKET];
int lastLen16 = 0;
unsigned long lastPrint16 = 0;

// =====================================================
// FUNCTIONS
// =====================================================

String getHexString(const uint8_t* buf, int len) {
  String s;
  for (int i = 0; i < len; i++) {
    char temp[4];
    sprintf(temp, "%02X", buf[i]);
    s += temp;
    if (i < len - 1) {
      s += ' ';
    }
  }
  return s;
}

bool verifyChecksum(const uint8_t* buf, int len) {
  if (len < 4) {
    return false;
  }
  uint8_t sum = 0;
  for (int i = 2; i < len - 1; i++) {
    sum += buf[i];
  }
  return sum == buf[len - 1];
}

String escapeJson(const String& s) {
  String result;
  for (unsigned int i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '"' || c == '\\') {
      result += '\\';
      result += c;
    } else if (c == '\n') {
      result += "\\n";
    } else if (c == '\r') {
      result += "\\r";
    } else {
      result += c;
    }
  }
  return result;
}

String getTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "NO_TIME";
  }
  char ts[30];
  strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(ts);
}

bool connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 30000) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection failed");
    return false;
  }

  Serial.println("WiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  return true;
}

bool syncTime() {
  configTime(19800, 0, "pool.ntp.org", "time.nist.gov");
  struct tm timeinfo;
  Serial.print("Syncing time");
  unsigned long start = millis();
  while (!getLocalTime(&timeinfo) && millis() - start < 15000) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Time sync failed");
    return false;
  }
  Serial.println("Time synchronized");
  return true;
}

void uploadPacket(const String& packetData) {
  if (!connectWiFi()) {
    Serial.println("Skipping upload: WiFi not connected");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.setTimeout(UPLOAD_TIMEOUT_MS);

  if (!http.begin(client, serverURL)) {
    Serial.println("HTTP begin failed");
    return;
  }

  http.addHeader("Content-Type", "application/json");

  String payload = "{";
  payload += "\"chip\":\"" + deviceId + "\",";
  payload += "\"time\":\"" + getTimestamp() + "\",";
  payload += "\"log\":\"" + escapeJson(packetData) + "\"";
  payload += "}";

  Serial.println("Uploading to Render:");
  Serial.println(payload);

  int code = http.POST(payload);
  Serial.print("HTTP Response: ");
  Serial.println(code);
  if (code > 0) {
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.print("Upload failed: ");
    Serial.println(http.errorToString(code));
  }

  http.end();
}

void logToCSV(const String& timestamp, const String& gpio, const String& checksum, const String& packetData) {
  String line = timestamp + "," + gpio + "," + checksum + ",\"" + packetData + "\"";
  if (logFile) {
    logFile.println(line);
    logFile.flush();
  }
}

void sendLivePacket(String line) {
  webSocket.broadcastTXT(line);
}

void printPacket(const uint8_t* buf, int len, const char* gpio) {
  String packetData = getHexString(buf, len);
  bool csOK = verifyChecksum(buf, len);

  String ts = getTimestamp();
  if (ts == "NO_TIME") {
    ts = "UNKNOWN_TIME";
  }

  Serial.println();
  Serial.println("================================");
  Serial.print("TIME: ");
  Serial.println(ts);
  Serial.print("GPIO: ");
  Serial.println(gpio);
  Serial.print("HEX : ");
  Serial.println(packetData);
  Serial.print("CHECKSUM: ");
  Serial.println(csOK ? "OK" : "FAIL");
  Serial.println("================================");

  logToCSV(ts, gpio, csOK ? "OK" : "FAIL", packetData);

  String liveLine = "[" + ts + "] " + String(gpio) + " " + packetData;
  sendLivePacket(liveLine);

  if (!csOK) {
    Serial.println("Skipping upload due to checksum failure.");
    return;
  }

  bool isDuplicate = false;
  if (String(gpio) == "GPIO18") {
    if (len == lastLen18 && lastLen18 > 0 && memcmp(buf, lastBuf18, len) == 0 && (millis() - lastPrint18) < DEDUP_WINDOW_MS) {
      isDuplicate = true;
    } else {
      memcpy(lastBuf18, buf, len);
      lastLen18 = len;
      lastPrint18 = millis();
    }
  } else if (String(gpio) == "GPIO16") {
    if (len == lastLen16 && lastLen16 > 0 && memcmp(buf, lastBuf16, len) == 0 && (millis() - lastPrint16) < DEDUP_WINDOW_MS) {
      isDuplicate = true;
    } else {
      memcpy(lastBuf16, buf, len);
      lastLen16 = len;
      lastPrint16 = millis();
    }
  }

  if (isDuplicate) {
    Serial.println("Duplicate packet within dedup window — skipping upload.");
    return;
  }

  String uploadData = packetData;
  if (uploadData.length() > MAX_UPLOAD_LEN) {
    uploadData = uploadData.substring(0, MAX_UPLOAD_LEN) + "...";
  }
  uploadPacket(uploadData);
}

void setupWebPage() {
  server.on("/", []() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>UART LOGGER</title>
  <style>
    body { background: black; color: lime; font-family: monospace; padding: 20px; }
    #log { border: 1px solid lime; height: 500px; overflow-y: scroll; padding: 10px; white-space: pre-wrap; }
  </style>
</head>
<body>
  <h2>ESP32 UART LOGGER</h2>
  <div id="log"></div>
  <script>
    var gateway = 'ws://' + window.location.hostname + ':81/';
    var websocket = new WebSocket(gateway);
    websocket.onmessage = function(event) {
      var log = document.getElementById('log');
      log.innerHTML += event.data + '\n';
      log.scrollTop = log.scrollHeight;
    };
  </script>
</body>
</html>
)rawliteral";
    server.send(200, "text/html", html);
  });
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // =====================================================
  // PREVENT FLOATING RX PINS
  // =====================================================

  pinMode(RX_PIN_18, INPUT_PULLUP);
  pinMode(RX_PIN_16, INPUT_PULLUP);

  // =====================================================
  // UART START
  // =====================================================

  uart18.begin(9600, SERIAL_8N1, RX_PIN_18, -1);
  uart16.begin(9600, SERIAL_8N1, RX_PIN_16, -1);

  // Bigger buffers
  uart18.setRxBufferSize(1024);
  uart16.setRxBufferSize(1024);

  // =====================================================
  // LITTLEFS
  // =====================================================

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Failed");
    return;
  }

  if (!LittleFS.exists("/uartlog.csv")) {
    File file = LittleFS.open("/uartlog.csv", FILE_WRITE);
    file.println("Timestamp,GPIO,Checksum,Data");
    file.close();
  }

  logFile = LittleFS.open("/uartlog.csv", FILE_APPEND);
  if (!logFile) {
    Serial.println("Failed to open log file");
  }

  // =====================================================
  // WIFI + TIME
  // =====================================================

  connectWiFi();
  syncTime();

  // =====================================================
  // WEB
  // =====================================================

  setupWebPage();
  server.begin();
  webSocket.begin();

  // =====================================================
  // START MESSAGE
  // =====================================================

  Serial.println();
  Serial.println("==========================");
  Serial.println("UART LOGGER STARTED");
  Serial.println("==========================");

  Serial.print("Local page: http://");
  Serial.println(WiFi.localIP());

  Serial.println("Render dashboard: https://uart-sniffer.onrender.com/");
  Serial.println("Upload endpoint: https://uart-sniffer.onrender.com/log");

  Serial.println();
  Serial.println("Waiting for UART packets...");
}

void loop() {
  server.handleClient();
  webSocket.loop();

  while (uart18.available()) {
    uint8_t b = uart18.read();
    if (len18 == 0 && b != 0xB4) continue;
    Serial.printf("RX18: %02X\n", b);
    if (len18 < MAX_PACKET) buf18[len18++] = b;
    last18 = millis();
  }

  if (len18 > 0 && millis() - last18 > PACKET_TIMEOUT_MS) {
    Serial.printf("GPIO18 packet length=%d\n", len18);
    printPacket(buf18, len18, "GPIO18");
    len18 = 0;
  }

  while (uart16.available()) {
    uint8_t b = uart16.read();
    if (len16 == 0 && b != 0xB4) continue;
    Serial.printf("RX16: %02X\n", b);
    if (len16 < MAX_PACKET) buf16[len16++] = b;
    last16 = millis();
  }

  if (len16 > 0 && millis() - last16 > PACKET_TIMEOUT_MS) {
    Serial.printf("GPIO16 packet length=%d\n", len16);
    printPacket(buf16, len16, "GPIO16");
    len16 = 0;
  }
}
