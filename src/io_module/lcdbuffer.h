#pragma once

#include "settings.h"
#include "ringbuffer.h"

class LCDBuffer {
  RingBuffer<char, LCD_RING> ringBuf;
  char screenBuf[TOTAL_LINES][LINE_SIZE + 1];

  uint8_t pos = 0;
  uint8_t currentLine = 0;
  uint8_t topLine = 0;
  uint8_t bottomLine = TOTAL_LINES - 1;
  uint8_t topVisibleLine = 0;
  uint8_t bottomVisibleLine = VISIBLE_LINES - 1;
  
  mutable bool changed = false;
  static_assert(TOTAL_LINES <= 256, "TOTAL_LINES cannot exceed 256.");

public:
  struct Settings {
    DisplayMode mode = static_cast<DisplayMode>(DISPLAY_MODE_DEFAULT_SETTING);
    bool autoscrollEnabled = AUTOSCROLL_DEFAULT_SETTING;
    bool echoEnabled = ECHO_DEFAULT_SETTING;
    char delimiter = DELIMITER_DEFAULT_SETTING;
    bool operator==(Settings const &other) const;
    bool operator!=(Settings const &other) const;
  };

private:
  Settings settings;

public:
  void begin();
  void update();
  void enqueue(byte const c);
  void enqueueEcho(byte const c);
  
  void scrollDown(uint8_t n = 1);
  void scrollUp(uint8_t n = 1);
  void clear();

  void setMode(DisplayMode const mode);
  void setAutoscrollEnabled(bool const val);
  void setEchoEnabled(bool const val);
  void setDelimiter(char const delim);

  void setSettings(Settings const &set);
  Settings const &getSettings() const;

  struct View {
    char const *lines[VISIBLE_LINES];
    uint8_t cursorRow;
    uint8_t cursorCol;
    size_t id;
  };

  View view() const;


  
private:
  void insertAsAscii(byte const c);
  void insertAsHex(byte const c);
  void insertAsDecimal(byte const c);
  void clearLine(uint8_t const idx);
  void bringIntoView();
  void enter();
  void tab();
  void put(char const c);
  void newLine();

  uint8_t normalize(uint8_t line) const;
};
