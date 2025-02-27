#pragma once
#include "PS2Keyboard.h"


class KeyboardBuffer {
  
  PS2Keyboard kb;
  RingBuffer<char, 256> ringBuf;

public:
  void begin();
  void update();
  char get();
  char peek() const;
};
