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

HardwareSerial touchTX(1);   // GPIO18
HardwareSerial touchRX(2);   // GPIO16

// =====================================================
// WIFI
// =====================================================

const char* ssid     = "RND_TESTING";
const char* password = "White@4321";

// =====================================================
// SERVER
// =====================================================

// Use Render URL or local test server URL.
String serverURL = "https://uart-sniffer.onrender.com/log";
// String serverURL = "http://192.168.68.112:5000/log"; // Local test

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

// =====================================================
// BUFFERS
// =====================================================

uint8_t buf18[MAX_PACKET];
int len18 = 0;
unsigned long last18 = 0;

uint8_t buf16[MAX_PACKET];
int len16 = 0;
unsigned long last16 = 0;

// =====================================================
// FUNCTIONS
// =====================================================

String getHexString(const uint8_t* buf, int len) {
  String s = "";
  for (int i = 0; i < len; i++) {
    char temp[5];
    sprintf(temp, "%02X ", buf[i]);
    s += temp;
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

void uploadPacket(const String& packetData) {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.setTimeout(UPLOAD_TIMEOUT_MS);

  Serial.println("Uploading...");

  if (!http.begin(client, serverURL)) {
    Serial.println("HTTP Begin Failed");
    return;
  }

  http.addHeader("Content-Type", "application/json");

  String payload = "{";
  payload += "\"chip\":\"ESP32S3\",";
  payload += "\"time\":\"" + getTimestamp() + "\",";
  payload += "\"log\":\"" + escapeJson(packetData) + "\"";
  payload += "}";

  Serial.println(payload);

  int code = http.POST(payload);
  Serial.print("HTTP Response: ");
  Serial.println(code);

  if (code > 0) {
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.print("Upload Failed: ");
    Serial.println(http.errorToString(code));
  }

  http.end();
}

void logToCSV(const String& timestamp,
              const String& gpio,
              const String& checksum,
              const String& packetData) {
  String line = timestamp + "," + gpio + "," + checksum + ",\"" + packetData + "\"";
  logFile.println(line);
  logFile.flush();
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

  // Dedup + upload throttling: only upload when checksum OK and
  // the same packet hasn't been uploaded recently for the same GPIO.
  if (csOK) {
    bool isDuplicate = false;

    if (String(gpio) == "GPIO18") {
      if (len == lastLen18 && lastLen18 > 0 && memcmp(buf, lastBuf18, len) == 0 && (millis() - lastPrint18) < DEDUP_WINDOW_MS) {
        isDuplicate = true;
      } else {
        // update last seen
        memcpy(lastBuf18, buf, len);
        lastLen18 = len;
        lastPrint18 = millis();
      }
    } else if (String(gpio) == "GPIO16") {
      if (len == lastLen16 && lastLen16 > 0 && memcmp(buf, lastBuf16, len) == 0 && (millis() - lastPrint16) < DEDUP_WINDOW_MS) {
        isDuplicate = true;
      } else {
        // update last seen
        memcpy(lastBuf16, buf, len);
        lastLen16 = len;
        lastPrint16 = millis();
      }
    }

    if (isDuplicate) {
      Serial.println("Duplicate packet within dedup window — skipping upload.");
    } else {
      String uploadData = packetData;
      if (uploadData.length() > MAX_UPLOAD_LEN) {
        uploadData = uploadData.substring(0, MAX_UPLOAD_LEN) + "...";
      }
      uploadPacket(uploadData);
    }
  } else {
    Serial.println("Skipping upload due to checksum failure.");
  }
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

  touchTX.begin(9600, SERIAL_8N1, 18, -1);
  touchRX.begin(9600, SERIAL_8N1, 16, -1);

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

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();
  Serial.println("WiFi Connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  configTime(19800, 0, "pool.ntp.org", "time.nist.gov");
  struct tm timeinfo;
  Serial.print("Waiting for NTP time");
  while (!getLocalTime(&timeinfo)) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();
  Serial.println("Time synchronized");

  setupWebPage();
  server.begin();
  webSocket.begin();

  Serial.println();
  Serial.println("==========================");
  Serial.println("UART LOGGER STARTED");
  Serial.println("==========================");
  Serial.print("Browser: http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();
  webSocket.loop();

  while (touchTX.available()) {
    uint8_t b = touchTX.read();
    if (len18 == 0 && b != 0xB4) continue;
    if (len18 < MAX_PACKET) buf18[len18++] = b;
    last18 = millis();
  }

  if (len18 > 0 && millis() - last18 > PACKET_TIMEOUT_MS) {
    printPacket(buf18, len18, "GPIO18");
    len18 = 0;
  }

  while (touchRX.available()) {
    uint8_t b = touchRX.read();
    if (len16 == 0 && b != 0xB4) continue;
    if (len16 < MAX_PACKET) buf16[len16++] = b;
    last16 = millis();
  }

  if (len16 > 0 && millis() - last16 > PACKET_TIMEOUT_MS) {
    printPacket(buf16, len16, "GPIO16");
    len16 = 0;
  }
}
