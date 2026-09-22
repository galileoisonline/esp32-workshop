# Getting started with the ESP32

Hands-on workshop by HackHardware. You start with an ESP32-S3 development
board and end with the same board serving a web page to your phone that shows its status
and switches its LED. Along the way you learn the six things every embedded project is made
of: pins, serial output, timing without `delay()`, Wi-Fi, HTTP, and board-to-board radio.

No electronics experience is needed. If you can install a program and edit a line of text,
you can do everything here.

## Contents

- [What you need](#what-you-need)
- [Quick start](#quick-start)
- [The five tasks](#the-five-tasks)
- [How the code is organised](#how-the-code-is-organised)
- [The board](#the-board)
- [The web API (Tasks 4 and 5)](#the-web-api-tasks-4-and-5)
- [Building from the command line](#building-from-the-command-line)
- [When something does not work](#when-something-does-not-work)
- [For facilitators](#for-facilitators)
- [Repository layout](#repository-layout)
- [Licence](#licence)

## What you need

| Item | Notes |
|---|---|
| An **ESP32-S3-DevKitC-1** board | Espressif's official ESP32-S3 board (ESP32-S3-WROOM-1 module, N8R8 or N16R8), either the original or v1.1. 44 pins, two USB-C connectors, an RGB LED, BOOT and RST buttons. The code also runs on a classic ESP32 DevKit V1 if you set `LED_PIN 2`, `LED_IS_RGB 0` and `TOUCH_PIN T0` in `config.h`. |
| A USB-C **data** cable | Both connectors on the board are USB-C. Many cables are charge-only and will not show a serial port; if in doubt, bring two. |
| A laptop | macOS, Windows or Linux. About 500 MB of disk for the IDE and the ESP32 toolchain. |
| A phone | Anything with Wi-Fi and a browser, for Tasks 3–5. |

Nothing else. Every task uses parts already on the board: the RGB LED, the BOOT button, a
capacitive-touch pin, the internal temperature sensor and the Wi-Fi radio.

## Quick start

1. **Install the tools** (10 minutes, needs internet): [docs/SETUP.md](docs/SETUP.md).
   In short: Arduino IDE 2.x, then the **esp32 by Espressif Systems** board package (3.3.x)
   from Boards Manager, board **ESP32S3 Dev Module**. Do this before the session if you can.
2. **Get this repository**: **Code > Download ZIP** and unzip it, or
   `git clone` it. Keep the folder structure: the IDE needs each `.ino` inside a folder of the same name.
3. **Name your board**: open `tasks/01_blink/config.h` and set

   ```c
   #define BOARD_NAME "ESP32-1"      // change to something unique, max 11 characters
   ```

   Each task folder has its own identical `config.h`; change it in each task you open, or copy the file over. The name becomes your board's Wi-Fi network name in Task 3.
4. **Plug the cable into the connector labelled UART**, not the one labelled USB. Both power the board and both upload, but only UART shows `Serial` output in the Serial Monitor.
5. **Do the tasks in order.** Open `tasks/01_blink/01_blink.ino` in the IDE, select your board and port, click **Upload**. Each task's `README.md` has the steps, one observable check, and ideas for when you finish early.

## The five tasks

| # | Task | What you learn | Check | Time |
|---|---|---|---|---|
| 1 | [Blink an LED](tasks/01_blink/) | `setup()`/`loop()`, `rgbLedWrite`, `delay()`, uploading and un-sticking uploads | the RGB LED blinks at a rate you chose | 15 min |
| 2 | [Serial output and a button](tasks/02_serial_button/) | Serial Monitor, `INPUT_PULLUP`, debouncing, edge detection, `millis()` instead of `delay()`, `touchRead`, `temperatureRead` | pressing BOOT prints `EVENT button pressed` | 15 min |
| 3 | [Turn on Wi-Fi](tasks/03_wifi/) | Wi-Fi scan and RSSI, access-point vs station mode, IP addresses, Wi-Fi events, mDNS | your phone joins the network `ESP32-<name>` | 15 min |
| 4 | [A web page served by the board](tasks/04_web_dashboard/) | HTTP routes with `WebServer`, JSON, a page that polls the board, controlling hardware from a browser | you switch the LED from your phone at `http://192.168.4.1` | 25 min |
| 5 | [Messages between boards](tasks/05_esp_now/) | ESP-NOW: connectionless packets between ESP32s, structs on the air, peer tables | another board's name appears on your page | 15 min, optional |

Each task is a complete, working sketch. If you fall behind, open the next one; nothing
depends on you having finished the previous task.

Finished early? [CHALLENGES.md](CHALLENGES.md) has fifteen extensions in two difficulty
groups, each with a hint and a "done when".

## How the code is organised

```
tasks/04_web_dashboard/
├── 04_web_dashboard.ino    the sketch: setup(), loop(), one small function per job
├── config.h                pins, board name, Wi-Fi settings (identical in every task)
├── page.h                  the HTML/CSS/JS the board serves, as one string in flash
└── README.md               steps, check, extensions, task-specific troubleshooting
```

- **`config.h`** is the only file you need to edit to adapt to a different board:
  `BOARD_NAME`, `LED_PIN` (38; 48 on original-revision boards), `LED_IS_RGB` (1), `LED_BRIGHTNESS` (40),
  `BUTTON_PIN` (0), `TOUCH_PIN` (T4 = GPIO 4), `SERIAL_BAUD` (115200), `AP_PASSWORD`
  (`hackhardware`), `USE_STATION_MODE` (0 = the board runs its own Wi-Fi network, 1 = it
  joins `WIFI_SSID`/`WIFI_PASS`, for example your phone's hotspot).
- Every sketch prints a start-up line `[<BOARD_NAME>] Task N: <name>` at 115200 baud so
  you can tell what is running.
- From Task 2 onward there is no `delay()` in `loop()`. Everything is timed with `millis()`
  so the button, the LED pattern and the web server all run together.
- Sketches use only libraries that ship with the ESP32 core (`WiFi`, `WebServer`,
  `ESPmDNS`, `esp_now`). Nothing to install from Library Manager.
- Extension points are marked in the code with `// ---- add your own ... ----`.

## The board

The code assumes an ESP32-S3-DevKitC-1. Constants are in `config.h` if yours differs.

| Part | Where | Notes |
|---|---|---|
| RGB LED | GPIO 38 (v1.1, the default) or GPIO 48 (original) | `LED_PIN`. One addressable WS2812-type LED, driven with `rgbLedWrite(LED_PIN, r, g, b)`; no plain LED. Both board versions look the same: if nothing lights, change 48 to 38. The red LED next to it is power, always on. |
| BOOT button | GPIO 0 | `BUTTON_PIN`. Reads `LOW` when pressed. Hold it during **Upload** if the IDE is stuck on `Connecting...`. |
| RST button | – | Resets the chip. |
| Touch pad | GPIO 4 (`T4`) | Header pin labelled 4, left side. Touch the pin or a jumper wire plugged into it. The value **rises** when touched. |
| Serial | GPIO 43 (TX), 44 (RX) | This is `Serial`, via the CP2102N chip behind the **UART** connector. Leave these pins alone. |
| USB | two USB-C connectors | **UART**: normal serial port, use this one. **USB**: the chip's native USB; uploads work but `Serial` output does not appear. |
| Power | USB 5 V in, 3.3 V logic | GPIO pins are **not** 5 V tolerant. A pin sources about 12 mA. |
| Wi-Fi | built in | 2.4 GHz only. Bluetooth 5 LE, no Bluetooth Classic. In AP mode the board is always `192.168.4.1`. |

Pins you can and cannot use on the ESP32-S3-DevKitC-1:

| GPIO | Status |
|---|---|
| 26–32 | Never: connected to the flash chip (not on the headers). |
| 33–37 | Used by the octal flash/PSRAM on R8 boards. 35, 36, 37 are on the header: avoid them. |
| 0, 3, 45, 46 | Strapping pins, sampled at reset (0 = BOOT, 45 = flash voltage, 46 must be low at boot, 3 = JTAG source). Usable afterwards with care. |
| 19, 20 | USB D-/D+ for the USB connector. Avoid. |
| 43, 44 | TX0/RX0 = `Serial`. Leave alone. |
| 38 or 48 | RGB LED, whichever your board version uses. |
| 11–20 | ADC2: `analogRead` is unreliable while Wi-Fi is on. Use 1–10 (ADC1) for analog. |
| 1, 2, 4–18, 21, 39–42, 47, and the other of 38/48 | Free for inputs and outputs. 8/9 are the default I²C pins, 10–13 the default SPI pins. No input-only pins; every GPIO has a pull-up. |

Header pins, top to bottom (USB connectors at the bottom). Left: 3V3, 3V3, RST, 4, 5, 6, 7, 15, 16, 17, 18, 8, 3, 46, 9, 10, 11, 12, 13, 14, 5V, GND. Right: GND, TX, RX, 1, 2, 42, 41, 40, 39, 38, 37, 36, 35, 0, 45, 48, 47, 21, 20, 19, GND, GND.

## The web API (Tasks 4 and 5)

Join the board's Wi-Fi network (`ESP32-<name>`, password `hackhardware`), then:

| Route | Returns |
|---|---|
| `GET /` | the status page (`page.h`) |
| `GET /api/status` | `{"ok":true,"name":"ESP32-1","uptime_s":42.0,"temp_c":47.3,"touch":24310,"button":0,"led":1,"pattern":"blink","clients":1,"rssi":null,"heap_free":251032,"chip":"ESP32-S3"}` |
| `GET /api/led?state=on\|off\|toggle` | the same status object after applying the change |
| `GET /api/pattern?name=off\|on\|blink\|sos` | the same status object |
| `GET /api/ping` | `{"ok":true}` |
| `GET /api/peers` (Task 5) | `[{"name":"ESP32-2","age_s":1.4,"temp_c":48.1,"led":1,"rssi":-52}, ...]` |

The page polls `/api/status` once a second with `fetch()`. Typing a route into the phone's
browser is the quickest way to debug a new one. If the page will not load, turn off mobile
data (the phone is routing around a network with no internet) and make sure the address
starts with `http://`, not `https://`.

## Building from the command line

The Arduino IDE is all you need for the workshop. If you prefer `arduino-cli`:

```sh
arduino-cli config set board_manager.additional_urls \
  https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32
arduino-cli compile --fqbn esp32:esp32:esp32s3 tasks/01_blink
arduino-cli upload  --fqbn esp32:esp32:esp32s3 -p /dev/cu.usbserial-0001 tasks/01_blink
arduino-cli monitor -p /dev/cu.usbserial-0001 -c baudrate=115200
```

All five sketches compile for `esp32:esp32:esp32s3` on core 3.3.12 with the default 4 MB
partition scheme.

## When something does not work

The five most common problems, from [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md):

| Symptom | Fix |
|---|---|
| Upload works but the Serial Monitor is empty | The cable is in the **USB** connector. Move it to the one labelled **UART**. |
| No serial port appears | Use a different (data) cable. Then install the CP210x driver ([docs/SETUP.md](docs/SETUP.md)). |
| Upload works but the LED never lights | Your board is v1.1: set `LED_PIN 38` in `config.h`. |
| `Failed to connect to ESP32-S3: Wrong boot mode detected` | Hold **BOOT** while the IDE prints `Connecting...`; release when it says `Writing at`. |
| `Brownout detector was triggered`, board reboots in a loop | Weak USB power. Plug straight into the laptop with a short cable, not a hub. |
| Garbage in the Serial Monitor | Set the monitor to 115200 baud. |
| Phone cannot open `192.168.4.1` | Stay on the board's network when the phone warns "no internet", turn off mobile data, use `http://`. |

Everything else: [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md), then ask a helper.
Keep [docs/CHEATSHEET.md](docs/CHEATSHEET.md) open; every function used in the workshop is
on that one page.

## For facilitators

- [docs/FACILITATOR_GUIDE.md](docs/FACILITATOR_GUIDE.md): prep checklist, room setup, the
  120-minute run of show with "if you are behind at minute X, cut Y" rules, 90- and
  150-minute variants, per-task talking points and what students get stuck on, helper roles,
  wrap-up script.
- [slides/](slides/): `ESP32_Workshop.pptx`, 32 slides with speaker notes, ready for
  Google Slides or PowerPoint. `slides/README.md` lists the placeholders to fill in and
  which slides to skip if time is short.
- Bring: boards, USB-C data cables, USB-C to USB-A adapters, power strips, a USB stick with the IDE
  installers and the ESP32 core for offline installs, one board pre-flashed with Task 4
  to demo. Details in the guide.

## Repository layout

```
tasks/
  01_blink/            Task 1
  02_serial_button/    Task 2
  03_wifi/             Task 3
  04_web_dashboard/    Task 4 (+ page.h)
  05_esp_now/          Task 5 (+ page.h)
  README.md            task index
docs/
  SETUP.md             install the IDE, drivers and board package on macOS, Windows, Linux
  TROUBLESHOOTING.md   symptom, cause, fix
  CHEATSHEET.md        one printable page of every function used
  FACILITATOR_GUIDE.md how to run the session
slides/
  ESP32_Workshop.pptx  the deck
  README.md
CHALLENGES.md          extensions for people who finish early
LICENSE
```

## Licence

Code is MIT. Slides and documentation are CC BY 4.0. Fork it, change it, run your own
session; keep the HackHardware credit.
