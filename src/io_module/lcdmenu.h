
#pragma once
#include "button.h"
#include "lcdbuffer.h"
#include "lcdscreen.h"
struct MenuItem;

class LCDMenu {
  unsigned long _lastActiveTime = 0;
  LCDBuffer &_buffer;
  LCDScreen &_screen;
  MenuItem *_current;

public:
  LCDMenu(LCDBuffer &buf, LCDScreen &scr);
  void begin();
  void enter();
  bool active() const;
  void handleButtons(ButtonState const up, ButtonState const down, ButtonState const both);

private:
  void display();
};