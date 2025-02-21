#include "keyboard.h"
#include "settings.h"

void Keyboard::begin() {
  kb.begin(KEYBOARD_DATA_PIN, KEYBOARD_CLOCK_INTERRUPT_PIN);
}

byte Keyboard::get() {
  return kb.available() ? kb.read() : 0;
}

void Keyboard::flush() {
  while (kb.available()) kb.read();
}
