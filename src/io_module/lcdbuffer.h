#pragma once

#include "LiquidCrystal_74HC595.h"
#include "settings.h"
#include "ringbuffer.h"

class LCDBuffer {
  LiquidCrystal_74HC595 lcd;
  RingBuffer<char, 256> ringBuf;
  char screenBuf[TOTAL_LINES][LINE_SIZE + 1];

  uint8_t pos = 0;
  uint8_t currentLine = 0;
  uint8_t topLine = 0;
  uint8_t bottomLine = TOTAL_LINES - 1;
  uint8_t topVisibleLine = 0;
  uint8_t bottomVisibleLine = VISIBLE_LINES - 1;
  
  bool changed = false;
  unsigned long directTimeout = 0;

  DisplayMode mode = ASCII;
  static_assert(TOTAL_LINES <= 128, "TOTAL_LINES cannot exceed 128.");

public:
  LCDBuffer();

  void begin(char const *msg = 0);
  void push(byte const c);
  void push(char const *str);
  void send(bool const forced = false);
  void scrollDown(uint8_t n = 1);
  void scrollUp(uint8_t n = 1);
  void clear(char const *msg = 0);
  void nextMode(bool const prompt);
  void previousMode(bool const prompt);

  template <typename ... Args>
  void direct(int timeout, Args ... args) {
    static_assert(sizeof ... (Args) <= VISIBLE_LINES, "Too many lines");
    char const *lines[] = {args ...};
    lcd.clear();
    for (uint8_t idx = 0; idx != sizeof...(args); ++idx) {
      lcd.setCursor(0, idx);
      lcd.print(lines[idx]);
    }
    lcd.noCursor();
    directTimeout = millis() + timeout;
  }

  
private:
  void insertAsAscii(byte const c);
  void insertAsHex(byte const c);
  void insertAsDecimal(byte const c);
  void update();
  void clearLine(uint8_t const idx);
  void bringIntoView();
  void enter();
  void tab();
  void put(char const c);
  void newLine();

  uint8_t normalize(uint8_t line) const;
};
