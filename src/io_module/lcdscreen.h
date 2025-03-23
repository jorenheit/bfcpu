#pragma once

#include "LiquidCrystal_74HC595.h"
#include "settings.h"

class LCDScreen {
  Settings settings;
  LiquidCrystal_74HC595 lcd;
  unsigned long tempTimeout = 0;

public:
  LCDScreen(Settings const &s);

  void begin(char const *msg = 0);
  void display(LCDBuffer::View const &view, bool forced = false); 
  void displayTemp(char const *lines[], uint8_t const n, size_t const timeout);
  void displayFrequency();
  void abortTemp();
  void enableCursor();
  void disableCursor();

  template <typename ... Args>
  void displayTemp(size_t const timeout, Args ... args) {
    static_assert(sizeof ... (Args) <= VISIBLE_LINES, "Too many lines");
    char const *lines[] = {args ...};
    displayTemp(lines, sizeof ... (args), timeout);
  }
};