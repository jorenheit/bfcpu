#pragma once

enum LCDDriverPins {
  SH_CP = 6, // SRCLK
  DS = 5,    // SER
  ST_CP = 4  // RCLK
};

enum ModuleDriverPins {
  SYSTEM_CLOCK_INTERRUPT_PIN = 2,
  KEYBOARD_CLOCK_INTERRUPT_PIN = 3,
  DISPLAY_ENABLE_PIN = A1,
  DISPLAY_MODE_PIN = A2,
  SCROLL_UP_PIN = A3,
  SCROLL_DOWN_PIN = A4,
  KEYBOARD_DATA_PIN = A5,
  KEYBOARD_ENABLE_PIN = A6,   // CAUTION!!! CANNOT USE DIGITALREAD ON THIS PIN!!!

  D0 = 7,
  D1 = 8,
  D2 = 9,
  D3 = 10,
  D4 = 11,
  D5 = 12,
  D6 = 13,
  D7 = A0
};

inline constexpr int DATA_PINS[] = {
  D0, D1, D2, D3, D4, D5, D6, D7
};

enum LCDParams {
  VISIBLE_LINES = 2,
  TOTAL_LINES = 20,
  CHARS = 16,
  TAB = 4,
  REFRESH_DELAY = 100,
  BUTTON_DEBOUNCE_DELAY = 50
};

enum DisplayMode {
  ASCII = 0,
  DECIMAL = 10,
  HEXADECIMAL = 16
};

