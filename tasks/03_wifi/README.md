# Task 3: wifi

List every Wi-Fi network in the room, then start the board's own Wi-Fi network that a phone can join.

## What you learn

- The two Wi-Fi roles: station (joins a network) and access point (is the network).
- What RSSI means and how to read a scan result.
- Why the board hosts its own network here (campus Wi-Fi blocks device-to-device traffic; `192.168.4.1` always works).
- Wi-Fi events: the driver calls your function when a phone connects or disconnects.

## Steps

1. **File > Open...** and pick `tasks/03_wifi/03_wifi.ino`.
2. In `config.h` check your `BOARD_NAME`. It becomes the name of your Wi-Fi network, so it has to be
   different from everyone else's in the room. Leave `USE_STATION_MODE 0`.
3. Click **Verify**, then **Upload**.
4. Open the **Serial Monitor** (top-right icon), baud **115200**. Press the board's **RST** button to restart and
   watch phase A: a table of networks with RSSI, channel, `open`/`lock` and name. RSSI closer to 0 is stronger
   (-40 is very good, -70 is fine, -90 is barely there).
5. Watch phase B: `AP started  SSID: ESP32-1  IP: 192.168.4.1` and `password: hackhardware`.
6. On your phone open Wi-Fi settings. Find your board's name, join it with the password `hackhardware`.
   If the phone warns that the network has no internet, choose to stay connected.
7. Look at the Serial Monitor: `client connected` followed by your phone's MAC address. The LED switches from a
   slow 1 s blink to a fast 0.2 s blink. Turn Wi-Fi off on the phone: `client disconnected` and the slow blink is back.

Check: your phone's Wi-Fi list shows your board's name, joining it prints `client connected` and makes the LED blink fast.

## If you finish early

- Strongest network. At the `your code` comment in `scanNetworks()`, loop over the results and remember the index
  with the highest `WiFi.RSSI(i)`, then print `strongest: <name> (<RSSI> dBm)`.
- Hotspot mode. Turn on your phone's personal hotspot, set `USE_STATION_MODE 1` and fill in `WIFI_SSID` /
  `WIFI_PASS` in `config.h`. Upload again and read the IP address the hotspot gives you.
- Client counter on the LED. Blink N times in a burst, where N is `WiFi.softAPgetStationNum()`, then pause.
  Hint: a small counter variable and the same `millis()` pattern as Task 2.

## If it does not work

- Phone cannot see the network: the ESP32 is 2.4 GHz only, and some phones take a few seconds to list new
  networks; pull to refresh. `AP_PASSWORD` must be 8 or more characters or `softAP` fails (Serial says so).
- Scan prints `No networks found`: press **RST** to restart; the first scan after power-up sometimes fails.
- `http://esp32-1.local` does not open: mDNS is unreliable on Android and Windows. Use `192.168.4.1`.
- Everything else: [../../docs/TROUBLESHOOTING.md](../../docs/TROUBLESHOOTING.md).
