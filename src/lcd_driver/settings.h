#pragma once

enum LCDDriverPins {
  SH_CP = 4, // 5
  DS = 3,    // 4
  ST_CP = 2  // 3  --> need pin 2 for interrupt handling
};

enum ModuleDriverPins {
  CLOCK_INTERRUPT_PIN = 2,
  WRITE_ENABLE_PIN = A0,  // should be able to do digitalRead on analog pin, right?
  DISPLAY_MODE_PIN = A1,

  D0 = 6,
  D1 = 7,
  D2 = 8,
  D3 = 9,
  D4 = 10,
  D5 = 11,
  D6 = 12,
  D7 = 13
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
};

enum DisplayMode {
  ASCII = 0,
  DECIMAL = 10,
  HEXADECIMAL = 16
};
