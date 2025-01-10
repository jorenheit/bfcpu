#include "keyboard.h"

void Keyboard::begin() {
  ps2keyboard.begin(KEYBOARD_DATA_PIN, KEYBOARD_CLOCK_INTERRUPT_PIN);
}

void Keyboard::send() {
  if (!interrupted) {
    data = ps2keyboard.available() ? ps2keyboard.read() : 0;
  }
  
  ready = false;
  interrupted = false;
  for (int i = 0; i != 8; ++i) {
    digitalWrite(DATA_PINS[i], (data >> i) & 1);
  }
  ready = true;
}
