/*
 * Task 2: serial and button
 *
 * Goal: the board prints its status to the Serial Monitor once a second,
 * and the BOOT button switches the blinking LED on and off.
 *
 * What to look for: Serial Monitor (115200 baud) prints a line like
 *     t=12.3s temp=41.2C touch=57 button=0 led=1
 * every second. Press BOOT and you get "EVENT button pressed".
 * Touch the pin labelled 4 (T0) and the touch value drops.
 *
 * Lines to edit: BLINK_MS (blink speed) and REPORT_MS (how often to print).
 *
 * Concepts: millis() instead of delay() so the program never freezes,
 * INPUT_PULLUP buttons read LOW when pressed, edge detection (react to the
 * moment of pressing, not to "is held down").
 */

#include "config.h"

// ----- timing -------------------------------------------------------------
const unsigned long BLINK_MS    = 500;   // LED half-period
const unsigned long REPORT_MS   = 1000;  // one status line per second
const unsigned long DEBOUNCE_MS = 30;    // ignore button bounces shorter than this

// ----- state --------------------------------------------------------------
bool blinkEnabled = true;    // toggled by the BOOT button
bool ledOn        = false;   // what the LED is doing right now

unsigned long lastBlinkAt  = 0;   // millis() when the LED last changed
unsigned long lastReportAt = 0;   // millis() when we last printed a line

// Button bookkeeping for edge detection + debounce.
int  lastButtonReading = HIGH;   // raw pin value we saw on the previous loop
int  buttonState       = HIGH;   // debounced, "official" button state
unsigned long lastButtonChangeAt = 0;

// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(300);
  printBanner();

#if !LED_IS_RGB
  pinMode(LED_PIN, OUTPUT);   // plain LED only; rgbLedWrite() sets its own pin up
#endif

  // INPUT_PULLUP: an internal resistor holds the pin at HIGH (1) when nothing
  // is pressed. The BOOT button connects the pin to GND, so pressed = LOW (0).
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

// ---------------------------------------------------------------------------
// Notice: no delay() anywhere in loop(). Each job checks the clock and acts
// only when it is its turn, so the button is never missed while blinking.
// ---------------------------------------------------------------------------
void loop() {
  updateLed();
  checkButton();
  printStatus();
}

// Blink without delay(): flip the LED whenever BLINK_MS has passed.
void updateLed() {
  if (!blinkEnabled) {
    setLed(false);
    return;
  }
  unsigned long now = millis();
  if (now - lastBlinkAt >= BLINK_MS) {   // subtraction is safe even when millis() wraps
    lastBlinkAt = now;
    setLed(!ledOn);
  }
}

// Detect the moment the button goes from released to pressed (a "falling
// edge") and ignore the electrical noise that happens during that moment.
void checkButton() {
  int reading = digitalRead(BUTTON_PIN);

  // Any change in the raw reading restarts the debounce timer.
  if (reading != lastButtonReading) {
    lastButtonChangeAt = millis();
    lastButtonReading  = reading;
  }

  // Only accept the new value once it has been stable for DEBOUNCE_MS.
  if (millis() - lastButtonChangeAt >= DEBOUNCE_MS && reading != buttonState) {
    buttonState = reading;
    if (buttonState == LOW) {          // LOW = pressed (because of the pull-up)
      handleButtonPress();
    }
  }
}

// Runs exactly once per press, no matter how long the button is held.
void handleButtonPress() {
  blinkEnabled = !blinkEnabled;
  Serial.println("EVENT button pressed");
  Serial.print("blinking is now ");
  Serial.println(blinkEnabled ? "on" : "off");

  // ---- your code: do something else on press (change BLINK_MS? count presses?) ----
}

// Once a second, gather readings and print one line.
void printStatus() {
  unsigned long now = millis();
  if (now - lastReportAt < REPORT_MS) return;
  lastReportAt = now;

  float    uptimeS = now / 1000.0;               // milliseconds -> seconds
  float    tempC   = temperatureRead();          // chip's internal sensor, deg C (rough)
  uint32_t touch   = touchRead(TOUCH_PIN);       // ESP32-S3: number goes UP when touched
  int      button  = (buttonState == LOW) ? 1 : 0;  // report 1 = pressed, easier to read

  Serial.print("t=");      Serial.print(uptimeS, 1);   // one decimal place
  Serial.print("s temp="); Serial.print(tempC, 1);
  Serial.print("C touch="); Serial.print(touch);
  Serial.print(" button="); Serial.print(button);
  Serial.print(" led=");   Serial.println(ledOn ? 1 : 0);

  // ---- your code: add a status field here (for example free heap, analogRead) ----
}

// One place that touches the hardware, so ledOn always matches reality.
void setLed(bool on) {
  ledOn = on;
#if LED_IS_RGB
  uint8_t v = on ? LED_BRIGHTNESS : 0;   // addressable RGB LED: equal r,g,b = white
  rgbLedWrite(LED_PIN, v, v, v);
#else
  digitalWrite(LED_PIN, on ? HIGH : LOW);
#endif
}

void printBanner() {
  Serial.println();
  Serial.print("[");
  Serial.print(BOARD_NAME);
  Serial.println("] Task 2: serial and button");
  Serial.println("Press BOOT to start/stop blinking. Touch pin 4 (T0) to change the touch value.");
}
