/*
 * Task 4: web dashboard
 *
 * Goal: the board serves a web page to your phone. The page shows the
 * board's status every second and has buttons that control the LED
 * (on/off and blink patterns).
 *
 * What to look for: Serial Monitor prints the URL. Join the Wi-Fi network
 * named BOARD_NAME on your phone, open http://192.168.4.1/ and tap "LED on".
 *
 * Lines to edit: BOARD_NAME in config.h. Then look for the two
 * "add your own" comments to add a status field and a command.
 *
 * Concepts: HTTP routes (URL -> handler function), JSON as the shared
 * language between C++ and JavaScript, a loop with no delay() that runs a
 * pattern state machine from millis().
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "config.h"
#include "page.h"             // PAGE_HTML lives there (it is long)

const int           WIFI_CHANNEL   = 1;      // fixed channel so Task 5 can join in
const unsigned long BLINK_MS       = 500;
const unsigned long DEBOUNCE_MS    = 30;
const unsigned long STA_TIMEOUT_MS = 15000;

// ----- LED patterns -------------------------------------------------------
enum Pattern { OFF, ON, BLINK, SOS };
const char* PATTERN_NAMES[] = { "off", "on", "blink", "sos" };

// SOS = ... --- ...  Each step: LED on/off + how long to hold it.
struct Step { bool on; unsigned long ms; };
const unsigned long DOT = 150, DASH = 450, GAP = 150, LETTER_GAP = 450, WORD_GAP = 1500;
const Step SOS_STEPS[] = {
  {true, DOT}, {false, GAP}, {true, DOT}, {false, GAP}, {true, DOT}, {false, LETTER_GAP},     // S
  {true, DASH}, {false, GAP}, {true, DASH}, {false, GAP}, {true, DASH}, {false, LETTER_GAP},  // O
  {true, DOT}, {false, GAP}, {true, DOT}, {false, GAP}, {true, DOT}, {false, WORD_GAP},       // S
};
const int SOS_STEP_COUNT = sizeof(SOS_STEPS) / sizeof(SOS_STEPS[0]);

// ----- state --------------------------------------------------------------
WebServer server(80);              // the web server, listening on the normal HTTP port
Pattern       pattern       = BLINK;
bool          ledOn         = false;
unsigned long lastBlinkAt   = 0;
int           sosStep       = 0;
unsigned long sosStepAt     = 0;
int           lastButtonReading = HIGH, buttonState = HIGH;
unsigned long lastButtonChangeAt = 0;

// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(300);
  touchRead(TOUCH_PIN);        // first touch reading is garbage while the sensor calibrates; discard it
  printBanner();
#if !LED_IS_RGB
  pinMode(LED_PIN, OUTPUT);   // plain LED only; rgbLedWrite() sets its own pin up
#endif
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  startWifi();           // Wi-Fi AP (or join a hotspot)
  startServer();         // web server routes
  startNameService();    // http://<board-name>.local/
}

void loop() {
  server.handleClient();   // answer any waiting phone request (fast, does not block)
  runPattern();            // keep the LED moving
  checkButton();           // BOOT button still toggles the LED
}

// ---------------------------------------------------------------------------
// Wi-Fi
// ---------------------------------------------------------------------------
void startWifi() {
#if USE_STATION_MODE
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Joining "); Serial.print(WIFI_SSID);
  unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startedAt < STA_TIMEOUT_MS) {
    delay(250); Serial.print('.');
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Page: http://"); Serial.print(WiFi.localIP()); Serial.println("/");
    return;
  }
  Serial.println("Could not join hotspot, starting our own access point instead.");
#endif
  WiFi.mode(WIFI_AP);
  WiFi.softAP(BOARD_NAME, AP_PASSWORD, WIFI_CHANNEL);
  Serial.print("AP started  SSID: "); Serial.print(BOARD_NAME);
  Serial.print("  IP: "); Serial.println(WiFi.softAPIP());
  Serial.print("password: "); Serial.println(AP_PASSWORD);
  Serial.print("Page: http://"); Serial.print(WiFi.softAPIP()); Serial.println("/");
}

void startNameService() {
  String host = String(BOARD_NAME);
  host.toLowerCase();
  if (MDNS.begin(host.c_str())) {
    MDNS.addService("http", "tcp", 80);
    Serial.print("Also try: http://"); Serial.print(host); Serial.println(".local/");
  }
}

// ---------------------------------------------------------------------------
// Web server: every URL maps to one small handler function.
// ---------------------------------------------------------------------------
void startServer() {
  server.on("/",            HTTP_GET, handlePage);
  server.on("/api/ping",    HTTP_GET, handlePing);
  server.on("/api/status",  HTTP_GET, handleStatus);
  server.on("/api/led",     HTTP_GET, handleLedCommand);
  server.on("/api/pattern", HTTP_GET, handlePatternCommand);
  // ---- add your own command here ----
  // for example: server.on("/api/blink", HTTP_GET, handleBlink);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Web server listening on port 80");
}

void handlePage() {
  server.send_P(200, "text/html", PAGE_HTML);   // _P = read straight from flash
}

void handlePing() {
  sendJson(200, "{\"ok\":true}");
}

void handleStatus() {
  sendJson(200, buildStatusJson());
}

// /api/led?state=on|off|toggle
void handleLedCommand() {
  String state = server.arg("state");
  if      (state == "on")     setLedEnabled(true);
  else if (state == "off")    setLedEnabled(false);
  else if (state == "toggle") setLedEnabled(pattern == OFF);
  else { sendJson(400, "{\"ok\":false,\"error\":\"state must be on, off or toggle\"}"); return; }
  Serial.print("CMD led "); Serial.println(state);
  sendJson(200, buildStatusJson());
}

// /api/pattern?name=off|on|blink|sos
void handlePatternCommand() {
  String name = server.arg("name");
  for (int i = 0; i < 4; i++) {
    if (name == PATTERN_NAMES[i]) {
      setPattern((Pattern)i);
      Serial.print("CMD pattern "); Serial.println(name);
      sendJson(200, buildStatusJson());
      return;
    }
  }
  sendJson(400, "{\"ok\":false,\"error\":\"unknown pattern\"}");
}

void handleNotFound() {
  sendJson(404, "{\"ok\":false,\"error\":\"no such route\"}");
}

// JSON must never be cached by the phone, or it would show stale numbers.
void sendJson(int code, const String& body) {
  server.sendHeader("Cache-Control", "no-store");
  server.send(code, "application/json", body);
}

// Build the JSON by hand: {"key":value,...}. Strings get quotes, numbers don't.
String buildStatusJson() {
  String json = "{\"ok\":true";
  json += ",\"name\":\"" + String(BOARD_NAME) + "\"";
  json += ",\"uptime_s\":" + String(millis() / 1000.0, 1);
  json += ",\"temp_c\":"   + String(temperatureRead(), 1);
  json += ",\"touch\":"    + String(touchRead(TOUCH_PIN));
  json += ",\"button\":"   + String(buttonState == LOW ? 1 : 0);
  json += ",\"led\":"      + String(ledOn ? 1 : 0);
  json += ",\"pattern\":\"" + String(PATTERN_NAMES[pattern]) + "\"";
#if USE_STATION_MODE
  json += ",\"clients\":1,\"rssi\":" + String(WiFi.RSSI());
#else
  json += ",\"clients\":" + String(WiFi.softAPgetStationNum()) + ",\"rssi\":null";
#endif
  json += ",\"heap_free\":" + String(ESP.getFreeHeap());
  json += ",\"chip\":\""    + String(ESP.getChipModel()) + "\"";
  // ---- add your own status field here ----
  // for example: json += ",\"adc34\":" + String(analogRead(34));
  json += "}";
  return json;
}

// ---------------------------------------------------------------------------
// LED pattern state machine: called thousands of times per second, never blocks.
// ---------------------------------------------------------------------------
void runPattern() {
  unsigned long now = millis();
  switch (pattern) {
    case OFF: setLed(false); break;
    case ON:  setLed(true);  break;
    case BLINK:
      if (now - lastBlinkAt >= BLINK_MS) { lastBlinkAt = now; setLed(!ledOn); }
      break;
    case SOS:
      // Hold the current step for its duration, then move to the next one.
      if (now - sosStepAt >= SOS_STEPS[sosStep].ms) {
        sosStepAt = now;
        sosStep = (sosStep + 1) % SOS_STEP_COUNT;   // wrap around to repeat
        setLed(SOS_STEPS[sosStep].on);
      }
      break;
  }
}

void setPattern(Pattern p) {
  pattern = p;
  sosStep = 0;                 // restart SOS from the first dot
  sosStepAt = millis();
  lastBlinkAt = millis();
  if (p == SOS) setLed(SOS_STEPS[0].on);
  if (p == OFF) setLed(false);   // reflect the change right away, not on the next blink tick
}

// "LED on" means: if it was off, start blinking. Any active pattern counts as on.
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

// Same edge + debounce logic as Task 2.
void checkButton() {
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonReading) { lastButtonChangeAt = millis(); lastButtonReading = reading; }
  if (millis() - lastButtonChangeAt >= DEBOUNCE_MS && reading != buttonState) {
    buttonState = reading;
    if (buttonState == LOW) {
      Serial.println("EVENT button pressed");
      setLedEnabled(pattern == OFF);
    }
  }
}

void printBanner() {
  Serial.println();
  Serial.print("["); Serial.print(BOARD_NAME); Serial.println("] Task 4: web dashboard");
}
