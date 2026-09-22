# Facilitator guide

You can run this workshop having never seen the material before. Read section 1 now, section 5 tonight, and keep section 4 open on your laptop during the event.

## 1. Read this first

- The shape of it. Each participant's ESP32 board goes from a blinking LED, to printing its status over serial, to hosting its own Wi-Fi network, to a web page on the participant's phone that shows the board's status and controls the LED. Fast finishers make boards talk to each other over ESP-NOW.
- Five tasks, one folder each under `tasks/`. Participants open the `.ino`, upload it, watch it work, then edit one clearly marked line or block. All shared settings (`BOARD_NAME`, `LED_PIN`, `AP_PASSWORD`, and so on) live in `config.h` next to each sketch.
- The boards are **ESP32-S3-DevKitC-1** (Espressif's official S3 board). Two things about it every helper must know: it has two USB-C connectors, labelled UART and USB, and students must use **UART** (the USB one uploads but shows no Serial output); and its LED is an addressable RGB LED on GPIO 48 on the original board or GPIO 38 on v1.1, and both versions are in the room and look identical. If nothing lights, `LED_PIN 38` in `config.h`.
- No extra hardware, no internet dependency. Everything uses what is on the board (RGB LED on GPIO 48/38, BOOT button on GPIO 0, internal temperature sensor, touch pin GPIO 4, Wi-Fi). The board hosts its own Wi-Fi network (named `BOARD_NAME`, password `hackhardware`, page at `http://192.168.4.1`). Never rely on campus Wi-Fi.
- The four things that eat time: (1) laptops that cannot see the board: charge-only cables, CP210x driver; (2) cable in the USB connector instead of UART: upload works, Serial Monitor empty; (3) `Wrong boot mode`: hold BOOT during upload; (4) phones that will not open the page: turn off mobile data, use `http://`. Every helper must know these cold. They are in `docs/TROUBLESHOOTING.md`.
- Your job during the event is pacing, not debugging. Helpers debug. You watch the clock, call checks, and cut scope using the rules in section 4.
- Definition of success: every participant turns their LED on and off from their phone (the Task 4 check). Everything after that is a bonus.

## 2. Prep checklist

### Night before

Hardware

- [ ] ESP32-S3-DevKitC-1 boards (original or v1.1; note which are which if you can, the LED pin differs): one per participant plus 15% spares (some arrive dead or with weak regulators).
- [ ] USB-C data cables, one per board plus spares. Both connectors on the board are USB-C. Test each cable by uploading Blink through the UART connector; charge-only cables look identical. Mark tested cables with a tape flag.
- [ ] Adapters: USB-C to USB-A for laptops with only USB-A ports, or USB-C to USB-A cables. Bring 10 or more.
- [ ] Power strips and extension cords: one outlet per two participants.
- [ ] A backup portable hotspot (phone or MiFi) on 2.4 GHz with a simple password, for the station-mode option and for emergency core downloads.
- [ ] Jumper wires, a handful of LEDs and 220 ohm resistors (for the 150-minute hardware variant and for challenges that want a plain LED).
- [ ] Tape and a marker to tag dead boards and bad cables.

Software on a USB stick (assume the venue has no reliable internet)

- [ ] Arduino IDE 2.3.x installers: macOS Apple Silicon, macOS Intel, Windows 64-bit `.exe`, Linux AppImage. From https://www.arduino.cc/en/software.
- [ ] Driver installers: Silicon Labs CP210x Universal Windows Driver and macOS VCP driver (the DevKitC-1's UART connector uses a CP2102N). Links in `docs/SETUP.md` section 1. CH340 drivers only matter for people who bring their own board.
- [ ] A pre-populated esp32 core. On your own machine, install esp32 3.3.x via Boards Manager, quit the IDE, then copy the whole `Arduino15` folder to the stick:
  - macOS: `~/Library/Arduino15`
  - Windows: `%LOCALAPPDATA%\Arduino15`
  - Linux: `~/.arduino15`

  Participants who cannot download copy `packages/esp32` (and `package_esp32_index.json`) into their own `Arduino15`, restart the IDE, and **ESP32S3 Dev Module** appears. The toolchain binaries differ per OS, so ideally prepare one `Arduino15` from each OS you expect (mac ARM, mac Intel, Windows). At minimum, do macOS ARM and Windows. About 600 MB each.
- [ ] A zip of this repo (participants cannot `git clone` without internet).
- [ ] The slide deck. Test it opens on the presenter laptop.

Paper

- [ ] `docs/CHEATSHEET.md` printed, one per participant.
- [ ] `docs/TROUBLESHOOTING.md` printed, one per helper.
- [ ] Helper cards (see section 7): index cards with the three fixes.
- [ ] Sign-in sheet and feedback QR code (or short URL) for the wrap-up.

Demo board

- [ ] A pre-flashed board running Task 4 (`tasks/04_web_dashboard`), or Task 5 if you have a partner board, with `BOARD_NAME` set to something like `DEMO`. Test: connect your phone to `DEMO`, open `http://192.168.4.1`, tap LED off and LED on. You show this at the start and use it to demo Task 4.
- [ ] Your own laptop fully set up (IDE, core, drivers) and able to upload to a board in under 30 s. You live-code from it.

### One hour before

- [ ] Projector: HDMI plus USB-C to HDMI adapter, tested with the presenter laptop. Slides open. In the IDE, **View > Zoom In** three times so the back row can read it.
- [ ] Power strips laid out, one board and cable per seat.
- [ ] The pre-flashed board powered on (a USB charger is fine) and visible on your phone.
- [ ] Helpers briefed (section 7), each with their own flashed board, a helper card, and a spare tested cable.
- [ ] Write on the whiteboard or first slide: the Boards Manager URL, board **ESP32S3 Dev Module**, "cable in the **UART** connector", `hackhardware` / `http://192.168.4.1`, "Wrong boot mode? Hold BOOT." and "LED dark? LED_PIN 38."
- [ ] Set a visible timer.

## 3. Room setup

- Tables in pods of 4 to 6, not rows. Participants debug each other's cables. One helper per one or two pods.
- Every seat has: a power outlet within reach, board, tested cable, printed cheat sheet.
- Facilitator at the front with the projector, the pre-flashed board, and a parts table: spare boards, cables, adapters, USB stick, jumper LEDs.
- Wi-Fi reality check. 30 boards will each create a 2.4 GHz access point, all on channel 1. It works, but phones' Wi-Fi lists get long and the page may lag. Tell participants to stay near their own board and to pick a unique `BOARD_NAME` so they can find their network.
- The access point accepts 4 clients by default, so one phone per board. A laptop can connect to the board instead of a phone.
- If the room is huge or has metal walls, expect some phones to see only nearby boards. That is fine.

## 4. Run of show (120 minutes)

The clock starts when doors open. The deck is 32 slides: 1 title, 2 plan, 3 setup check. Start the background block at 0:10 on time even if setup is not finished.

| Clock | Block | Slides | You do | Participants do | Check to call |
|---|---|---|---|---|---|
| 0:00 | Setup check | 3 | Greet. Point at the whiteboard. Say out loud: "two USB-C connectors, use the one labelled UART." Helpers walk the room confirming the cable is in UART and **Tools > Port** shows the board. Hand the USB stick to anyone with no IDE. | Install IDE and core (`docs/SETUP.md`). Upload Blink. | "Raise your hand if you do not see a port." |
| 0:10 | Background | 4-13 | Show the pre-flashed board: phone, page, LED goes on and off. "In two hours yours does this." Microcontroller vs computer. Board tour: the two USB-C connectors (UART is ours), RST, BOOT, the RGB LED on GPIO 48 or 38, 3.3 V max. Compile, flash, runs forever. `setup()`/`loop()`. | Listen. Unzip the repo. | Everyone has `tasks/` open in Finder or Explorer. |
| 0:30 | Task 1: blink | 14 (concept 15) | Live: open `01_blink.ino`, edit `BOARD_NAME` in `config.h`, Upload, LED blinks, Serial banner. Change `BLINK_MS`. Say: "if your Serial works but the LED is dark, change 48 to 38." Then let them go. | Same. Then change the rate, try colour / SOS / heartbeat / breathe. | "Hold your board up if it is blinking at a rate you chose." |
| 0:45 | Task 2: serial and button | 16 (concepts 17-20) | Live: open `02_serial_button.ino`, Serial Monitor at 115200, press BOOT, `EVENT button pressed`. Touch pin 4. Slides 17-20: Serial, pull-ups, why `millis()` not `delay()`. | Upload. Watch the status line. Add a field. Serial Plotter on touch. | "Press BOOT. Does a line appear?" |
| 1:00 | Break | | Helpers fix stragglers. You set up Task 3 on the projector. | Stretch. | Everyone at least at the Task 1 check. |
| 1:05 | Task 3: wifi | 21 (concept 22) | Live: upload `03_wifi`. Show the scan table (RSSI, channels). Then `AP started`; show your board's name in your phone's Wi-Fi list; join; `client connected` prints. Slide 22: station vs access point, 192.168.4.1. | Upload. Read the scan. Find their network on their phone. | "Thumbs up when Serial says client connected." |
| 1:20 | Task 4: web dashboard | 23 (concepts 24-26) | Live: upload `04_web_dashboard`, join the network on the phone, turn off mobile data, `http://192.168.4.1`, tap LED off and LED on. Slides 24-26: request, route, response; the fetch loop; make it yours. Show the two `add your own` comments. | Upload. Page on phone. Add a status field, add a command. | "Turn your LED on and off from your phone." This is the success criterion. Take the room photo here. |
| 1:45 | Task 5 or challenges | 27-29 | Announce: fast finishers do `05_esp_now`; everyone else picks from `CHALLENGES.md`. Circulate. Pick three volunteers for show-and-tell. | Build. | Peer tables filling up with each other's board names. |
| 2:00 | Wrap-up | 30-32 | Three one-minute show-and-tells. Where to go next, get a board, thanks. Wrap-up text (section 8). Feedback QR. | Show off. Fill in feedback. | Photos, boards returned or taken home (decide beforehand). |

The wrap-up runs about 10 minutes past 2:00. If the room is booked hard until 2:00, start Task 5 / challenges at 1:40 by trimming Task 4's "add a command" step.

### Pacing rules: if you are behind, cut this

| If at this minute | you are still on | Cut |
|---|---|---|
| 0:10 | setup check, more than 25% have no port | Start the background block anyway. Helpers keep working the room quietly. Pair no-port students with a neighbour for Task 1 (one laptop, two people). |
| 0:30 | background | Skip slides 10-13 except `setup()`/`loop()`. Go straight to live-coding Task 1. |
| 0:45 | Task 1 | Skip the "if you finish early" ideas. Move on; SOS and heartbeat become challenges. |
| 1:00 | Task 2 | Cut the break. Skip Serial Plotter. Do not explain debounce; say "the code ignores button noise for you." |
| 1:20 | Task 3 | Skip the scan discussion (say "it lists the Wi-Fi in the room, look later"). Skip station mode. Skip mDNS. |
| 1:45 | Task 4 | Skip "add a command." Participants change `BOARD_NAME` only. Everyone must reach the LED check; give it until 1:55. |
| 1:55 | Task 4 | Cut Task 5 and challenges. Go straight to wrap-up at 2:00. |
| 2:05 | anything | Skip show-and-tell. Wrap-up text plus feedback QR. End. |

Ahead of schedule? Give Task 4 the extra time; the customisation step is where the learning is. Then a longer Task 5.

### Variants

- 90 minutes: cut Task 5 and the break. Task 4 is 20 minutes: participants change `BOARD_NAME` and add one command only.
- 150 minutes: add 15 minutes of hardware after Task 2 (external LED plus 220 ohm on a spare GPIO, then a potentiometer on GPIO 5 with `analogRead`; GPIO 1-10 are ADC1 and work with Wi-Fi on). Task 5 gets 30 minutes.

## 5. Per-task notes

### Task 1: blink (`tasks/01_blink/`)

Talking points

- A microcontroller runs one program, forever, from power-on. No OS, no windows, no "quit".
- `setup()` once, `loop()` forever. Every Arduino program is exactly this.
- `digitalWrite(pin, HIGH)` puts 3.3 V on a pin. This board's LED is different: an addressable RGB LED with one data pin (GPIO 48, or 38 on v1.1), driven with `rgbLedWrite(LED_PIN, r, g, b)`. `setLed()` at the bottom of the sketch hides that. Brightness is the r, g, b values; `LED_BRIGHTNESS` in `config.h` is 40 because 255 hurts.
- `delay()` freezes everything. Fine here; Task 2 fixes it.
- "Changing one number and uploading again is most of embedded programming."

Demo. Upload, LED blinks, Serial banner shows your board name. Change `BLINK_MS` from 500 to 100, upload again: fast blinking. The upload takes 15 s; say so. Then change the colour to red on the projector (`rgbLedWrite(LED_PIN, 40, 0, 0)`): it costs 10 seconds and shows what the LED can do.

Stuck on, one-line fix

- `Connecting......` hangs: hold BOOT until dots stream.
- LED does not light but Serial prints `led on`: v1.1 board. `LED_PIN 38` in `config.h`, upload. This is the first thing to try; expect several per room.
- Upload works but Serial is empty: cable in the USB connector. Move it to UART, reselect the port.
- Serial shows garbage: baud dropdown to 115200.
- First compile takes 2 minutes: normal, say it out loud before it happens.

Check to look for: boards blinking at different rates around the room. If they are all at 500 ms, nobody edited anything; push them.

### Task 2: serial and button (`tasks/02_serial_button/`)

Talking points

- The Serial Monitor is your window into the chip. `Serial.print` is `printf` for hardware.
- `INPUT_PULLUP`: the pin idles at HIGH; pressing the button connects it to ground, so LOW. Pressed = 0. Counter-intuitive; say it twice.
- Edge vs level: react to the moment of pressing, not "is it held".
- `millis()` is a stopwatch since boot. "If enough time has passed, do the thing" instead of `delay()`. This is the single most important habit in the workshop.
- `temperatureRead()` is the chip's internal die temperature. It is rough (plus or minus 5 C) and reads warm. It is still a sensor.

Demo. Open the Serial Monitor, the status line ticks every second. Press BOOT: `EVENT button pressed`, blinking stops. Touch GPIO 4 with a finger (left header, fourth pin from the top): `touch=` rises from roughly 25000 to 40000 or more. Say it: on the S3 the touch value goes up, not down like the classic ESP32; students who read older tutorials will expect a drop. Then **Tools > Serial Plotter** on the touch value: a live graph of a finger.

Stuck on, one-line fix

- Opening the Serial Monitor rebooted the board: normal, that is what it does.
- Nothing prints: cable in the USB connector (move to UART), wrong port, baud, or press RST.
- "Button does not work": they are pressing RST (resets the board) instead of BOOT.
- `touch=0` or constant: wrong pin; T4 is GPIO 4, left side. "It does not drop": right, it rises.

Check: every Serial Monitor scrolling. Ask "who has touched pin 4?" You want everyone to.

### Task 3: wifi (`tasks/03_wifi/`)

Talking points

- The ESP32 has a full Wi-Fi radio for a few dollars. It can be a client (station, like your laptop) or a base station (access point, like your router).
- Scan first: it needs no credentials and proves the radio works. RSSI is in dBm: -40 is next to you, -90 is barely there.
- We use access-point mode because campus Wi-Fi blocks device-to-device traffic. Your board becomes its own network: named `BOARD_NAME`, password `hackhardware`, IP always `192.168.4.1`.
- WPA2 requires an 8+ character password; that is why `AP_PASSWORD` is not `1234`.
- 2.4 GHz only. Your phone will show it; a 5 GHz-only laptop dongle will not.
- Wi-Fi events: the driver calls your function when a phone joins or leaves. No polling needed.

Demo. The scan table appears on the projector listing every network in the building. Then your board's name appears in your phone's Wi-Fi list. Connect. Serial prints `client connected` with the phone's MAC, and the LED speeds up. Disconnect. `client disconnected`, LED slows.

Stuck on, one-line fix

- Phone does not show the network: wait 10 s, pull down to refresh; check `BOARD_NAME` is unique (six `ESP32-1`s in the room is confusing but not broken).
- `Brownout detector was triggered` when Wi-Fi starts: power; different cable or port.
- Password rejected: `AP_PASSWORD` shorter than 8 characters.
- Station mode never connects: campus Wi-Fi. Use the hotspot or go back to `USE_STATION_MODE 0`.

Check: phones out, thumbs up. Do not let anyone move to Task 4 without seeing their network; Task 4 debugging is much harder if Wi-Fi is not proven.

### Task 4: web dashboard (`tasks/04_web_dashboard/`)

Talking points

- HTTP in one sentence: the phone asks for a path (`/api/status`), the board runs a function for that path and sends text back. `server.on(path, function)`.
- JSON is just text with quotes and braces. The board builds it with string concatenation; the phone's JavaScript parses it.
- The page is one HTML file stored inside the sketch (`page.h`). The board is the web server. No cloud, no internet.
- `fetch('/api/status')` every second is polling. Simple, good enough.
- `server.handleClient()` in `loop()`, and no `delay()`, or the page freezes. This is why Task 2 mattered.

Demo. Phone screen on the projector if you can (or hold it up to the camera): tap LED off, the pre-flashed board's LED stops. Tap LED on. Tap sos. Then edit the sketch on screen: add `json += ",\"secret\":42";` at the `add your own status field` comment, upload, open `/api/status`: the new number is there. That is their customisation task.

Stuck on, one-line fix

- Page will not load: mobile data off; `http://` not `https://`; connected to their own board's network.
- "No internet, stay connected?": tap keep / stay connected.
- Loads but numbers frozen: mobile data crept back on; reload.
- Added a field, JSON breaks: missing comma or quote. Open `http://192.168.4.1/api/status` directly in the browser; the error is visible.
- Everything slow: 30 access points on one channel. Normal.
- `WebServer.h: No such file`: wrong board selected.

Check: the LED goes on and off from the phone. Call it loudly. Take the room photo. Anyone not there by 1:40 gets a helper glued to them.

### Task 5: esp-now (`tasks/05_esp_now/`)

Talking points

- ESP-NOW: Espressif's radio protocol with no router, no IP, no connection. Broadcast a message, anyone listening gets it. About 1 ms latency.
- We broadcast `{name, uptime, temp, led}` every 2 s to `FF:FF:FF:FF:FF:FF`. Every board keeps a table of who it heard in the last 10 s.
- Works alongside the access point because both are on channel 1 (`WiFi.mode(WIFI_AP_STA)`).
- Both sides must define the same struct, or the bytes mean different things.

Demo. Your page's "Boards nearby" table fills with the room's board names. Ask someone to power off their board: 10 s later it drops off.

Stuck on, one-line fix

- Compile error about `esp_now_recv_cb_t`: callback must be `(const esp_now_recv_info_t*, const uint8_t*, int)` (core 3.x).
- No peers: both boards must be on the same channel; the sketch fixes channel 1. Someone's `BOARD_NAME` over 11 characters gets cut off; shorten it.
- Peers appear and vanish: distance or interference; more or less normal.

Check: two or more pages list each other. Everyone else is on `CHALLENGES.md` and does not need this.

## 6. Live-coding tips

- **View > Zoom In** three times before you start. Keep the Serial Monitor docked so it is always visible.
- Say the three normal-but-scary things before they happen: "the first compile takes two minutes", "the IDE will download 300 MB", "opening the Serial Monitor reboots the board."
- When something fails on the projector, narrate the fix from TROUBLESHOOTING.md. A live failure fixed calmly is the best teaching moment of the day.
- Never type long code live. Open the task file, point at the `add your own` comment, change one line.
- Call checks with a physical gesture (hold up the board, thumbs up, phones in the air). You get an instant count of who is behind.

## 7. Helper roles

- Ratio: one helper per 6 to 8 participants. Fewer than that and the setup check overruns.
- Before the event each helper: does the full SETUP on their own laptop, flashes all five tasks to their own board, and reads `docs/TROUBLESHOOTING.md` top to bottom. Their board is the loaner when a participant's board is dead.
- Helper card (index card, each helper holds one):
  ```
  0. Cable in the UART connector, not USB. (Upload OK but Serial empty = wrong connector.)
  1. No port?          Data cable. Direct USB. CP210x driver (SETUP section 1).
  2. Wrong boot mode?  Hold BOOT while "Connecting...". Or Upload Speed 115200.
  3. No page?          Mobile data off. http://192.168.4.1 (not https). Joined your own board's network?
  Brownout: cable/port.   LED dark: LED_PIN 38 (v1.1 board).   Garbage: 115200.   Touch goes UP on the S3.
  ```
- During the setup check, helpers walk their pods and confirm each cable is in the UART connector and each laptop shows a port. Anyone without one gets the USB stick and one-to-one attention immediately; do not let it wait.
- During tasks, helpers watch for the check, not for hands. Hands come up late. Someone staring at a static screen for 2 minutes is stuck.
- Three-minute rule: if a helper cannot fix a laptop in 3 minutes, they swap in a working board or pair the participant with a neighbour, and come back later. Never let one laptop absorb a helper for 15 minutes during a task.
- Helpers do not type on participants' keyboards. Point; the participant types.
- One helper owns the parts table and tracks loaned boards and cables.

## 8. Wrap-up (about a minute)

> Two hours ago that board was doing nothing. Right now it blinks, it reports its status, it runs its own Wi-Fi network, and it takes commands from your phone. You wrote the code that does that.
>
> Everything you did today is the foundation of every connected device you own: a loop that never stops, pins that go high and low, sensors turned into numbers, numbers turned into JSON, and a web page that talks to hardware.
>
> Three things to do next. One: keep the code. The repo link is on the screen, and `CHALLENGES.md` has more ideas. Two: get your own board. They cost a few dollars; the slide tells you what to look for and, most importantly, buy a data cable. Three: come to the next HackHardware meeting. We will do sensors, radios and eventually design a PCB together.
>
> Scan the QR code and give us two minutes of feedback; it decides what we run next. Thanks for coming.

## 9. After the event

- [ ] Push a `solutions` branch with completed "if you finish early" ideas and any fixes you made live, so participants who continue at home are not stuck on a bug you already solved.
- [ ] Fix anything in the docs that tripped people up today, while you remember. Especially TROUBLESHOOTING rows you had to invent on the spot.
- [ ] Collect feedback (the QR form). Read it within 48 hours; reply to anyone who left contact details.
- [ ] Post photos (the room photo at the Task 4 check, pages on phones) to the club channels with the repo link. Tag the venue or sponsor if any.
- [ ] Inventory: count boards and cables back in, throw out tagged dead ones, restock adapters. Mark v1.1 boards (LED on GPIO 38) so next time you can tell people in advance.
- [ ] Next workshop ideas, in roughly increasing effort:
  - Sensors: BME280 (temperature, humidity, pressure) or an ultrasonic rangefinder, as new rows on the same page. Reuses everything from today.
  - LoRa: long-range links with two Heltec/TTGO LoRa boards.
  - MQTT and Home Assistant: publish the status to a broker on a Raspberry Pi.
  - Battery and deep sleep: how long can the board run on a small battery?
  - PCB design: KiCad a carrier board for the ESP32 with an LED bar and a battery holder.
- [ ] Book the room for the next one.
