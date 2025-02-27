
#pragma once
#include "button.h"
#include "lcdbuffer.h"
#include "lcdscreen.h"

namespace MenuTree {
  struct MenuItem;
  extern MenuItem rootMenu;
}

class LCDMenu {
  unsigned long _lastActiveTime = 0;
  LCDBuffer &_buffer;
  LCDScreen &_screen;
  MenuTree::MenuItem *_current = &MenuTree::rootMenu;

public:
  LCDMenu(LCDBuffer &buf, LCDScreen &scr);
  void begin();
  void enter();
  bool active() const;
  void handleButtons(ButtonState const up, ButtonState const down, ButtonState const both);

private:
  void display();
};