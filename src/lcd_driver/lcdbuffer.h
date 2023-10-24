#pragma once
#include <LiquidCrystalSerial.h>
#include "settings.h"

class LCDBuffer {
  char buf[TOTAL_LINES][CHARS + 1];
  LiquidCrystalSerial lcd;
  int line = 0;
  int pos = 0;
  int topVisibleLine = 0;
  DisplayMode mode = ASCII;
  bool changed = false;

public:
  LCDBuffer();

  void push(byte const c);
  void send();
  void scrollDown();
  void scrollUp();
  void scroll(int amount);
  void clear(char const fill = ' ');
  
  void setMode(DisplayMode m);
  DisplayMode getMode() const;

private:
  void pushAscii(byte const c);
  void pushNumber(byte const c, char const separator = '|');
  void clearLine(int const idx, char const fill = ' ');
  
};
