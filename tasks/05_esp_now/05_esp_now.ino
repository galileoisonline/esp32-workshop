/*
 * Task 5: esp-now
 *
 * Goal: boards talk to each other. Every 2 s your board sends a small
 * status message over ESP-NOW (radio to radio, no Wi-Fi network needed).
 * Every message it hears goes into a table of nearby boards, printed to
 * Serial and shown on the web page under "Boards nearby".
 *
 * What to look for: Serial prints "boards nearby: N" with a table.
 * The page at http://192.168.4.1/ lists them.
 *
 * Line to edit: BOARD_NAME in config.h (must be different on every board).
 *
 * Concepts: ESP-NOW broadcast, packing a struct into bytes, receive
 * callbacks, a peer table with expiry, AP + ESP-NOW at the same time
 * (both must use the same Wi-Fi channel).
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "config.h"
#include "page.h"

const int           WIFI_CHANNEL   = 1;      // every board must use the same channel
const unsigned long BLINK_MS       = 500;
const unsigned long DEBOUNCE_MS    = 30;
const unsigned long BROADCAST_MS   = 2000;   // how often we send our status
const unsigned long PEER_EXPIRE_MS = 10000;  // forget a board after 10 s of silence
const unsigned long TABLE_PRINT_MS = 5000;
const int           MAX_PEERS      = 16;

// ----- the message every board broadcasts (must be identical everywhere) -----
struct Message {
  char     name[12];   // 11 chars + terminating zero
  uint32_t uptime_s;
  float    temp_c;
  uint8_t  led;
};
// The compiler pads the struct to 24 bytes. ESP-NOW allows at most 250.
static_assert(sizeof(Message) <= 250, "Message must fit in one ESP-NOW packet");

struct Peer {
  Message       last;        // most recent message from this board
  int           rssi;        // signal strength of that message (dBm)
  unsigned long heardAt;     // millis() when it arrived; 0 = empty slot
};
Peer peers[MAX_PEERS];
const uint8_t BROADCAST_ADDR[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

// ----- LED patterns (same as Task 4) ---------------------------------------
enum Pattern { OFF, ON, BLINK, SOS };
const char* PATTERN_NAMES[] = { "off", "on", "blink", "sos" };
struct Step { bool on; unsigned long ms; };
const unsigned long DOT = 150, DASH = 450, GAP = 150, LETTER_GAP = 450, WORD_GAP = 1500;
const Step SOS_STEPS[] = {
  {true, DOT}, {false, GAP}, {true, DOT}, {false, GAP}, {true, DOT}, {false, LETTER_GAP},
  {true, DASH}, {false, GAP}, {true, DASH}, {false, GAP}, {true, DASH}, {false, LETTER_GAP},
  {true, DOT}, {false, GAP}, {true, DOT}, {false, GAP}, {true, DOT}, {false, WORD_GAP},
};
const int SOS_STEP_COUNT = sizeof(SOS_STEPS) / sizeof(SOS_STEPS[0]);

WebServer server(80);
Pattern       pattern = BLINK;
bool          ledOn = false;
unsigned long lastBlinkAt = 0, sosStepAt = 0, lastBroadcastAt = 0, lastTableAt = 0;
int           sosStep = 0;
int           lastButtonReading = HIGH, buttonState = HIGH;
unsigned long lastButtonChangeAt = 0;

// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(300);
  printBanner();
#if !LED_IS_RGB
  pinMode(LED_PIN, OUTPUT);   // plain LED only; rgbLedWrite() sets its own pin up
#endif
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  startWifi();
  startEspNow();
  startServer();
}

void loop() {
  server.handleClient();
  runPattern();
  checkButton();
  broadcastStatus();
  printPeers();
}

// ---------------------------------------------------------------------------
// Wi-Fi: AP for the phone + STA interface for ESP-NOW, both on WIFI_CHANNEL.
// ---------------------------------------------------------------------------
void startWifi() {
  WiFi.mode(WIFI_AP_STA);                               // both roles at once
  WiFi.softAP(BOARD_NAME, AP_PASSWORD, WIFI_CHANNEL);   // explicit channel so both can share it
  Serial.print("AP started  SSID: "); Serial.print(BOARD_NAME);
  Serial.print("  IP: "); Serial.println(WiFi.softAPIP());
  Serial.print("password: "); Serial.println(AP_PASSWORD);
  Serial.print("Page: http://"); Serial.print(WiFi.softAPIP()); Serial.println("/");
  String host = String(BOARD_NAME);
  host.toLowerCase();
  if (MDNS.begin(host.c_str())) MDNS.addService("http", "tcp", 80);
}

void startEspNow() {
  if (esp_now_init() != ESP_OK) { Serial.println("ESP-NOW init failed"); return; }
  esp_now_register_recv_cb(onMessageReceived);

  // Register the broadcast address as a "peer" so we are allowed to send to it.
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, BROADCAST_ADDR, 6);
  peer.channel = 0;              // 0 = use the current Wi-Fi channel
  peer.encrypt = false;          // broadcast cannot be encrypted
  peer.ifidx   = WIFI_IF_STA;    // send from the station interface
  if (esp_now_add_peer(&peer) != ESP_OK) Serial.println("could not add broadcast peer");
  Serial.print("ESP-NOW ready, my MAC: "); Serial.println(WiFi.macAddress());
}

// Every BROADCAST_MS: fill a Message with our status and send it to everyone.
void broadcastStatus() {
  unsigned long now = millis();
  if (now - lastBroadcastAt < BROADCAST_MS) return;
  lastBroadcastAt = now;

  Message m = {};
  strncpy(m.name, BOARD_NAME, sizeof(m.name) - 1);   // -1 keeps the terminating zero
  m.uptime_s = now / 1000;
  m.temp_c   = temperatureRead();
  m.led      = ledOn ? 1 : 0;
  esp_err_t err = esp_now_send(BROADCAST_ADDR, (const uint8_t*)&m, sizeof(m));
  if (err != ESP_OK) Serial.println("broadcast failed");
}

// Runs automatically whenever a message arrives. Keep it short: just store it.
void onMessageReceived(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len != sizeof(Message)) return;           // not one of ours (different struct)
  Message m;
  memcpy(&m, data, sizeof(m));
  m.name[sizeof(m.name) - 1] = '\0';            // never trust a string from the radio

  int slot = findPeerSlot(m.name);
  if (slot < 0) return;                         // table full
  peers[slot].last    = m;
  peers[slot].heardAt = millis();
  peers[slot].rssi    = (info->rx_ctrl != NULL) ? info->rx_ctrl->rssi : 0;
}

// Existing entry with this name, otherwise the first empty/expired slot.
int findPeerSlot(const char* name) {
  for (int i = 0; i < MAX_PEERS; i++)
    if (peers[i].heardAt != 0 && strcmp(peers[i].last.name, name) == 0) return i;
  for (int i = 0; i < MAX_PEERS; i++)
    if (!peerAlive(i)) return i;
  return -1;
}

bool peerAlive(int i) {
  return peers[i].heardAt != 0 && millis() - peers[i].heardAt < PEER_EXPIRE_MS;
}

void printPeers() {
  unsigned long now = millis();
  if (now - lastTableAt < TABLE_PRINT_MS) return;
  lastTableAt = now;
  int count = 0;
  for (int i = 0; i < MAX_PEERS; i++) if (peerAlive(i)) count++;
  Serial.print("boards nearby: "); Serial.println(count);
  for (int i = 0; i < MAX_PEERS; i++) {
    if (!peerAlive(i)) continue;
    Serial.print("  "); Serial.print(peers[i].last.name);
    Serial.print("  age="); Serial.print((now - peers[i].heardAt) / 1000); Serial.print("s");
    Serial.print("  temp="); Serial.print(peers[i].last.temp_c, 1);
    Serial.print("C  led="); Serial.print(peers[i].last.led);
    Serial.print("  rssi="); Serial.println(peers[i].rssi);
  }
}

// [{"name":"ESP32-2","age_s":1,"temp_c":42.1,"led":1,"rssi":-55}, ...]
String buildPeersJson() {
  String json = "[";
  bool first = true;
  for (int i = 0; i < MAX_PEERS; i++) {
    if (!peerAlive(i)) continue;
    if (!first) json += ",";
    first = false;
    json += "{\"name\":\"" + String(peers[i].last.name) + "\"";
    json += ",\"age_s\":"   + String((millis() - peers[i].heardAt) / 1000);
    json += ",\"uptime_s\":" + String(peers[i].last.uptime_s);
    json += ",\"temp_c\":"  + String(peers[i].last.temp_c, 1);
    json += ",\"led\":"     + String(peers[i].last.led);
    json += ",\"rssi\":"    + String(peers[i].rssi) + "}";
  }
  return json + "]";
  // ---- your code: add a field to Message and expose it here (every board must match) ----
}

// ---------------------------------------------------------------------------
// Web server (same routes as Task 4, plus /api/peers)
// ---------------------------------------------------------------------------
void startServer() {
  server.on("/",            HTTP_GET, handlePage);
  server.on("/api/ping",    HTTP_GET, handlePing);
  server.on("/api/status",  HTTP_GET, handleStatus);
  server.on("/api/peers",   HTTP_GET, handlePeers);
  server.on("/api/led",     HTTP_GET, handleLedCommand);
  server.on("/api/pattern", HTTP_GET, handlePatternCommand);
  server.onNotFound(handleNotFound);
  server.begin();
}

void handlePage()     { server.send_P(200, "text/html", PAGE_HTML); }
void handlePing()     { sendJson(200, "{\"ok\":true}"); }
void handleStatus()   { sendJson(200, buildStatusJson()); }
void handlePeers()    { sendJson(200, buildPeersJson()); }
void handleNotFound() { sendJson(404, "{\"ok\":false,\"error\":\"no such route\"}"); }

void handleLedCommand() {
  String state = server.arg("state");
  if      (state == "on")     setLedEnabled(true);
  else if (state == "off")    setLedEnabled(false);
  else if (state == "toggle") setLedEnabled(pattern == OFF);
  else { sendJson(400, "{\"ok\":false,\"error\":\"state must be on, off or toggle\"}"); return; }
  sendJson(200, buildStatusJson());
}

void handlePatternCommand() {
  String name = server.arg("name");
  for (int i = 0; i < 4; i++) {
    if (name == PATTERN_NAMES[i]) { setPattern((Pattern)i); sendJson(200, buildStatusJson()); return; }
  }
  sendJson(400, "{\"ok\":false,\"error\":\"unknown pattern\"}");
}

void sendJson(int code, const String& body) {
  server.sendHeader("Cache-Control", "no-store");
  server.send(code, "application/json", body);
}

String buildStatusJson() {
  int alive = 0;
  for (int i = 0; i < MAX_PEERS; i++) if (peerAlive(i)) alive++;
  String json = "{\"ok\":true";
  json += ",\"name\":\"" + String(BOARD_NAME) + "\"";
  json += ",\"uptime_s\":" + String(millis() / 1000.0, 1);
  json += ",\"temp_c\":"   + String(temperatureRead(), 1);
  json += ",\"touch\":"    + String(touchRead(TOUCH_PIN));
  json += ",\"button\":"   + String(buttonState == LOW ? 1 : 0);
  json += ",\"led\":"      + String(ledOn ? 1 : 0);
  json += ",\"pattern\":\"" + String(PATTERN_NAMES[pattern]) + "\"";
  json += ",\"clients\":"  + String(WiFi.softAPgetStationNum()) + ",\"rssi\":null";
  json += ",\"peers\":"    + String(alive);
  json += ",\"heap_free\":" + String(ESP.getFreeHeap());
  json += ",\"chip\":\""    + String(ESP.getChipModel()) + "\"}";
  return json;
}

// ---------------------------------------------------------------------------
// LED pattern state machine + button (identical to Task 4)
// ---------------------------------------------------------------------------
void runPattern() {
  unsigned long now = millis();
  switch (pattern) {
    case OFF:   setLed(false); break;
    case ON:    setLed(true);  break;
    case BLINK: if (now - lastBlinkAt >= BLINK_MS) { lastBlinkAt = now; setLed(!ledOn); } break;
    case SOS:
      if (now - sosStepAt >= SOS_STEPS[sosStep].ms) {
        sosStepAt = now;
        sosStep = (sosStep + 1) % SOS_STEP_COUNT;
        setLed(SOS_STEPS[sosStep].on);
      }
      break;
  }
}

void setPattern(Pattern p) {
  pattern = p; sosStep = 0; sosStepAt = millis(); lastBlinkAt = millis();
  if (p == SOS) setLed(SOS_STEPS[0].on);
}
void setLedEnabled(bool on) {
  if (on && pattern == OFF) setPattern(BLINK);
  if (!on) setPattern(OFF);
}
void setLed(bool on) {
  ledOn = on;
#if LED_IS_RGB
  uint8_t v = on ? LED_BRIGHTNESS : 0;   // addressable RGB LED: equal r,g,b = white
  rgbLedWrite(LED_PIN, v, v, v);
#else
  digitalWrite(LED_PIN, on ? HIGH : LOW);
#endif
}

void checkButton() {
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonReading) { lastButtonChangeAt = millis(); lastButtonReading = reading; }
  if (millis() - lastButtonChangeAt >= DEBOUNCE_MS && reading != buttonState) {
    buttonState = reading;
    if (buttonState == LOW) { Serial.println("EVENT button pressed"); setLedEnabled(pattern == OFF); }
  }
}

void printBanner() {
  Serial.println();
  Serial.print("["); Serial.print(BOARD_NAME); Serial.println("] Task 5: esp-now");
}
