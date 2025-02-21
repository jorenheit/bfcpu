#pragma once
#include <PS2Keyboard.h>

class Keyboard {
  PS2Keyboard kb;

public:
  void begin();
  byte get();
  void flush();
};
