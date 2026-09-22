/*
 * Task 3: wifi
 *
 * Goal: turn on the Wi-Fi radio. Phase A: list every network in the room.
 * Phase B: start the board's own Wi-Fi network, named BOARD_NAME, so a
 * phone can connect to it.
 *
 * What to look for: Serial Monitor prints a table of networks, then
 *     AP started  SSID: ESP32-1  IP: 192.168.4.1
 * Your phone's Wi-Fi list shows ESP32-1. When the phone joins, Serial
 * prints "client connected <MAC>" and the LED blinks faster.
 *
 * Lines to edit: BOARD_NAME / AP_PASSWORD in config.h.
 * Set USE_STATION_MODE 1 there to join a hotspot instead.
 *
 * Concepts: Wi-Fi station (STA) vs access point (AP), RSSI (signal strength
 * in dBm, closer to 0 = stronger), IP addresses, mDNS, Wi-Fi events.
 */

#include <WiFi.h>
#include <ESPmDNS.h>
#include "config.h"

const unsigned long BLINK_IDLE_MS      = 1000;  // nobody connected: slow blink
const unsigned long BLINK_CONNECTED_MS = 200;   // a phone is connected: fast blink
const int           WIFI_CHANNEL       = 1;     // fixed channel (Task 5 needs this)
const unsigned long STA_TIMEOUT_MS     = 15000; // give up joining a hotspot after 15 s

bool          ledOn       = false;
unsigned long lastBlinkAt = 0;

// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(300);
  printBanner();
#if !LED_IS_RGB
  pinMode(LED_PIN, OUTPUT);   // plain LED only; rgbLedWrite() sets its own pin up
#endif

  scanNetworks();      // Phase A: listen
  startWifi();         // Phase B: create a network (or join one)
  startNameService();  // extra: reachable as http://<board-name>.local/
}

void loop() {
  updateLed();
}

// ---------------------------------------------------------------------------
// Phase A: scan. Works everywhere, needs no password.
// ---------------------------------------------------------------------------
void scanNetworks() {
  Serial.println("Scanning for Wi-Fi networks...");
  WiFi.mode(WIFI_STA);          // scanning is a "station" activity
  WiFi.disconnect();            // make sure we are not mid-connection
  delay(100);

  int16_t count = WiFi.scanNetworks();   // blocks for a few seconds, returns how many found
  if (count <= 0) {
    Serial.println("No networks found (or scan failed). Try again with RST.");
    return;
  }

  Serial.print(count);
  Serial.println(" networks found:");
  Serial.println("  #  RSSI  CH  SEC   SSID");
  Serial.println("  -- ----  --  ----  --------------------------");
  for (int i = 0; i < count; i++) {
    // RSSI is in dBm: -40 is excellent, -70 is okay, -90 is barely there.
    printPadded(i + 1, 4);
    printPadded(WiFi.RSSI(i), 6);
    printPadded(WiFi.channel(i), 4);
    Serial.print("  ");
    Serial.print(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "lock");
    Serial.print("  ");
    Serial.println(WiFi.SSID(i));
  }
  WiFi.scanDelete();            // free the memory the scan used
  Serial.println();

  // ---- your code: find and print the strongest network ----
}

// Right-align a number in a column of the given width.
void printPadded(long value, int width) {
  String text = String(value);
  for (int i = text.length(); i < width; i++) Serial.print(' ');
  Serial.print(text);
}

// ---------------------------------------------------------------------------
// Phase B: become a Wi-Fi network (default), or join one (station mode).
// ---------------------------------------------------------------------------
void startWifi() {
#if USE_STATION_MODE
  joinHotspot();
#else
  startAccessPoint();
#endif
}

void startAccessPoint() {
  // Ask Wi-Fi to call our functions when a phone joins or leaves. These
  // "events" arrive on their own, so loop() does not have to poll for them.
  WiFi.onEvent(onClientConnected,    ARDUINO_EVENT_WIFI_AP_STACONNECTED);
  WiFi.onEvent(onClientDisconnected, ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);

  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(BOARD_NAME, AP_PASSWORD, WIFI_CHANNEL);
  if (!ok) {
    Serial.println("softAP failed. Is AP_PASSWORD at least 8 characters?");
    return;
  }
  Serial.print("AP started  SSID: ");
  Serial.print(BOARD_NAME);
  Serial.print("  IP: ");
  Serial.println(WiFi.softAPIP());          // almost always 192.168.4.1
  Serial.print("password: ");
  Serial.println(AP_PASSWORD);
  Serial.println("On your phone: join that Wi-Fi. In Task 4 you will open http://192.168.4.1/");
}

// Called by the Wi-Fi driver when a phone joins our network.
void onClientConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.print("client connected ");
  Serial.println(macToString(info.wifi_ap_staconnected.mac));
}

// Called by the Wi-Fi driver when a phone leaves our network.
void onClientDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.print("client disconnected ");
  Serial.println(macToString(info.wifi_ap_stadisconnected.mac));
}

// Turn 6 raw bytes into the usual "AA:BB:CC:DD:EE:FF" form.
String macToString(const uint8_t mac[6]) {
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

void joinHotspot() {
  Serial.print("Joining ");
  Serial.print(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startedAt < STA_TIMEOUT_MS) {
    delay(250);                              // OK to block here: nothing else is running yet
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected. IP: ");
    Serial.println(WiFi.localIP());           // address handed out by the hotspot (DHCP)
    Serial.print("RSSI: ");
    Serial.println(WiFi.RSSI());
    Serial.print("On your phone (same hotspot) open http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");
  } else {
    Serial.println("Could not join. Starting our own access point instead.");
    startAccessPoint();
  }
}

// mDNS lets phones/laptops find us by name: http://esp32-1.local/
// (Works on iOS/macOS/most Linux; Android and Windows support varies.)
void startNameService() {
  String host = String(BOARD_NAME);
  host.toLowerCase();                        // DNS names are case-insensitive; keep it tidy
  if (MDNS.begin(host.c_str())) {
    Serial.print("mDNS name: http://");
    Serial.print(host);
    Serial.println(".local/");
  } else {
    Serial.println("mDNS failed to start (not critical).");
  }
}

// ---------------------------------------------------------------------------
// Blink rate tells you, from across the room, whether anyone is connected.
// ---------------------------------------------------------------------------
void updateLed() {
  unsigned long period = hasClients() ? BLINK_CONNECTED_MS : BLINK_IDLE_MS;
  unsigned long now = millis();
  if (now - lastBlinkAt >= period) {
    lastBlinkAt = now;
    setLed(!ledOn);
  }
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

bool hasClients() {
#if USE_STATION_MODE
  return WiFi.status() == WL_CONNECTED;     // "connected" to the hotspot counts
#else
  return WiFi.softAPgetStationNum() > 0;    // how many phones joined our AP
#endif
}

void printBanner() {
  Serial.println();
  Serial.print("[");
  Serial.print(BOARD_NAME);
  Serial.println("] Task 3: wifi");
}
