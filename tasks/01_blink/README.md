# Task 1: blink

Make the onboard RGB LED blink, then change how fast it blinks.

## What you learn

- How a sketch is structured: `setup()` runs once, `loop()` runs forever.
- How to drive the board's RGB LED with `rgbLedWrite(pin, r, g, b)` (and, for a plain LED, `pinMode()` and `digitalWrite()`).
- How `delay()` pauses the program (and why Task 2 replaces it).
- How to compile, upload and read the Serial Monitor in Arduino IDE 2.

## Steps

1. In Arduino IDE 2 open **File > Open...** and pick `tasks/01_blink/01_blink.ino`.
   The IDE opens two tabs: `01_blink.ino` and `config.h`.
2. Click the `config.h` tab. Change `#define BOARD_NAME "ESP32-1"` to a name of your own
   (letters, digits, dashes; max 11 characters). Leave `LED_PIN 38` for now (see step 7 if nothing lights).
3. Plug the cable into the USB-C connector labelled **UART** on the board, not the one labelled **USB**.
   Select the board: **Tools > Board > esp32 > ESP32S3 Dev Module**.
   Select the port: **Tools > Port**, the one that looks like `/dev/cu.usbserial-...`, `/dev/cu.SLAB_USBtoUART` or `COMx`.
4. Click **Verify** (the tick, top-left). The first compile takes a minute or two; that is normal.
   The output panel should end with `Sketch uses ... bytes`.
5. Click **Upload** (the arrow). If the output sits on `Connecting...`, hold the **BOOT** button on the board
   until dots start streaming. Wait for `Hard resetting via RTS pin...`.
6. Open the **Serial Monitor** (magnifying-glass icon, top-right) and set the baud dropdown to **115200**.
   You should see `[ESP32-1] Task 1: blink` (with your name) followed by `led on` / `led off` lines.
7. Look at the board. The LED that blinks is a small white square next to the always-on red power LED.
   If it never lights but the Serial Monitor shows `led on` / `led off`, your board is the original revision:
   in `config.h` change `LED_PIN 38` to `LED_PIN 48` and upload again.
8. In `01_blink.ino` find `const unsigned long BLINK_MS = 500;`. Change `500` to `100`, click **Upload**
   again, and watch the LED speed up. Try `2000`.

Check: the RGB LED blinks at a rate you picked, and the Serial Monitor shows your board name in the first line.

## If you finish early

- Colour. The LED is an RGB LED. `setLed()` at the bottom of the sketch turns it on with equal red, green and blue
  (`rgbLedWrite(LED_PIN, v, v, v)`, white) and off with all zeros. Try `rgbLedWrite(LED_PIN, 40, 0, 0)` for red,
  `0, 40, 0` for green, `0, 0, 40` for blue. Change `LED_BRIGHTNESS` in `config.h` to make it dimmer or brighter
  (0-255; 255 is painful to look at).
- Two colours. Instead of on/off, alternate between two colours: red for `BLINK_MS`, then blue for `BLINK_MS`.
- SOS. Morse SOS is three short, three long, three short. Write `void dot()` (LED on 150 ms, off 150 ms)
  and `void dash()` (on 450 ms, off 150 ms), then call them in order inside `loop()` with a longer pause
  between repeats.
- Heartbeat. Two quick blinks (on 80 ms, off 80 ms, on 80 ms) followed by a long rest (about 800 ms).
- Breathe. A `for` loop that takes `b` from 0 to 40 and back to 0, calling `rgbLedWrite(LED_PIN, b, b, b)` with
  a few ms of `delay` per step. (`analogWrite` cannot dim this LED; brightness is the r, g, b values.)

## If it does not work

- LED never lights but the upload succeeded and Serial prints `led on`: your board is v1.1, whose LED is on
  GPIO 38. Set `LED_PIN 38` in `config.h`. Both board versions are in the room and look identical.
- Upload works but the Serial Monitor stays empty: the cable is in the **USB** connector. Move it to **UART**.
- `Failed to connect to ESP32-S3: Wrong boot mode`: hold **BOOT** while the IDE prints `Connecting...`, then release.
- Garbage characters in the Serial Monitor: the baud dropdown must say **115200**.
- Everything else: [../../docs/TROUBLESHOOTING.md](../../docs/TROUBLESHOOTING.md).
