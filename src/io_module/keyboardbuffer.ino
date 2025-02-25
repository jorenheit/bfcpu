#include "keyboardbuffer.h"

void KeyboardBuffer::begin() {
  kb.begin();
}

void KeyboardBuffer::update() {
  while (kb.available()) {
    ringBuf.push(kb.read());
  }
}

char KeyboardBuffer::get() {
  auto result = ringBuf.get();
  return result.ok ? result.value : 0;
}