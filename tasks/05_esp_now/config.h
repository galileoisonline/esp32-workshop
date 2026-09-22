// config.h - shared settings for every task.
//
// Arduino only lets a sketch #include files that live in its own folder, so
// each task folder has an identical copy of this file. Edit the values here,
// not in the .ino files. If you change something, copy it to the other folders.

#pragma once

// ---------------------------------------------------------------------------
// Your board
// ---------------------------------------------------------------------------

// A name for your board. Letters, digits and dashes only, max 11 characters
// (it has to fit in an ESP-NOW message in Task 5). This becomes the name of
// the Wi-Fi network your board creates, so pick something nobody else in the
// room will pick.
#define BOARD_NAME "ESP32-1"

// ---------------------------------------------------------------------------
// Pins. Written for the ESP32-S3-DevKitC-1. See the bottom for other boards.
// ---------------------------------------------------------------------------

// The DevKitC-1 has no plain LED. It has an addressable RGB LED (WS2812 type)
// controlled through ONE data pin: GPIO 48 on the original board, GPIO 38 on
// v1.1. Both are sold and look the same. If nothing lights up in Task 1,
// change 48 to 38.
#define LED_PIN 48

// 1 = LED_PIN drives an addressable RGB LED (rgbLedWrite).
// 0 = LED_PIN drives a plain LED (digitalWrite), like on a classic ESP32 DevKit.
#define LED_IS_RGB 1

// Brightness of the RGB LED when "on", 0-255. It is very bright; 40 is plenty.
#define LED_BRIGHTNESS 40

// The BOOT button on the board is wired to GPIO 0. Pressed = LOW (0).
#define BUTTON_PIN 0

// Capacitive touch input. T4 is GPIO 4 (header pin labelled "4", left side).
// On the ESP32-S3 the value goes UP when you touch the pin.
#define TOUCH_PIN T4

// Classic ESP32 DevKit V1 / WROOM-32 instead? Use:
//   LED_PIN 2, LED_IS_RGB 0, TOUCH_PIN T0 (also GPIO 4; the value goes DOWN).

// ---------------------------------------------------------------------------
// Serial
// ---------------------------------------------------------------------------

// Speed of the USB serial link. The Serial Monitor must be set to the same
// number (115200) or you will see garbage characters.
#define SERIAL_BAUD 115200

// ---------------------------------------------------------------------------
// Wi-Fi (used from Task 3 onwards)
// ---------------------------------------------------------------------------

// Password of the Wi-Fi network your board creates. WPA2 needs 8+ characters.
// Change it if you do not want classmates connecting to your board.
#define AP_PASSWORD "hackhardware"

// 0 = the board creates its own Wi-Fi network named BOARD_NAME and phones
//     connect directly to it. Works everywhere, no internet needed. Default.
// 1 = the board joins an existing network (for example your phone hotspot)
//     using WIFI_SSID / WIFI_PASS below. Campus Wi-Fi usually blocks this;
//     use a personal hotspot instead.
#define USE_STATION_MODE 0

// Only used when USE_STATION_MODE is 1. Fill in your hotspot's name/password.
#define WIFI_SSID "YOUR-HOTSPOT-NAME"
#define WIFI_PASS "YOUR-HOTSPOT-PASSWORD"
