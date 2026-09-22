# Challenges

Finished a task early, or done with Task 4? Pick one challenge below. Each takes 15 to 30 minutes starting from the workshop code in `tasks/`. Ask a helper if you are stuck for more than a few minutes. At the end we do a short show-and-tell, so be ready to demo yours in a minute.

Start from the task named in each challenge. Keep `config.h` next to your sketch. `docs/CHEATSHEET.md` has every function you need.

## Shorter

### 1. Morse code board name

Goal: the LED blinks your `BOARD_NAME` in Morse, forever.
Start from: `01_blink`
Hint: one unit = 150 ms. Dot = LED on 1 unit; dash = on 3 units; gap between symbols 1 unit off, between letters 3, between words 7. Write `void dot()`, `void dash()`, then a `switch` (or an array of strings indexed by `c - 'A'`) mapping each letter and digit to a string like `".-"`. Loop over the name with `for (int i = 0; BOARD_NAME[i]; i++)`.
Done when: a helper can read at least the first letter of your name off the LED.

### 2. Reaction-time game

Goal: the LED turns on after a random delay; press BOOT as fast as you can; the Serial Monitor prints your reaction time and your best score.
Start from: `02_serial_button`
Hint: `random(1000, 5000)` for the wait. Record `unsigned long lit = millis()` when the LED goes on, then in `handleButtonPress()` compute `millis() - lit`. Keep a `bestMs` variable. Print "too early" if the button is pressed while the LED is off.
Done when: you and a neighbour have compared best times. Under 200 ms is very good.

### 3. Touch controls brightness or colour

Goal: the harder your finger presses the touch pin, the brighter the LED (or the more its colour shifts).
Start from: `02_serial_button`
Hint: `touchRead(TOUCH_PIN)` is roughly 10000-30000 untouched and rises to about double that when touched on the S3 (print yours to find the range). `int b = map(t, 25000, 45000, 0, 255); b = constrain(b, 0, 255); rgbLedWrite(LED_PIN, b, b, b);`. Replace the blink with this in `loop()`. For colour instead: `rgbLedWrite(LED_PIN, b, 0, 255 - b)` goes from blue to red. `analogWrite` does not work on this LED.
Done when: touching lightly gives a dim LED, pressing firmly gives full brightness (or a clearly different colour).

### 4. Open the page by name

Goal: open the page at `http://<name>.local` instead of `192.168.4.1`.
Start from: `03_wifi` or `04_web_dashboard`
Hint: the sketches already call `MDNS.begin()` with your name in lowercase. Read `startNameService()`, then try the URL. Add `MDNS.addService("http", "tcp", 80);` if it is not there. Print `http://<host>.local` to Serial.
Done when: an iPhone or a Mac/Windows laptop opens `http://<name>.local`. Most Android phones cannot resolve `.local`; that is Android, not you. Demo with a helper's iPhone.

### 5. Strongest Wi-Fi network, live

Goal: the page shows the name and RSSI of the strongest Wi-Fi network nearby, updated every few seconds.
Start from: `04_web_dashboard`
Hint: `WiFi.scanNetworks()` blocks for about 2 s and freezes the page. Use the async form: call `WiFi.scanNetworks(true)` once, then each loop check `int n = WiFi.scanComplete(); if (n >= 0) { /* loop i<n, track max WiFi.RSSI(i) and WiFi.SSID(i) */ WiFi.scanDelete(); WiFi.scanNetworks(true); }`. Add `"strongest":"...","strongest_rssi":...` to `/api/status` at the `add your own status field` comment. Needs `WiFi.mode(WIFI_AP_STA)` so the access point stays up while scanning.
Done when: walking toward the campus router visibly changes the number on your phone.

### 6. `/api/blink?n=5` route and a page button

Goal: a new API route makes the LED blink exactly n times, then return to what it was doing.
Start from: `04_web_dashboard`
Hint: the sketch has the comment `// ---- add your own command here ----`. Register `server.on("/api/blink", HTTP_GET, handleBlink);` there and write `void handleBlink() { int n = server.arg("n").toInt(); blinksLeft = n * 2; sendJson(200, "{\"ok\":true,\"n\":" + String(n) + "}"); }`. No `delay()` in the handler: add a global `int blinksLeft = 0;` and let `runPattern()` toggle the LED and count `blinksLeft` down to zero, then fall back to the current `pattern`. Add a button in `page.h` that calls `cmd('/api/blink?n=3','blink 3')`.
Done when: `http://192.168.4.1/api/blink?n=5` in the phone browser gives exactly 5 blinks, and so does the button.

### 7. Colour picker on the page

Goal: a colour picker on the web page sets the colour of the RGB LED.
Start from: `04_web_dashboard`
Hint: at `// ---- add your own command here ----` register `server.on("/api/color", HTTP_GET, handleColor);` and write `void handleColor() { ledR = server.arg("r").toInt(); ledG = server.arg("g").toInt(); ledB = server.arg("b").toInt(); sendJson(200, "{\"ok\":true}"); }` with three global `uint8_t` variables that the code uses in place of `LED_BRIGHTNESS` wherever it calls `rgbLedWrite`. In `page.h` add `<input type="color" id="col">` and `$('col').oninput = function(e){ var h = e.target.value; cmd('/api/color?r=' + parseInt(h.substr(1,2),16) + '&g=' + parseInt(h.substr(3,2),16) + '&b=' + parseInt(h.substr(5,2),16), 'color'); };`. Full 255 is very bright; cap each value at about 60 in the handler.
Done when: dragging the picker on the phone changes the LED colour in under a second.

## Longer

### 8. Remember `BOARD_NAME` with Preferences

Goal: change the board name from the page and have it survive a power cycle.
Start from: `04_web_dashboard`
Hint: `#include <Preferences.h>; Preferences prefs;`. In `setup()` before starting Wi-Fi: `prefs.begin("esp32", false); String name = prefs.getString("name", BOARD_NAME);` and use `name` everywhere the macro `BOARD_NAME` was used (Wi-Fi name, JSON, banner). Add `server.on("/api/name", ...)` reading `server.arg("name")`, calling `prefs.putString("name", name)` and then `ESP.restart()` so the network renames itself.
Done when: you set a new name from your phone, unplug and replug the board, and the Wi-Fi network has the new name.

### 9. Temperature graph with `<canvas>`

Goal: a live line chart of temperature (or touch) for the last 60 seconds on the page.
Start from: `04_web_dashboard` (edit `page.h` only; pure HTML/JS, no C++)
Hint: add `<canvas id="chart" width="320" height="120"></canvas>`. In `render(d)`: `hist.push(d.temp_c); if (hist.length > 60) hist.shift();` then draw: `var c = $('chart').getContext('2d'); c.clearRect(0,0,320,120); c.beginPath(); hist.forEach(function(v,i){ c.lineTo(i*320/60, 120 - (v - min) / (max - min) * 120); }); c.stroke();`. Compute `min` and `max` from `hist` each frame.
Done when: holding your thumb on the chip for 10 s produces a visible rising curve.

### 10. Deep sleep with a boot counter

Goal: the board sleeps for 10 s, wakes up, prints how many times it has booted, and goes back to sleep. The counter survives sleep.
Start from: `02_serial_button`
Hint: `RTC_DATA_ATTR int bootCount = 0;` at the top of the file keeps a variable in RTC memory, which stays powered during deep sleep. In `setup()`: `bootCount++; Serial.printf("boot #%d\n", bootCount); Serial.flush(); esp_sleep_enable_timer_wakeup(10ULL * 1000000ULL); esp_deep_sleep_start();`. On wake, `setup()` runs from scratch; `loop()` never runs. Print `esp_sleep_get_wakeup_cause()` to see why it woke.
Extra: wake on touch instead of a timer. Before `esp_deep_sleep_start()` call `touchSleepWakeUpEnable(T4, 40000);` (pick a threshold between your untouched and touched readings). On the S3 the board wakes when the touch value goes **above** the threshold, the opposite of the classic ESP32.
Note: a sleeping board cannot take uploads. Hold **BOOT** while uploading, or press RST and upload in the first second.
Done when: the Serial Monitor shows `boot #1`, `boot #2`, `boot #3` ten seconds apart, and the count survives.

### 11. Rate-limit the API

Goal: a classmate flooding `/api/led?state=toggle` from their laptop cannot take over your LED; your board answers `429 Too Many Requests` after 5 requests per second from one client.
Start from: `04_web_dashboard`
Hint: in the `/api/led` handler get the caller with `IPAddress ip = server.client().remoteIP();`. Keep a small array of `{IPAddress ip; unsigned long windowStart; int count;}` (max 4 entries; the access point only allows 4 clients). If `count > 5` within a 1000 ms window: `sendJson(429, "{\"ok\":false,\"error\":\"slow down\"}"); return;`. Attack from a laptop joined to your network: `for i in $(seq 50); do curl -s "http://192.168.4.1/api/led?state=toggle"; done` (or a `fetch` loop in the browser console). Count the 429s in the event log.
Done when: the attacker's loop shows mostly 429s while your own phone still controls the LED normally.

### 12. Two boards: BOOT on one toggles the LED on the other

Goal: press BOOT on board A and the LED on board B toggles, over ESP-NOW. Needs a partner.
Start from: `05_esp_now`
Hint: add `uint8_t type;` as the first field of `struct Message` (0 = status, 1 = toggle) and update `sizeof` checks on both boards. In `checkButton()`, when pressed, fill a `Message` with `type = 1` and `esp_now_send(BROADCAST_ADDR, ...)`. In `onMessageReceived()`, if `m.type == 1` and the name is not your own, call `setLedEnabled(pattern == OFF)`. Both boards on channel 1 with `WiFi.mode(WIFI_AP_STA)`, which the sketch already does.
Done when: pressing BOOT on either board toggles the LED on the other one within a fraction of a second.

### 13. Captive portal

Goal: when a phone joins your board's network, the page pops up automatically, with no typing an IP.
Start from: `04_web_dashboard`
Hint: `#include <DNSServer.h>; DNSServer dns;` (ships with the core). After the access point starts: `dns.start(53, "*", WiFi.softAPIP());` and in `loop()`: `dns.processNextRequest();`. Then answer every unknown URL with a redirect: replace the body of `handleNotFound()` with `server.sendHeader("Location", "http://192.168.4.1/", true); server.send(302, "text/plain", "");`. Phones probe URLs like `/generate_204` (Android) and `/hotspot-detect.html` (iOS); the redirect makes them show the "sign in to network" sheet with your page inside.
Done when: forgetting the network and rejoining it on a phone pops the page without opening the browser.

### 14. Temperature alarm

Goal: if the chip temperature crosses a threshold, the LED switches to SOS and the page shows an alarm line until you acknowledge it from the phone.
Start from: `04_web_dashboard`
Hint: the sketch already has an `SOS` pattern and `setPattern(Pattern p)`. In `loop()` call `setPattern(SOS); alarm = true;` when `temperatureRead() > THRESHOLD` (pick about 3 C above your idle reading) and `alarm` is still false. Add `"alarm":` + `String(alarm ? "true" : "false")` to the JSON in `buildStatusJson()` and a route `/api/ack` that clears `alarm` and calls `setPattern(BLINK)`. In `page.h`, show or hide a `<p>` based on `d.alarm`. Warm the chip with your thumb to trigger it.
Done when: thumb on chip gives SOS plus the alarm line; tapping an acknowledge button on the phone returns to normal.

### 15. A real sensor

Goal: add one physical sensor and show its reading on the page as a new row.
Start from: `04_web_dashboard` (ask the helper table what sensors are available)
Hint: easiest with no library: a potentiometer (outer legs to 3V3 and GND, middle to GPIO 5), then `analogRead(5)`; or a photoresistor plus a 10 k resistor as a voltage divider on GPIO 5. GPIO 1-10 are ADC1 and keep working while Wi-Fi is on; GPIO 11-20 (ADC2) do not. Add the reading at the `// ---- add your own status field here ----` comment, then a row in `page.h`. Sensors that need a library (BME280, DHT22) require **Sketch > Include Library > Manage Libraries**, which needs internet; ask first.
Note: 3.3 V only. Never connect a 5 V sensor output to an ESP32 pin.
Done when: turning the knob (or covering the light sensor) moves a number on your phone in under a second.

Finished one? Tell a helper, show a neighbour, and put your board name on the whiteboard under the challenge number. Then pick another.
