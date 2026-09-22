# ESP32-S3 cheat sheet (ESP32-S3-DevKitC-1, Arduino IDE 2, esp32 core 3.3.x)

Board facts: RGB LED on GPIO 38 (v1.1) or 48 (original), `rgbLedWrite` only. BOOT button GPIO 0 (pressed = LOW). RST button resets. Touch T4 = GPIO 4, value rises when touched. Two USB-C connectors: use **UART** (the **USB** one uploads but shows no Serial). 3.3 V max on any pin, never 5 V. Wi-Fi 2.4 GHz only. Access-point IP 192.168.4.1. Serial 115200 on GPIO 43/44.

Upload stuck at `Connecting...`? Hold BOOT. Still failing? **Tools > Upload Speed > 115200**. Garbage in Serial? Set 115200. Serial empty? Cable to the UART connector.

## Which pins you can use

| Pins | Use |
|---|---|
| 1, 2, 4-18, 21, 39-42, 47, and 38 or 48 (whichever is not the LED) | Safe for input or output. Start here. Every GPIO has a pull-up; there are no input-only pins. |
| 1-10 | ADC1: `analogRead` works with Wi-Fi on. |
| 11-20 | ADC2: `analogRead` is unreliable while Wi-Fi is on. Fine as digital pins. |
| 1-14 | Touch pads T1-T14 (T4 = GPIO 4 in the workshop). |
| 8, 9 | Default I2C SDA, SCL. 10, 11, 12, 13 default SPI SS, MOSI, SCK, MISO. |
| 0, 3, 45, 46 | Strapping pins: read at boot (0 = BOOT, 45 = flash voltage, 46 must be low, 3 = JTAG). OK once running, with care. |
| 19, 20 | USB D-/D+ of the USB connector. Avoid. |
| 43, 44 | TX/RX = `Serial` via the UART connector. Using them breaks the Serial Monitor. |
| 35, 36, 37 | Used by the octal flash/PSRAM on R8 boards. Avoid. (26-34 are not on the header.) |
| 38 or 48 | The RGB LED, depending on board version. |

```cpp
// ---- Sketch skeleton --------------------------------------------------
#include "config.h"                  // BOARD_NAME, LED_PIN, LED_BRIGHTNESS, BUTTON_PIN, TOUCH_PIN, AP_PASSWORD
void setup() {                       // runs once at power-on / RST press
  Serial.begin(SERIAL_BAUD);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // idle HIGH, pressed LOW
}                                    // the RGB LED needs no pinMode; rgbLedWrite sets it up
void loop() { }                      // runs forever, as fast as it can

// ---- The RGB LED --------------------------------------------------------
rgbLedWrite(LED_PIN, 40, 40, 40);    // r, g, b each 0-255: white at brightness 40 (255 is blinding)
rgbLedWrite(LED_PIN, 0, 0, 0);       // off
rgbLedWrite(LED_PIN, 40, 0, 0);      // red;  0,40,0 green;  0,0,40 blue. Brightness IS the r,g,b values.
                                     // digitalWrite / analogWrite do nothing useful on this LED.

// ---- Digital I/O (any free GPIO, e.g. an external LED on 5) ------------
pinMode(5, OUTPUT);  digitalWrite(5, HIGH);   // 3.3 V on the pin   (LOW = 0 V)
int pressed = (digitalRead(BUTTON_PIN) == LOW);
analogWrite(5, 128);                 // PWM 0-255 on a plain pin (core 3.x, no setup); not for the RGB LED
ledcAttach(5, 5000, 8);  ledcWrite(5, 64);    // explicit PWM: 5 kHz, 8-bit; duty by pin (ledcSetup is gone)

// ---- Analog, touch, temperature ----------------------------------------
int   raw   = analogRead(5);         // 0-4095 (12-bit), GPIO 1-10 when Wi-Fi is on
float volts = analogReadMilliVolts(5) / 1000.0;
uint32_t t  = touchRead(TOUCH_PIN);  // S3: roughly 10000-30000 idle, RISES to about double that when touched (varies)
float tempC = temperatureRead();     // chip die temperature, C, rough (reads 5-10 C warm). hallRead() no longer exists.

// ---- Timing: the millis() pattern (never delay() once Wi-Fi is on) -----
unsigned long lastBlink = 0;  bool ledOn = false;
void loop() {
  if (millis() - lastBlink >= 500) {   // subtraction survives the 49-day wrap
    lastBlink = millis();
    ledOn = !ledOn;  setLed(ledOn);      // setLed() is the workshop helper around rgbLedWrite
  }
}
delay(500);                          // blocks everything for 500 ms; Task 1 only

// ---- Serial ------------------------------------------------------------
Serial.print("temp="); Serial.print(tempC, 1); Serial.println("C");  // 1 decimal
Serial.printf("t=%lus temp=%.1fC touch=%u\n", millis()/1000, tempC, t);
if (Serial.available()) { String cmd = Serial.readStringUntil('\n'); }

// ---- Wi-Fi: access point (default in this workshop) --------------------
#include <WiFi.h>
WiFi.mode(WIFI_AP);
WiFi.softAP(BOARD_NAME, AP_PASSWORD, 1);   // password 8-63 chars, 2.4 GHz, channel 1
Serial.println(WiFi.softAPIP());           // 192.168.4.1
int clients = WiFi.softAPgetStationNum();
WiFi.onEvent(onJoin, ARDUINO_EVENT_WIFI_AP_STACONNECTED);      // call before softAP()
void onJoin(WiFiEvent_t e, WiFiEventInfo_t info) { /* info.wifi_ap_staconnected.mac[0..5] */ }

// ---- Wi-Fi: station (join a hotspot) + scan ----------------------------
WiFi.mode(WIFI_STA);
WiFi.begin(WIFI_SSID, WIFI_PASS);
while (WiFi.status() != WL_CONNECTED) { delay(250); Serial.print('.'); }
Serial.println(WiFi.localIP());  int rssi = WiFi.RSSI();   // dBm: -40 great, -85 poor
int n = WiFi.scanNetworks();     // then WiFi.SSID(i), WiFi.RSSI(i), WiFi.channel(i), WiFi.encryptionType(i)

// ---- mDNS (optional; iPhones resolve it, most Androids do not) ---------
#include <ESPmDNS.h>
MDNS.begin("esp32-1");  MDNS.addService("http", "tcp", 80);   // http://esp32-1.local

// ---- WebServer route returning JSON ------------------------------------
#include <WebServer.h>
WebServer server(80);
void setup() {  /* Wi-Fi first */
  server.on("/api/ping", []() { server.send(200, "application/json", "{\"ok\":true}"); });
  server.on("/api/led", []() {                       // GET /api/led?state=on|off|toggle
    String s = server.arg("state");
    if (s == "on") ledOn = true; else if (s == "off") ledOn = false; else ledOn = !ledOn;
    setLed(ledOn);                                   // rgbLedWrite(LED_PIN, v, v, v) with v = 40 or 0
    server.send(200, "application/json", "{\"led\":" + String(ledOn) + "}");
  });
  server.on("/api/color", []() {                     // GET /api/color?r=40&g=0&b=0
    rgbLedWrite(LED_PIN, server.arg("r").toInt(), server.arg("g").toInt(), server.arg("b").toInt());
    server.send(200, "application/json", "{\"ok\":true}");
  });
  server.onNotFound([]() { server.send(404, "text/plain", "no such route"); });
  server.begin();
}
void loop() { server.handleClient(); }                // call it every loop, no delay()
// Workshop routes: /  /api/status  /api/led?state=  /api/pattern?name=off|on|blink|sos  /api/ping  /api/peers (Task 5)
// /api/color?r=&g=&b= is an extension, not in the stock sketch.

// ---- Preferences (survives reboot) --------------------------------------
#include <Preferences.h>
Preferences prefs;  prefs.begin("esp32", false);
prefs.putString("name", "ESP32-7");  String name = prefs.getString("name", BOARD_NAME);

// ---- ESP-NOW minimal broadcast (core 3.x signatures) --------------------
#include <esp_now.h>
struct Message { char name[12]; uint32_t uptime_s; float temp_c; uint8_t led; };   // 24 bytes; max 250
uint8_t BCAST[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
void onRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {          // not (const uint8_t *mac, ...)
  Message m; memcpy(&m, data, min(len, (int)sizeof m));
  Serial.printf("heard %s rssi %d\n", m.name, info->rx_ctrl->rssi);                   // sender MAC: info->src_addr
}
void setup() {
  WiFi.mode(WIFI_AP_STA);  WiFi.softAP(BOARD_NAME, AP_PASSWORD, 1);   // fixed channel 1 so all boards match
  esp_now_init();  esp_now_register_recv_cb(onRecv);
  esp_now_peer_info_t peer = {};  memcpy(peer.peer_addr, BCAST, 6);  peer.channel = 0;  peer.encrypt = false;
  esp_now_add_peer(&peer);
}
Message msg = {"ESP32-1", millis()/1000, temperatureRead(), 1};
esp_now_send(BCAST, (const uint8_t*)&msg, sizeof msg);              // every 2 s from loop()

// ---- Deep sleep ----------------------------------------------------------
RTC_DATA_ATTR int bootCount = 0;                      // survives deep sleep (not power loss)
esp_sleep_enable_timer_wakeup(10ULL * 1000000ULL);   // microseconds
touchSleepWakeUpEnable(TOUCH_PIN, 40000);             // S3: wake when touch value > threshold (classic ESP32: <)
esp_deep_sleep_start();                               // setup() runs again on wake
```

Arduino IDE 2 shortcuts: Verify Ctrl/Cmd+R. Upload Ctrl/Cmd+U. Serial Monitor Ctrl/Cmd+Shift+M. Serial Plotter Ctrl/Cmd+Shift+L. Boards Manager Ctrl/Cmd+Shift+B. Auto-format Ctrl/Cmd+T. Comment line Ctrl/Cmd+/. Clean rebuild: Shift+click Verify.

Menus: **Tools > Board > esp32 > ESP32S3 Dev Module** (leave USB CDC On Boot Disabled, Flash Size 4MB, PSRAM Disabled). **Tools > Port**. **Tools > Upload Speed**. **Tools > Erase All Flash Before Sketch Upload** (boot-loop rescue). **File > Examples > 01.Basics > Blink** (knows only GPIO 48; on a v1.1 board it lights nothing).

Chip: ESP32-S3, 2 Xtensa LX7 cores at 240 MHz, 512 KB SRAM, 8 or 16 MB flash and 8 MB PSRAM on the module, Wi-Fi 2.4 GHz b/g/n, Bluetooth 5 LE only, 45 GPIO, 2 x 12-bit ADC, 14 touch pads, temperature sensor, USB OTG, 8 PWM channels, 3 UART, 2 I2C, 4 SPI, no DAC, no hall sensor. Deep sleep about 8 uA, Wi-Fi TX peaks about 300 mA.

Reading a compiler error: scroll up to the first red line. Missing `;`, unbalanced `{}`, or a typo most of the time.

Sources (checked Sept 2026): ESP32-S3-DevKitC-1 user guide (pinout, RGB LED, connectors) https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide.html. Default pins in variants/esp32s3/pins_arduino.h https://github.com/espressif/arduino-esp32/blob/master/variants/esp32s3/pins_arduino.h. core 3.x migration guide (LEDC, hallRead removed, ESP-NOW callback) https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html. `rgbLedWrite`/`temperatureRead`/`analogWrite` in esp32-hal.h https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal.h. ESP-NOW 3.x callback change https://github.com/espressif/arduino-esp32/issues/9737. IDE 2 Tools menu https://docs.espressif.com/projects/arduino-esp32/en/latest/guides/tools_menu.html. ESP32-S3 pin notes https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/gpio.html.
