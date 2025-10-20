#pragma once

// ============================  Static Settings ================================= //

enum LCDDriverPins: uint8_t {
  // Connections from Shift Register to Arduino
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
  K_OUT_PIN = 0,
  K_IN_PIN = 1,
  SYSTEM_CLOCK_INTERRUPT_PIN = 2,
  KEYBOARD_CLOCK_INTERRUPT_PIN = 3,
  DISPLAY_ENABLE_PIN = A1,
  KEYBOARD_ENABLE_PIN = A2,
  SCROLL_UP_PIN = A3,
  SCROLL_DOWN_PIN = A4,
  KEYBOARD_DATA_PIN = A5

/*
  // Data Pins (not used directly but here for documentation purposes):
  D0 = 7,
  D1 = 8,
  D2 = 9,
  D3 = 10,
  D4 = 11, 
  D5 = 12,
  D6 = 13,
  D7 = A0
*/
};

enum DisplayMode: uint8_t {
  ASCII,
  DECIMAL,
  HEXADECIMAL,
  N_MODES
};

enum InputMode: uint8_t {
  BUFFERED,
  IMMEDIATE
};

enum ButtonPrams: int {
  BUTTON_DEBOUNCE_DELAY = 200,
  BUTTON_HOLD_TIME = 1000
};

enum RNGParams: int {
  RNG_DEFAULT_SEED = 69
};

enum LCDParams: int {
  VISIBLE_LINES = 4,
  LINE_SIZE = 20,
  TOTAL_LINES = 20,
  TAB_WIDTH = 4,
  NO_SCROLL_DELAY = 200,
  BOOT_MESSAGE_DELAY = 500,
  MENU_TIMEOUT = 5000,
  TEMP_MESSAGE_TIMEOUT = 1000,
  MENU_SELECT_CHARACTER = 0x7E, // rightarrow
  MENU_LABEL_OFFSET = 2,
  LCD_COMMAND_EXECUTION_MICROS = 40,
  DISPLAY_FPS = 10,
  DISPLAY_FRAMETIME_MILLIS = 1000 / DISPLAY_FPS,

  AUTOSCROLL_DEFAULT_SETTING = true,
  ECHO_DEFAULT_SETTING = true,
  DISPLAY_MODE_DEFAULT_SETTING = ASCII,
  INPUT_MODE_DEFAULT_SETTING = BUFFERED,
  DELIMITER_DEFAULT_SETTING = ','
};

enum FrequencyParams: int { 
  // Frequency Measurements
  FREQUENCY_TIMEOUT = 2000,          // How many ms the frequency stays displayed after buttons released.
  FREQUENCY_UPDATE_INTERVAL = 1000,  // How many ms have to pass before a new frequency is measured.
  FREQUENCY_MEASUREMENT_TIME = 500,  // How many ms the ticks are sampled to calculate the frequency.
  FREQUENCY_DISPLAY_PRECISION = 3    // Number of decimals used to display the frequency.
};

enum KeyboardParams: int {
  KEYBOARD_TIMEOUT = 250
};

enum IOBuffers: uint8_t {
  KB_RING1 = 16,
  KB_RING2 = 16,
  LCD_RING = 64
};

enum EEPROMStorageParams: uint8_t {
  EEPROM_VALID_FLAG_VALUE = 0xab,
  EEPROM_VALID_FLAG_ADDRESS = 0,
  EEPROM_SETTINGS_ADDRESS = 1
};

enum HandshakeParams: int {
  HANDSHAKE_STARTUP_DELAY_MILLIS = 500
};

// ============================  Dynamic Settings ================================= //

struct Settings {
  DisplayMode displayMode = static_cast<DisplayMode>(DISPLAY_MODE_DEFAULT_SETTING);
  InputMode inputMode = static_cast<InputMode>(INPUT_MODE_DEFAULT_SETTING);
  bool autoscrollEnabled = AUTOSCROLL_DEFAULT_SETTING;
  bool echoEnabled = ECHO_DEFAULT_SETTING;
  char delimiter = DELIMITER_DEFAULT_SETTING;
  int rngSeed = RNG_DEFAULT_SEED;

  bool operator==(Settings const &other) const {
    return (displayMode == other.displayMode) &&
           (inputMode == other.inputMode) &&
           (autoscrollEnabled == other.autoscrollEnabled) &&
           (echoEnabled == other.echoEnabled) &&
           (delimiter == other.delimiter) && 
           (rngSeed == other.rngSeed);
  }

  bool operator!=(Settings const &other) const {
    return !(*this == other);
  }
};
