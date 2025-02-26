#pragma once

#define LCD_COMMAND_EXECUTION_MICROS 40

enum LCDDriverPins: uint8_t {
  // Connections frp, Shift Register to Arduino
  SH_CP = 6, // SRCLK
  DS    = 5, // SER
  ST_CP = 4, // RCLK

  // Connections from LCD panel to Shift Register
  RS_595 = 1,
  E_595  = 3,
  D0_595 = 4,
  D1_595 = 5,
  D2_595 = 6,
  D3_595 = 7
};

enum ModuleDriverPins: uint8_t {
  SYSTEM_CLOCK_INTERRUPT_PIN = 2,
  KEYBOARD_CLOCK_INTERRUPT_PIN = 3,
  DISPLAY_ENABLE_PIN = A1,
  KEYBOARD_ENABLE_PIN = A2,
  SCROLL_UP_PIN = A3,
  SCROLL_DOWN_PIN = A4,
  KEYBOARD_DATA_PIN = A5,

  D0 = 7,
  D1 = 8,
  D2 = 9,
  D3 = 10,
  D4 = 11, 
  D5 = 12,
  D6 = 13,
  D7 = A0
};

static constexpr uint8_t DATA_PINS[] = {D0, D1, D2, D3, D4, D5, D6, D7};

enum DisplayMode: uint8_t {
  ASCII,
  DECIMAL,
  HEXADECIMAL,
  N_MODES
};

static constexpr char const *displayModeString[N_MODES] = {
  "Text",
  "Decimal",
  "Hexadecimal"
};


enum LCDParams: int {
  VISIBLE_LINES = 2,
  TOTAL_LINES = 20,
  LINE_SIZE = 16,
  TAB_WIDTH = 4,
  NUMBER_MODE = HEXADECIMAL,
  NUMBER_SEPARATOR = '|',
  BUTTON_DEBOUNCE_DELAY = 200,
  BOOT_MESSAGE_DELAY = 500,
  NO_SCROLL_DELAY = 200,
  TEMP_MESSAGE_TIMEOUT = 1000,
  MENU_TIMEOUT = 5000,
  HOLD_TO_CLEAR_TIME = 2000
};

static_assert((int)NUMBER_MODE == (int)HEXADECIMAL || (int)NUMBER_MODE == (int)DECIMAL, 
  "NUMBER_MODE must be either DECIMAL or HEXADECIMAL");

static_assert(NUMBER_SEPARATOR >= 32 && NUMBER_SEPARATOR <= 126, 
  "NUMBER_SEPARATOR must be printable (32 - 126).");

enum KeyboardParams: int {
  KEYBOARD_TIMEOUT = 250
};

