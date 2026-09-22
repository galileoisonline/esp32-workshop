# Troubleshooting

Find your symptom in the table. The most common ones have longer sections below. Still stuck after 3 minutes? Ask a helper.

## The table

| Symptom | Likely cause | Fix |
|---|---|---|
| Upload works, Serial Monitor stays empty | Cable is in the **USB** connector; `Serial` only comes out of the **UART** connector | Move the cable to the connector labelled **UART**, pick the new port. See [Two connectors](#0-two-usb-c-connectors-use-uart). |
| Two serial ports appear for one board | Cables in both USB-C connectors | Use the **UART** one (`cu.usbserial-*`, `SLAB_USBtoUART`, `ttyUSB0`); unplug the other. |
| The port name changes after every upload | Cable is in the **USB** connector (native USB re-enumerates on reset) | Move the cable to **UART**. |
| No port under **Tools > Port** (or only `Bluetooth-Incoming-Port` / `COM1`) | Charge-only cable, missing driver, bad USB hub | Swap for a known data cable. Plug directly into the laptop, not a hub. Then see [No port](#1-no-port). |
| Port appears, then disappears a second later | Bad cable or connector, board drawing too much current, Linux `brltty` | Try another cable and USB port. On Ubuntu: `sudo apt remove brltty`. |
| Port appears greyed out / "Permission denied" (Linux) | Not in `dialout` group | `sudo usermod -aG dialout $USER`, log out and in. |
| `Connecting......_____....._____` forever | Board did not enter download mode | Hold BOOT while `Connecting...` prints. See [Wrong boot mode](#2-failed-to-connect--wrong-boot-mode). |
| `A fatal error occurred: Failed to connect to ESP32-S3: Wrong boot mode detected (0x8)! The chip needs to be in download mode.` | Same as above | Same as above. |
| `Failed to connect to ESP32-S3: No serial data received.` | Wrong port selected, board unpowered, another program holding the port | Pick the right port. Close other serial programs. Then the BOOT trick. |
| Upload starts, dies mid-way (`Timed out waiting for packet header`, `Serial data stream stopped`) | Flaky cable, hub, or high baud | **Tools > Upload Speed > 115200**. Direct USB port. |
| `Brownout detector was triggered` in Serial Monitor, board resets over and over | Not enough power over USB | Different cable, USB port, no hub. See [Brownout](#3-brownout-detector-was-triggered). |
| Upload OK, LED never lights | Board is the v1.1 revision: RGB LED on GPIO 38, not 48 | `LED_PIN 38` in `config.h`. See [LED](#5-led-does-not-light). |
| Garbage or `???` in Serial Monitor | Baud mismatch | Serial Monitor dropdown to 115200. |
| Serial Monitor empty | Cable in the **USB** connector, wrong port, board not running | Cable to **UART**. Press RST on the board. Confirm the port. Check baud. |
| Serial Monitor shows `ESP-ROM:esp32s3-...`, `rst:0x1 (POWERON)` and some hex lines every time | Normal boot log | Not an error. Opening the monitor reboots the board. |
| Board boot-loops: banner prints, then `rst:0xc (SW_CPU_RESET)` or `Guru Meditation Error` repeats | Crash in your code, or stale flash from a different partition scheme | Re-flash a known-good sketch. See [Factory reset](#how-to-factory-reset-a-board-that-boot-loops). |
| Phone cannot see your board's Wi-Fi network | 2.4 GHz only, password under 8 characters, board still scanning, everyone has the same name | Wait 10 s and refresh the Wi-Fi list. `AP_PASSWORD` must be 8+ characters. Change `BOARD_NAME`. |
| Phone joins the network but the page does not load | Phone fell back to mobile data, typed `https://`, wrong network | Turn off mobile data. Use `http://192.168.4.1`. See [Page](#4-phone-cannot-reach-the-page). |
| Phone shows "No internet, stay connected?" | Normal: the board has no internet | Tap keep / stay connected / use without internet. |
| Page loads but the numbers never update | JS `fetch` blocked, phone switched networks | Turn off mobile data. Reload. Check `http://192.168.4.1/api/ping` returns `{"ok":true}`. |
| Page is slow with 30 boards in the room | All access points share channel 1 | Normal. It is 30 Wi-Fi networks in one room. Stand closer to your board. |
| `http://<name>.local` does not open | Android does not resolve mDNS | Use the IP: `http://192.168.4.1`. |
| `fatal error: WebServer.h: No such file or directory` (or `WiFi.h`, `ESPmDNS.h`, `esp_now.h`) | Wrong board selected (for example Arduino Uno), or esp32 core not installed | **Tools > Board > esp32 > ESP32S3 Dev Module**. Boards Manager: install **esp32 by Espressif Systems**. |
| Upload says `Chip is ESP32-S3` but the wrong chip was selected, or `This chip is ESP32-S3 not ESP32` | **ESP32 Dev Module** (classic) selected instead of the S3 entry | **Tools > Board > esp32 > ESP32S3 Dev Module**, then upload again. |
| `'rgbLedWrite' was not declared` | esp32 core older than 3.0 | Boards Manager: update **esp32 by Espressif Systems** to 3.3.x. |
| `'ledcSetup' was not declared` / `'hallRead' was not declared` | Code written for esp32 core 2.x | On 3.x use `ledcAttach(pin, freq, bits)` + `ledcWrite(pin, duty)`, or just `analogWrite`. `hallRead` is gone. |
| `invalid conversion from 'void (*)(const uint8_t*, ...)' to 'esp_now_recv_cb_t'` | Old ESP-NOW callback signature | Use `void onRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len)`. Sender MAC is `info->src_addr`. |
| `config.h: No such file or directory` | Sketch opened from the wrong folder, or file dragged out of its folder | Open `tasks/0N_name/0N_name.ino`. The `.h` must sit next to it. |
| First compile takes forever (1-3 min, fan spins) | Normal | Wait. Later builds are 10-20 s. |
| Boards Manager download takes forever / stalls | 300+ MB on venue Wi-Fi | Normal. Wait, or ask for the offline `Arduino15` copy (SETUP.md section 8). |
| `Sketch too big` | Wrong partition scheme | **Tools > Partition Scheme > Default 4MB with spiffs**. |
| Red `exit status 1` with a compiler error you do not understand | Read the first red line, not the last | Scroll up in the output. Typical: missing `;`, mismatched `{ }`, typo in a name. |
| Upload works, but the sketch still behaves like the old one | Uploaded to a different board, or the IDE compiled a stale cache | Check the port. **Sketch > Verify** with Shift+click for a clean build (IDE 2.3.10+). |
| macOS: two ports for one board (`cu.usbserial-...` and `cu.SLAB_USBtoUART`) | Apple driver and the Silicon Labs driver both installed | Try the other port. Long-term: remove old `/Library/Extensions/usbserial.kext`, reboot. |
| Windows: Unknown device in Device Manager | Driver not fetched by Windows Update | Install the CP210x driver (SETUP.md section 1). |
| `touchRead` gives 0 or a constant | Wrong pin | `TOUCH_PIN` must be a touch-capable pin: T1-T14 = GPIO 1-14 on the S3. The workshop uses T4 = GPIO 4. |
| Touch value does not drop when touched | On the S3 it rises | Normal. Untouched is roughly 20000-30000, touched 40000+. Watch for a rise, not a drop. |
| `analogRead` returns 0 or 4095 while Wi-Fi is on | Pin is on ADC2 (GPIO 11-20) | Use an ADC1 pin: GPIO 1-10. |

## 0. Two USB-C connectors: use UART

The ESP32-S3-DevKitC-1 has two USB-C connectors, labelled **UART** and **USB** on the silkscreen. Both power the board and both can upload. Only **UART** shows `Serial` output.

- **UART** goes through a CP2102N USB-to-serial chip. It looks like a normal serial port (`cu.usbserial-*`, `cu.SLAB_USBtoUART`, `COMx`, `ttyUSB0`), auto-reset for upload works, and `Serial.print` appears in the Serial Monitor. Use this one.
- **USB** is the ESP32-S3's own USB port. It shows up as `cu.usbmodem*`, `ttyACM0` or "USB Serial Device (COMx)". Uploads work, but `Serial` is still wired to the UART pins (unless **Tools > USB CDC On Boot** is Enabled, which the workshop does not do), so the Serial Monitor stays empty, and the port disappears and reappears after every upload.

Symptom to remember: "upload works but the Serial Monitor is empty" means the cable is in the wrong connector. Move it to UART, pick the new port under **Tools > Port**, reopen the Serial Monitor.

## 1. No port

Work through in order. Stop when a port appears.

1. Cable. Swap it for one you know moves data (a helper's cable). Charge-only cables are the number one cause. The board's red power LED lighting up proves nothing. Make sure it is in the **UART** connector.
2. Direct connection. Plug into the laptop itself, not a hub, dock or monitor. USB-C to USB-A adapters are fine.
3. Look at the OS, not the IDE.
   - macOS: Terminal, `ls /dev/cu.*`. A new entry should appear when you plug in.
   - Windows: **Device Manager > Ports (COM & LPT)**. Also check **Other devices** for a warning icon, which means a driver is needed.
   - Linux: `dmesg | tail -20` right after plugging in. Look for `ttyUSB0`, or for `brltty`.
4. Driver. Only now install one; see SETUP.md section 1 for which chip needs which. Reboot on macOS.
5. Try another board. If a helper's board shows up on your laptop, your board or its USB connector is the problem. Swap boards.

Note: the IDE shows "Unknown" next to ESP32 ports on the UART connector. It only recognises Arduino-branded boards by name.

## 2. Failed to connect / Wrong boot mode

Full message:

```
A fatal error occurred: Failed to connect to ESP32-S3: Wrong boot mode detected (0x8)! The chip needs to be in download mode.
For troubleshooting steps visit: https://docs.espressif.com/projects/esptool/en/latest/troubleshooting.html
```

`0x8` means the chip booted normally from flash instead of into the serial download mode. The IDE tries to do this automatically by wiggling two USB signal lines; on some cables and drivers the timing does not work. The DevKitC-1's UART connector normally gets this right; if you see it every time, check the cable is in **UART**.

The BOOT trick (do this every upload if your board needs it):

1. Click **Upload**.
2. Wait for `Connecting...` to appear in the output.
3. Press and hold BOOT on the board.
4. When you see `Writing at 0x...`, let go.

Still failing: hold BOOT, tap RST once, release BOOT. The board is now stuck in download mode. Click Upload. After the upload, press RST once to start the program.

Failing every time even with the trick: **Tools > Upload Speed > 115200**, direct USB port, other cable.

Note: after a successful upload the board prints `Hard resetting via RTS pin...` and reboots. That is the IDE restarting it.

## 3. `Brownout detector was triggered`

In the Serial Monitor:

```
Brownout detector was triggered

ESP-ROM:esp32s3-20210327
rst:0x10 (RTCWDT_RTC_RESET),boot:0x8 (SPI_FAST_FLASH_BOOT)
```

The chip's supply voltage dipped below about 2.4 V, so it reset itself to avoid corrupting flash. It happens most when Wi-Fi turns on (Task 3 onwards), because the radio pulls a current spike. This is a power problem, never a code problem.

1. Different USB cable (thin or long cables drop voltage).
2. Different USB port, directly on the laptop. Unpowered hubs and keyboard or monitor USB ports are weak.
3. Remove anything else plugged into the board's 3V3 / 5V pins.
4. Try a USB wall charger just to power it (you lose Serial, but the access point still works and the page still loads).
5. If one board browns out and all others do not on the same cable and port, that board's voltage regulator is weak. Swap boards.

Do not disable the brownout detector in software. It is protecting your flash.

## 4. Phone cannot reach the page

Checklist, in order:

1. Wi-Fi list on the phone. You must be connected to your own board's network (its `BOARD_NAME`, for example `ESP32-1`), not the campus Wi-Fi and not a classmate's board. Password: `hackhardware` (or what you set in `AP_PASSWORD`).
2. "No internet" warning. Your board is not on the internet. When the phone says "This network has no internet access, stay connected?" tap yes / keep connection / stay connected. On iPhone, the Wi-Fi icon may show "No Internet Connection" under the name; that is fine.
3. Turn off mobile data (Settings > Mobile/Cellular > off, or airplane mode and then Wi-Fi back on). Android and iOS silently route browser traffic to 4G/5G when Wi-Fi has no internet. Samsung: also turn off Intelligent Wi-Fi / Switch to mobile data.
4. URL. Type `http://192.168.4.1`: `http`, not `https`. Browsers auto-upgrade to `https://` and then show a certificate error. If it does, delete the `s` by hand. `192.168.4.1` is the ESP32's default access-point address; the Serial Monitor prints it at boot.
5. Test the API directly: `http://192.168.4.1/api/ping` should show `{"ok":true}`. If that works but the page is blank, reload; if the page loads but numbers do not update, it is step 3.
6. Station mode users (`USE_STATION_MODE 1`): the IP is not 192.168.4.1. Read it from the Serial Monitor. The phone must be on the same hotspot. Campus Wi-Fi blocks device-to-device traffic; use a personal hotspot.
7. Too many phones. The access point allows 4 clients by default. One phone per board.
8. A laptop instead of a phone works too: join the board's network from the laptop and open the URL. You lose internet on the laptop while connected.

## 5. LED does not light

Upload said `Hard resetting via RTS pin...` and Serial prints the banner, but no light.

1. Find the LED. The DevKitC-1 has two: a red one = power (always on, not controllable) and a small white square next to it = the addressable RGB LED (yours). There is no plain blue LED like on the classic DevKit.
2. Check the board version. The RGB LED is on **GPIO 48** on the original board and **GPIO 38** on v1.1. Both are in the room and look the same. Open `config.h` and change
   ```cpp
   #define LED_PIN 48
   ```
   to `38` (or back to `48`), upload again. One of the two works.
3. The stock **File > Examples > 01.Basics > Blink** only knows GPIO 48 (`LED_BUILTIN`), so on a v1.1 board it lights nothing even though the board is fine. Use the workshop's Task 1 with `LED_PIN 38` instead.
4. Still dark on both pins? Confirm `LED_IS_RGB` is `1` in `config.h` and `LED_BRIGHTNESS` is not `0`. `digitalWrite` and `analogWrite` do nothing useful on this LED; it needs `rgbLedWrite(LED_PIN, r, g, b)`.
5. Need a plain LED anyway (for a challenge)? Borrow a jumper wire, an LED and a 220 ohm resistor from the helper table: a free GPIO (for example 5) to resistor to LED long leg; LED short leg to GND. Drive it with `pinMode`/`digitalWrite`.
6. Confirm the code actually runs: the Serial Monitor at 115200 should show `[ESP32-1] Task 1: blink` (with your name). If not, see the Serial rows in the table (most likely the cable is in the **USB** connector).

## How to factory-reset a board that boot-loops

Symptoms: the board restarts every second, Serial spams `rst:0xc (SW_CPU_RESET)` / `Guru Meditation Error` / `invalid header: 0xffffffff` / `Core dump flash config is corrupted`, and nothing you upload seems to change it.

Option A: flash something known-good. Open **File > Examples > 01.Basics > Blink** and upload it (with the BOOT trick if needed). Most "bricked" boards are just running a crashing sketch.

Option B: wipe the flash first.

1. **Tools > Erase All Flash Before Sketch Upload > Enabled**
2. Upload Blink. The upload now starts with `Erasing flash (this may take a while)...` and takes about 20 s longer.
3. **Tools > Erase All Flash Before Sketch Upload > Disabled** (leave it off, or every upload erases saved `Preferences`).

Option B also clears anything stored with `Preferences` (for example a saved board name from a challenge). Equivalent command line: `esptool --chip esp32s3 --port <PORT> erase_flash`.

Still looping after a clean Blink: hardware (power or flash chip). Swap boards; mark the bad one with tape.
