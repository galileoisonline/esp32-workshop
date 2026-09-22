# Task 5: esp-now

Boards talk to each other: every board broadcasts a small status message over ESP-NOW and lists every other board it can hear on its web page.

## What you learn

- ESP-NOW: radio-to-radio messages between ESP32s with no router, no IP address, no pairing.
- Packing a C `struct` into bytes and sending it, and why both sides must define the same struct.
- Receive callbacks (code that runs when a message arrives) and keeping the callback short.
- A peer table with expiry, and why the access point and ESP-NOW must share a Wi-Fi channel.

## Steps

1. Find a partner (or two) who finished Task 4. This task needs at least two boards.
2. **File > Open...** and pick `tasks/05_esp_now/05_esp_now.ino`.
3. In `config.h` make sure your `BOARD_NAME` is different from your partner's. The table is keyed by name,
   so two boards called `ESP32-1` would look like one board.
4. Click **Verify**, then **Upload**. Everyone in the group uploads the same sketch.
5. Open the **Serial Monitor** (top-right icon), baud **115200**. You should see `ESP-NOW ready, my MAC: ...`, and
   within 5 seconds `boards nearby: 1` with a row for your partner's name, age, temperature, LED state and RSSI.
6. On your phone join your board's Wi-Fi network and open `http://192.168.4.1/`. The **Boards nearby** table lists
   your partner's board and the event log prints `heard ESP32-2` (their name).
7. Ask your partner to tap **LED off** on their phone. Within about 2 s the LED column for their board changes on
   your page. You are watching their hardware through the radio.
8. Have your partner unplug their board. After 10 s their row disappears and the log says `lost ESP32-2`.

Check: the Boards nearby table on your page shows at least one other board's name with a live temperature.

## If you finish early

- Add a field. Put `uint32_t touch;` at the end of `struct Message` (S3 touch values go above 65535), fill it in
  `broadcastStatus()`, add it to `buildPeersJson()` and a column in `page.h`. Everyone in your group must upload
  the new struct or their messages will be ignored (`len != sizeof(Message)`).
- Remote command. Add a second message type: when you press BOOT, broadcast a "flash" message, and every board
  that hears it sets its pattern to `SOS` for 5 s. Hint: add a `uint8_t type;` field as the first byte.
- Remote colour. Add `uint8_t r, g, b;` to the message and a `/api/color?r=&g=&b=` route (see Task 4's extensions).
  Every board that hears you shows your colour on its own RGB LED with `rgbLedWrite`.
- Ping-pong. Two boards send a message only when they have just received one, incrementing a counter.
  Print the counter and the round-trip time to see how fast ESP-NOW is.

## If it does not work

- `boards nearby: 0` forever: both boards must be on the same channel (`WIFI_CHANNEL = 1` in both sketches, and
  `USE_STATION_MODE` must stay `0`; joining a hotspot moves you to its channel). Also check the partner's board is
  running Task 5, not Task 4.
- Partner appears, then vanishes every few seconds: packet loss. Move the boards closer, away from the laptop
  charger, and check RSSI (worse than -85 dBm is unreliable).
- `broadcast failed` in Serial: ESP-NOW did not initialise, usually a Wi-Fi start race. Press **RST** to reset.
- Everything else: [../../docs/TROUBLESHOOTING.md](../../docs/TROUBLESHOOTING.md).
