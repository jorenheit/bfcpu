#pragma once
#include <PS2Keyboard.h>

void setIOPinsTo(int mode) {
  for (int i = 0; i != 8; ++i) {
    if (mode == INPUT) digitalWrite(DATA_PINS[i], LOW);
    pinMode(DATA_PINS[i], mode);
  }
}
