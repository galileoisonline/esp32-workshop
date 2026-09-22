# Tasks

Five sketches that take your ESP32 from a blinking LED to a web page on your phone that controls the board. Do them in order; each one builds on the last.

First thing, before any task: open `config.h` and set `BOARD_NAME`. Each task folder has its own identical copy of `config.h`. Open the sketch, click the `config.h` tab, and change `#define BOARD_NAME "ESP32-1"` to a name of your own (letters, digits, dashes, max 11 characters). Your Wi-Fi network will have that name, so it must be unique in the room. `LED_PIN` is 48; if the LED never lights in Task 1 your board is the v1.1 revision and needs `LED_PIN 38`.

| # | Task | Goal | Check | Time |
|---|---|---|---|---|
| 1 | [blink](01_blink/README.md) | Blink the onboard RGB LED at a rate you choose | LED blinks at your rate; banner in Serial Monitor | 15 min |
| 2 | [serial and button](02_serial_button/README.md) | Print uptime/temp/touch/button once a second; BOOT toggles the blinking | Press BOOT, `EVENT button pressed` appears | 15 min |
| 3 | [wifi](03_wifi/README.md) | Scan Wi-Fi, then host your own network named `BOARD_NAME` | Phone sees the network; `client connected` in Serial | 15 min |
| 4 | [web dashboard](04_web_dashboard/README.md) | Serve a live status page and JSON API; control the LED from your phone | Tap LED off / LED on on the phone, LED follows | 25 min |
| 5 | [esp-now](05_esp_now/README.md) | ESP-NOW broadcast; list every other board you can hear | Partner's board name shows on your page | 15 min |

## Before you start

- Cable in the USB-C connector labelled **UART** (not **USB**). The USB one uploads fine but shows nothing in the Serial Monitor.
- Board: **Tools > Board > esp32 > ESP32S3 Dev Module**. Port: **Tools > Port**, your USB serial device.
- Serial Monitor (icon top-right) at **115200** baud.
- Upload sits on `Connecting...`? Hold the **BOOT** button on the board until dots stream.
- Full setup: [../docs/SETUP.md](../docs/SETUP.md). Problems: [../docs/TROUBLESHOOTING.md](../docs/TROUBLESHOOTING.md).

## Layout

```
tasks/
├── 01_blink/            01_blink.ino + config.h + README.md
├── 02_serial_button/    02_serial_button.ino + config.h + README.md
├── 03_wifi/             03_wifi.ino + config.h + README.md
├── 04_web_dashboard/    04_web_dashboard.ino + page.h + config.h + README.md
└── 05_esp_now/          05_esp_now.ino + page.h + config.h + README.md
```

Arduino only lets a sketch `#include` files in its own folder, which is why every folder carries the same
`config.h`. If you change it in one place, copy it to the others (or just set `BOARD_NAME` again).

All sketches target Arduino IDE 2.x + esp32 core 3.3.x on the ESP32-S3-DevKitC-1 and use only libraries bundled
with the core (`WiFi`, `WebServer`, `ESPmDNS`, `esp_now`; the RGB LED uses the core's `rgbLedWrite`). Nothing to
install from Library Manager.
