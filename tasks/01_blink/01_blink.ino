/*
 * Task 1: blink
 *
 * Goal: make the LED on the board blink.
 *
 * What to look for: the RGB LED (a small white square near the middle of the
 * board) turns on and off every half second. The red LED near the USB
 * connectors is the power LED; it stays on and you cannot control it.
 *
 * Line to edit: BLINK_MS below. Make it 100 or 2000, click Upload again,
 * and watch the rate change.
 *
 * If nothing lights up: your board is the v1.1 revision. In config.h change
 * LED_PIN from 48 to 38 and upload again.
 *
 * Concepts: setup() runs once, loop() runs forever, delay() pauses the
 * program. A plain LED is driven with digitalWrite(); this board's RGB LED
 * is driven with rgbLedWrite(), see setLed() at the bottom.
 */

#include "config.h"   // BOARD_NAME, LED_PIN, LED_IS_RGB, SERIAL_BAUD live here

// How long the LED stays on, and how long it stays off, in milliseconds.
// 500 ms on + 500 ms off = one blink per second.
const unsigned long BLINK_MS = 500;

// ---------------------------------------------------------------------------
// setup() runs once when the board powers up or you press the RST button.
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(SERIAL_BAUD);   // open the serial link to your computer
  delay(300);                  // give the serial port a moment to wake up
  printBanner();

#if !LED_IS_RGB
  // A plain LED: tell the chip that LED_PIN is an output (we drive it).
  // The RGB LED does not need this; rgbLedWrite() sets the pin up itself.
  pinMode(LED_PIN, OUTPUT);
#endif
}

// ---------------------------------------------------------------------------
// loop() runs again and again, forever, as fast as it can.
// ---------------------------------------------------------------------------
void loop() {
  setLed(true);                  // on
  Serial.println("led on");
  delay(BLINK_MS);               // stop everything for BLINK_MS milliseconds

  setLed(false);                 // off
  Serial.println("led off");
  delay(BLINK_MS);

  // ---- your code: try a new pattern here (SOS? heartbeat? colours?) ----
  // Ideas are in README.md under "If you finish early".
}

// Turn the LED on or off. One function so the rest of the sketch does not
// care which kind of LED the board has.
void setLed(bool on) {
#if LED_IS_RGB
  // An addressable RGB LED has one data pin. rgbLedWrite() sends the three
  // colour values down it as a precisely timed stream of pulses (the chip's
  // RMT peripheral does the timing). Equal r, g, b = white.
  uint8_t v = on ? LED_BRIGHTNESS : 0;
  rgbLedWrite(LED_PIN, v, v, v);
#else
  // A plain LED: HIGH = 3.3 V on the pin = LED on, LOW = 0 V = LED off.
  digitalWrite(LED_PIN, on ? HIGH : LOW);
#endif
}

// Print which program is running. Every task starts with this line so you
// always know what is on the board.
void printBanner() {
  Serial.println();
  Serial.print("[");
  Serial.print(BOARD_NAME);
  Serial.println("] Task 1: blink");
  Serial.print(LED_IS_RGB ? "RGB LED on GPIO " : "LED on GPIO ");
  Serial.print(LED_PIN);
  Serial.print(", blink every ");
  Serial.print(BLINK_MS);
  Serial.println(" ms");
}
