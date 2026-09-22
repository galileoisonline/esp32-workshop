# Slides

`ESP32_Workshop.pptx` is the deck for the workshop: 32 slides, 16:9, with speaker notes on
every slide. It is written for the ESP32-S3-DevKitC-1.

## Opening it

- **Google Slides:** Drive, New, File upload, pick the `.pptx`, then open it with Google
  Slides. The notes appear in the speaker-notes pane under each slide, and Presenter view
  (the arrow next to "Slideshow") shows them on your laptop while the projector shows the slide.
- **PowerPoint or Keynote:** double-click the file.

The deck uses only Arial and Courier New, so it looks the same everywhere and nothing gets
substituted on import. Diagrams are ordinary shapes, so you can move or edit them.

## Before presenting

Search the deck for these placeholders and fill them in:

- `[DATE]`, `[ROOM]` on slide 1
- `[CLUB DISCORD/LINK]`, `[FEEDBACK LINK]`, `[DATE / TOPIC]` on slide 32
- `[stickers / name tags]` in the notes of slide 1 (how students recognise helpers)

## How the deck is organised

| Slides | Content | Minutes |
|---|---|---|
| 1–3 | Title, plan, setup check (board, UART connector, port) | 0:00–0:10 |
| 4–13 | Background: what the chip is, chip vs module vs board, board tour, which pins are usable, power and brownout, how flashing works, why "hold BOOT" works, FreeRTOS underneath Arduino, the shape of a sketch | 0:10–0:30 |
| 14, 15 | Task 1 (blink) and what `digitalWrite` / `rgbLedWrite` do | 0:30–0:45 |
| 16–20 | Task 2 (serial and button), Serial, pull-ups and debouncing, timing without `delay()`, on-chip sensors | 0:45–1:00 |
| 21, 22 | Task 3 (Wi-Fi) and Wi-Fi modes | 1:05–1:20 |
| 23–26 | Task 4 (web page), HTTP, the polling loop, extending it | 1:20–1:45 |
| 27–29 | Task 5 (ESP-NOW), what is on the air, challenges | 1:45–2:00 |
| 30–32 | Where to go next, buying a board, wrap-up | 2:00 |

If you are behind, the background section (4–13) is where to cut: slides 9, 12, 15 and 20
are depth that can be skipped or left for students to read later. The notes say so where it
applies.
