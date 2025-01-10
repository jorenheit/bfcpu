#pragma once
#include "common.h"

class Keyboard {
  PS2Keyboard ps2keyboard;
  byte data = 0;

public:

  bool enabled = false;
  bool ready = true;
  volatile bool interrupted = false;

  void begin();
  void send();
};