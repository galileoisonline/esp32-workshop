# Task 4: web dashboard

The board serves a web page to your phone: live status every second, and buttons that control the LED.

## What you learn

- How a web server on a microcontroller works: a URL maps to a handler function (`server.on("/api/led", ...)`).
- JSON as the shared language between C++ on the board and JavaScript on the phone.
- The `fetch()` polling loop: the page asks `/api/status` once a second and redraws.
- A `loop()` with no `delay()`: `server.handleClient()` plus a `millis()`-driven pattern state machine (`enum Pattern`).

## Steps

1. **File > Open...** and pick `tasks/04_web_dashboard/04_web_dashboard.ino`. You get three tabs:
   the sketch, `config.h`, and `page.h` (the web page, stored as one long string).
2. In `config.h` check your `BOARD_NAME`. Leave `USE_STATION_MODE 0`.
3. Click **Verify**, then **Upload**.
4. Open the **Serial Monitor** (top-right icon), baud **115200**. Read the lines
   `AP started  SSID: ESP32-1  IP: 192.168.4.1` and `Page: http://192.168.4.1/`.
5. On your phone join the Wi-Fi network with your board's name (password `hackhardware`). If asked, choose to
   stay connected without internet. Turn off mobile data if the page will not load.
6. Open the browser and type `http://192.168.4.1/` exactly (with `http://`, not `https://`).
   The page shows your board name, a table of values that updates every second, and `updated 0.3 s ago` under the buttons.
7. Tap **LED off**: the LED stops and the Serial Monitor prints `CMD led off`. Tap **LED on**: it blinks again.
   Tap **sos**: the LED sends three short, three long, three short.
8. Press the board's **BOOT** button while watching the page: the Button row shows `pressed` and the event log
   records it.
9. Add a status field: in `buildStatusJson()` find the comment `// ---- add your own status field here ----` and add
   `json += ",\"adc5\":" + String(analogRead(5));` below it. Upload again, then open `http://192.168.4.1/api/status`
   in the phone browser to see your new number in the raw JSON. (GPIO 5 is on ADC1, which keeps working while
   Wi-Fi is on; GPIO 11-20 are ADC2 and do not.)

Check: tapping LED off and LED on on your phone changes the LED on the board.

## If you finish early

- Show your new field on the page. In `page.h` copy one `<tr>...</tr>` row, give the second cell a new `id`, then
  in `render(d)` add a line like `$('adc5').textContent=d.adc5;`.
- New command `/api/blink?n=3`. Add `server.on("/api/blink", HTTP_GET, handleBlink);` at the comment
  `// ---- add your own command here ----`, read `server.arg("n").toInt()`, and set a counter that `runPattern()`
  counts down. No `delay()`. Add a button in `page.h` that calls `cmd('/api/blink?n=3','blink 3')`.
- Colour command `/api/color?r=40&g=0&b=0`. Same place: read `server.arg("r")`, `"g"`, `"b"` with `.toInt()` into
  three global variables and use them in place of `LED_BRIGHTNESS` where the sketch calls `rgbLedWrite`. Then add
  page buttons `cmd('/api/color?r=40&g=0&b=0','red')` and so on. Try it in the phone browser first.
- Remember the pattern. Use `Preferences.h` (`prefs.begin("esp32"); prefs.putUChar("pattern", pattern);`) so the
  board starts in the last pattern after a reset.

## If it does not work

- Phone says "no internet" and the page never loads: phones fall back to mobile data. Turn mobile data off,
  or choose to stay connected on the Wi-Fi warning. Make sure you typed `http://` (not `https://`).
- `no response for 4 s` after a while: the phone left your board's network (screen lock often does this). Reconnect;
  the page recovers on its own.
- `WebServer.h: No such file`: the wrong board is selected. **Tools > Board > esp32 > ESP32S3 Dev Module**.
- Everything else: [../../docs/TROUBLESHOOTING.md](../../docs/TROUBLESHOOTING.md).
