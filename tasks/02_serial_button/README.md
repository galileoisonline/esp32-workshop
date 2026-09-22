# Task 2: serial and button

Print uptime, temperature, touch and button state to the Serial Monitor once a second, and use the BOOT button to start and stop the blinking.

## What you learn

- Timing with `millis()` instead of `delay()`, so the program never freezes.
- Reading a button with `INPUT_PULLUP` (and why pressed reads as `LOW`).
- Edge detection and debounce: react to the moment of a press, once.
- Reading the ESP32's built-in sensors: `temperatureRead()` and `touchRead()`.

## Steps

1. **File > Open...** and pick `tasks/02_serial_button/02_serial_button.ino`.
2. In the `config.h` tab set your `BOARD_NAME` (same as Task 1).
3. Click **Verify**, then **Upload** (hold **BOOT** if it sits on `Connecting...`).
4. Open the **Serial Monitor** (top-right icon), baud **115200**. Every second you should see a line like
   `t=12.3s temp=41.2C touch=24310 button=0 led=1`.
5. Press the **BOOT** button on the board once. You should see `EVENT button pressed` and the LED stops
   blinking. Press again and it resumes. Hold the button down: it fires only once per press.
6. Touch the bare header pin labelled **4** (left side, fourth from the top) with a fingertip and watch `touch=`
   rise from roughly 10000-30000 to well above that (about double). (On the classic ESP32 the value drops when touched; on the S3
   it rises.) Exact numbers vary from board to board.
7. Look at `updateLed()`: there is no `delay()` anywhere. Change `BLINK_MS` to `100` and upload again. The
   status line still arrives exactly once a second. That is the point of `millis()`.

Check: pressing BOOT prints `EVENT button pressed` in the Serial Monitor and the LED switches between blinking and off.

## If you finish early

- Serial Plotter. In `printStatus()` comment out every `Serial.print` except the touch value (print only the
  number, then `println`). Open **Tools > Serial Plotter** and watch a live graph while you touch pin 4.
- Speed control. In `handleButtonPress()`, instead of toggling, cycle `BLINK_MS` through 1000, 500, 100 and
  back to 1000. Hint: `BLINK_MS` has to stop being `const`.
- Extra field. Add `heap=` with `ESP.getFreeHeap()` to the status line at the `your code` comment.

## If it does not work

- Temperature reads about 50 C or looks constant: normal. The internal sensor is coarse and measures the chip,
  not the room. Hold a finger on the metal can for a minute to see it move.
- `touch=` does not change: you may be touching the wrong pin. `T4` is **GPIO 4**, left header, fourth pin from
  the top (after 3V3, 3V3, RST). Hold the metal pin itself, not the plastic.
- `touch=` does not drop when you touch: right, on the S3 it goes up. Watch for a rise.
- Serial Monitor empty although the upload worked: the cable is in the **USB** connector; move it to **UART**.
- Button triggers twice per press: raise `DEBOUNCE_MS` to `50`.
- Everything else: [../../docs/TROUBLESHOOTING.md](../../docs/TROUBLESHOOTING.md).
