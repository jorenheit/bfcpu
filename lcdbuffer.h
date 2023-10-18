#pragma once
#include <LiquidCrystalSerial.h>
#include "settings.h"

class LCDBuffer {
  String buf[2];
  LiquidCrystalSerial lcd;
  int line = 0;
  int pos = 0;
  DisplayMode mode = ASCII;
  bool changed = false;

public:
  LCDBuffer();

  void push(byte const c);
  void send();
  void setMode(DisplayMode m);
  DisplayMode getMode() const;

private:
  void pushAscii(byte const c);
  void pushNumber(byte const c, char const separator = '|');
  void clear(String &buf, char const fill = ' ');
};
