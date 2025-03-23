#pragma once
#include "settings.h"
#include "PS2Keyboard.h"


class KeyboardBuffer {   
  PS2Keyboard kb;
  RingBuffer<char, KB_RING2> ringBuf;
  Settings const &settings;

public:
  KeyboardBuffer(Settings const &s):
    settings(s)
  {}

  void begin();
  void update();
  char get();
  void clear();
  char peek() const;
};
