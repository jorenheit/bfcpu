#pragma once
#include "PS2Keyboard.h"


class KeyboardBuffer {
  
  PS2Keyboard kb;
  RingBuffer<char, KB_RING2> ringBuf;

public:
  void begin();
  void update();
  char get();
  char peek() const;
};
