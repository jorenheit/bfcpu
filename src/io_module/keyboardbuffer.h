#pragma once
#include "settings.h"
#include "PS2Keyboard.h"

// Make sure echo is defined somewhere!
void echo(char const);

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
  char peek() const;
};
