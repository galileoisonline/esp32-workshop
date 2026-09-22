# Setup: get your laptop talking to the board

Goal: Arduino IDE 2 installed, the ESP32 core installed, your board visible on a serial port, and one test program (Blink) uploaded. About 15 minutes with internet. Do section 0 at home if you can.

> Note: use a USB-C data cable. Many USB-C cables that came with headphones, power banks or toothbrushes are charge-only: they have no data wires. The board's power LED will light up, but no port will ever appear. If in doubt, borrow a cable from a helper.

> Which USB connector: the ESP32-S3-DevKitC-1 has two USB-C connectors, labelled **UART** and **USB** on the board. Use **UART**. It goes through a CP2102N USB-to-serial chip, shows up as a normal serial port, uploads without tricks, and shows `Serial` output in the Serial Monitor. The **USB** connector is the chip's own USB port: a port appears and uploads work, but nothing ever shows in the Serial Monitor and the port name changes after every upload. Both connectors power the board.

---

## 0. Before you arrive (5 min, needs internet)

Do these two things at home. They need ~600 MB of downloads that are slow on venue Wi-Fi.

1. Install **Arduino IDE 2.x** (section 2).
2. Install the **esp32 core** (section 3).

That is it. Everything else (drivers, port, test upload) can be done in the room. If you get stuck, stop and come as you are; a helper will finish it with you.

---

## 1. Drivers (USB-to-serial chip)

The board's **UART** connector talks to the laptop through a Silicon Labs **CP2102N** USB-to-serial chip (the small square chip between the two USB connectors). Driver: [CP210x VCP driver](https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers).

**Rule: plug the board in first. If a port appears (section 5), you don't need a driver.** Only install one if nothing shows up.

Brought your own board with a WCH CH340 / CH9102 chip instead? Drivers: [CH341SER.EXE](https://wch-ic.com/downloads/CH341SER_EXE.html) (Windows), [CH34XSER_MAC](https://wch-ic.com/downloads/CH34XSER_MAC_ZIP.html) (macOS); Linux has it in the kernel but see the `brltty` note below.

### macOS (11 Big Sur to 26 Tahoe, Intel and Apple Silicon)

- macOS ships its own driver for CP210x. The board appears as `/dev/cu.usbserial-XXXX` with **no install**.
- If it does not appear, install the Silicon Labs macOS VCP driver. macOS will block it: go to **System Settings > Privacy & Security**, scroll down, click **Allow** next to the Silicon Labs item; on macOS 15/26 also check **System Settings > General > Login Items & Extensions > Driver Extensions**. Reboot. The port appears as `/dev/cu.SLAB_USBtoUART`.
- Note: never install both Apple's and a vendor driver for the same chip via old `.kext` files from GitHub. Two ports appear and one of them is dead. If you have an old `/Library/Extensions/usbserial.kext`, delete it and reboot.

### Windows 10 / 11

- Plug in the board and wait ~30 s. Windows Update sometimes fetches the CP210x driver automatically, sometimes not.
- Check **Device Manager > Ports (COM & LPT)**. You want a line like **Silicon Labs CP210x USB to UART Bridge (COM3)**.
- If instead you see an **Unknown device** or a yellow warning icon under **Other devices**: download **CP210x Universal Windows Driver** (v11.x, for Windows 10 1803+/11) from the Silicon Labs page, unzip, right-click `silabser.inf` > **Install**. Or in Device Manager right-click the device > **Update driver > Browse my computer** > pick the unzipped folder.
- Unplug and replug the board after installing.

### Linux (Ubuntu / Debian / Fedora / Arch)

Drivers are in the kernel. Two things bite:

1. **Permissions.** Add yourself to the serial group, then **log out and back in** (a reboot is simplest):
   ```bash
   sudo usermod -aG dialout $USER     # Debian/Ubuntu/Mint
   sudo usermod -aG uucp    $USER     # Arch/Manjaro
   ```
2. **brltty steals some boards on Ubuntu 22.04.** The braille-display service `brltty` grabs a few USB-serial IDs (notably CH340, `1a86:7523`), so `/dev/ttyUSB0` appears for a second and vanishes. `dmesg` shows `claimed by ... while 'brltty' sets config`. If you don't use a braille display:
   ```bash
   sudo apt remove brltty
   ```
   then unplug/replug. (Fixed in Ubuntu 23.10+, still present on 22.04 LTS.)

Port will be `/dev/ttyUSB0` (the UART connector, CP210x). If you see `/dev/ttyACM0` instead, the cable is in the **USB** connector; move it to **UART**.

---

## 2. Install Arduino IDE 2.x

1. Go to **https://www.arduino.cc/en/software** and download **Arduino IDE 2.3.x** (2.3.10 as of writing). Pick:
   - **macOS**: **Apple Silicon** for M1/M2/M3/M4 Macs, **Intel** for older Macs. Not sure? Apple menu > **About This Mac** > look at *Chip*.
   - **Windows**: **Win 10 and newer, 64 bits** (`.exe` installer). Needs 64-bit Windows 10 or 11.
   - **Linux**: **AppImage 64 bits**. Then `chmod +x arduino-ide_*.AppImage` and double-click or run it. Ubuntu 22.04+ may need `sudo apt install libfuse2`.
2. Install and open it once. It downloads a small "Arduino AVR Boards" package on first run. Let it finish.
3. Ignore any prompt about "Arduino Cloud" — click **Skip** / close it.

---

## 3. Install the ESP32 core (esp32 by Espressif Systems)

The IDE doesn't know about ESP32 out of the box. You add Espressif's package index, then install it.

1. Open **File > Preferences** (Windows/Linux) or **Arduino IDE > Settings…** (macOS). Shortcut: **Ctrl/Cmd + ,**
2. In **Additional boards manager URLs** paste exactly:
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
   If the field already has a URL, click the icon at the right of the field and put this one on a new line.
3. Click **OK**.
4. Open **Tools > Board > Boards Manager…** (or the boards icon in the left sidebar, **Ctrl/Cmd + Shift + B**).
5. Search **esp32**. Find **esp32 by Espressif Systems**. Pick version **3.3.x** (3.3.11 as of writing) and click **INSTALL**.
6. Wait for "Platform esp32:esp32@3.3.11 installed" in the output panel.

> **This is normal:** the download is 300+ MB (compiler toolchain + libraries) and takes 3–10 minutes. The progress bar may sit still for a while. Don't close the IDE.

Note: do **not** install "Arduino ESP32 Boards" by Arduino — that is a different package for Arduino-branded boards. You want **esp32 by Espressif Systems**.

---

## 4. Select the board

**Tools > Board > esp32 > ESP32S3 Dev Module**

The esp32 menu is very long. Type `S3` in the board-search box at the top of the **Select Board** dropdown, or scroll. Not **ESP32 Dev Module** (that is the classic chip) and not any of the named S3 boards.

Leave every other Tools setting at default. The defaults (**USB CDC On Boot: Disabled**, Upload Speed 921600, Flash Size 4MB, PSRAM Disabled, Partition Scheme Default 4MB with spiffs) work. The board actually has 8 or 16 MB of flash; the 4 MB layout still works and is what the workshop code was tested with. Leave **USB CDC On Boot** disabled, or `Serial` moves to the USB connector and the Serial Monitor on the UART connector goes quiet.

---

## 5. Find the port

Plug the board in with your data cable, into the connector labelled **UART**. Then **Tools > Port**.

| OS | What to pick | Notes |
|---|---|---|
| macOS | `/dev/cu.usbserial-XXXX` or `/dev/cu.SLAB_USBtoUART` | Ignore `Bluetooth-Incoming-Port` and `debug-console`. Always pick `cu.`, not `tty.`. A `cu.usbmodem...` port means the cable is in the **USB** connector. |
| Windows | `COM3`, `COM5`, … | Unplug the board and look again: the one that disappeared is yours. `COM1` is usually the motherboard, not the board. |
| Linux | `/dev/ttyUSB0` | If it's greyed out or "Permission denied": dialout group (section 1). If it flickers: brltty (section 1). `/dev/ttyACM0` means the **USB** connector. |

The IDE 2 board dropdown (top toolbar) also lists ports. It may show "Unknown" next to the port — that's fine, the UART connector does not identify the board over USB. If two ports appear, you have cables in both connectors; unplug the **USB** one.

Check: a port appears when the board is plugged in and vanishes when unplugged. Raise your hand if you can't get here; nothing else works without it.

---

## 6. Test upload: Blink

1. **File > Examples > 01.Basics > Blink**
2. Confirm **Tools > Board** says **ESP32S3 Dev Module** and **Tools > Port** is your port.
3. Click **Upload** (the arrow, top-left) or **Ctrl/Cmd + U**.

> **This is normal:** the first compile takes 1–3 minutes (the IDE builds the whole ESP32 core once and caches it). Later uploads take 10–20 s. Orange text about "Multiple libraries found" or "sketch uses X bytes" is not an error.

4. Watch the output panel. You want:
   ```
   Connecting....
   Chip is ESP32-S3 (QFN56) (revision v0.2)
   ...
   Writing at 0x00010000... (100 %)
   Hard resetting via RTS pin...
   ```
5. Check: the small RGB LED next to the red power LED blinks white once per second. On **ESP32S3 Dev Module**, `LED_BUILTIN` maps to the RGB LED on **GPIO 48**, which is where the LED is on the original DevKitC-1. On the **v1.1** board the LED is on GPIO 38, so the stock Blink example lights nothing; that is not a fault. The workshop code reads the pin from `LED_PIN` in `config.h` (48 by default; change it to 38 if Task 1 lights nothing).

### If the upload hangs at `Connecting......_____....._____`

The board didn't drop into download mode by itself. Do the **BOOT trick**:

1. Click **Upload**.
2. As soon as the output shows `Connecting...`, **press and hold the BOOT button** on the board (labelled `BOOT`, next to the USB connectors; the other button is `RST`).
3. When dots start streaming and you see `Writing at…`, release BOOT.

Still fails with `A fatal error occurred: Failed to connect to ESP32-S3: Wrong boot mode detected`? Hold BOOT, tap RST once while still holding BOOT, release BOOT, then click Upload.

### If uploads fail randomly or stop mid-way

**Tools > Upload Speed > 115200**. Slower but reliable, especially through USB hubs and adapters.

---

## 7. Open the Serial Monitor

**Tools > Serial Monitor** (or the magnifier icon top-right, **Ctrl/Cmd + Shift + M**). Set the baud dropdown on the right to **115200**.

> **This is normal:** the board reboots when you open the Serial Monitor. You may see a few lines starting `rst:0x1 (POWERON)` or `ESP-ROM:esp32s3-...` — that's the chip's boot log, not an error. Garbage characters mean the baud rate is wrong; set it to 115200.

Serial Monitor completely empty although the upload worked? The cable is in the **USB** connector. Move it to **UART** and pick the new port.

You do **not** need to close the Serial Monitor before uploading in IDE 2 — it pauses automatically. But only one program can hold the port: close PuTTY, `screen`, PlatformIO, Thonny, etc.

---

## 8. Offline install (a helper brings the files)

No internet at the venue? Two options:

- **Pre-populated cache.** A helper has a USB stick with the IDE installers and a ready-made `Arduino15` folder containing `packages/esp32`. Install the IDE, quit it, then copy the folder to:
  - macOS: `~/Library/Arduino15` (Finder > **Go** > hold **Option** > **Library**)
  - Windows: `%LOCALAPPDATA%\Arduino15` (= `C:\Users\<you>\AppData\Local\Arduino15`)
  - Linux: `~/.arduino15`
  
  Start the IDE. **ESP32S3 Dev Module** is now in the board list with no download.
- **Phone hotspot.** Install the IDE from the USB stick, then tether to a phone for step 3 only. The core is ~300 MB; do it once, not on everyone's phone at the same time.

---

## Final checklist

- [ ] Arduino IDE 2.3.x opens.
- [ ] **Tools > Board > esp32 > ESP32S3 Dev Module** is selectable.
- [ ] Board plugged in with a data cable into the **UART** connector; a port appears under **Tools > Port** and is selected.
- [ ] Blink uploaded; output ended with `Hard resetting via RTS pin...`.
- [ ] RGB LED blinks (or you know your board is v1.1 and will set `LED_PIN 38`).
- [ ] Serial Monitor open at **115200** shows readable text.
- [ ] You know where the **BOOT** and **RST** buttons are.

Now open `tasks/01_blink/01_blink.ino` and follow its README. Problems: [TROUBLESHOOTING.md](TROUBLESHOOTING.md).

---

## Sources

Checked September 2026.

- Arduino IDE downloads: https://www.arduino.cc/en/software — IDE 2.3.10 release: https://github.com/arduino/arduino-ide/releases/tag/2.3.10
- Espressif install guide (Boards Manager URL): https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html
- esp32 core 3.3.11 release (2026‑07‑22): https://github.com/espressif/arduino-esp32/releases/tag/3.3.11
- Silicon Labs CP210x VCP drivers: https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers — Windows Universal driver release notes (v11.5.0): https://www.silabs.com/documents/public/release-notes/CP210x_Universal_Windows_Driver_ReleaseNotes.txt
- ESP32-S3-DevKitC-1 v1.1 user guide (connectors, RGB LED pin, header pinout): https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide.html
- WCH CH340/CH341 drivers (own boards only): Windows https://wch-ic.com/downloads/CH341SER_EXE.html — macOS https://wch-ic.com/downloads/CH34XSER_MAC_ZIP.html
- Ubuntu brltty / USB-serial conflict: https://bugs.launchpad.net/ubuntu/+source/brltty/+bug/1990189 and https://askubuntu.com/questions/1403705
- esptool boot-mode troubleshooting: https://docs.espressif.com/projects/esptool/en/latest/esp32s3/troubleshooting.html
- Arduino15 folder locations: https://support.arduino.cc/hc/en-us/articles/360018448279-Open-the-Arduino15-folder — offline install by copying it: https://forum.arduino.cc/t/how-to-install-arduino-ide-on-computer-without-internet-access/1089023/4
